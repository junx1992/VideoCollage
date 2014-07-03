/*************************************************************************\
Microsoft Research Asia
Copyright (c) 2003 Microsoft Corporation

Module Name:
    Utility Lib: Factory to create win32 windows
  
Abstract:
    
Notes:

Usage:
        
History:
    Created  on 2004 Sep. 25 by oliver_liyin
          
\*************************************************************************/

#pragma once

#include <Windows.h>
#include <assert.h>
#include <tchar.h>

/// nonstandard extension used: override specifier 'keyword'
#pragma warning(disable:4481)

namespace utl {

    /// Win32 message structure
    struct Win32Msg
    {
        LRESULT lResult;
        HWND    hWnd;
        UINT    uMsg;
        WPARAM  wParam;
        LPARAM  lParam;
    };

    /// Get the instance of Window from binding HWND
    /// If no window is binding, return NULL;
    /// Note: WindowResult must be public inherited from utl::Window
    template<class WindowResult>
    WindowResult* GetWindow(HWND hWnd)
    {
        LONG_PTR ptr = ::GetWindowLongPtr(hWnd, GWL_USERDATA);
        return dynamic_cast<WindowResult*>((Window*) ptr);
    }

    /// 1. First, for non-dialog window, 
    ///    define a window traits, defining styles and window class name
    ///
    ///     struct WindowTraits
    ///     {
    ///         const static UINT ClassStyle = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    ///         const static DWORD Style = WS_OVERLAPPEDWINDOW;
    ///         const static DWORD StyleEx = WS_EX_APPWINDOW;
    ///         static LPCTSTR ClassName() { return _T("utl::Window");}
    ///     };
    /// 
    /// 2. The child class must define a non-template version of window factory
    ///     It can be either Create, or CreateDialog, such as
    ///     static ChildWindow* Create(HINSTANCE hInstance, HWND hParent)
    ///     {
    ///         return super::Create<ChildWindow, ChildWindowTraits>(hInstance hParent);
    ///     }
    ///
    /// 3. To hide the ctor and dtor, you can define them as private as following
    ///     class ChildWindow : public utl::Window
    ///     {
    ///         ...
    ///     private:
    ///         friend class utl::Window;
    ///         ChildWindow(void);
    ///         ~ChildWindow(void);
    ///         ...
    ///     };
    ///
    class Window
    {
    public: /// Public interface

        /// Get the window's handle
        HWND GetHwnd() const
        {
            return m_hWnd;
        }

    protected:  /// To be overrided

        /// Should be override to handle window messages
        /// return TRUE if message is processed
        /// return FALSE if need further process
        /// NOTE: neverl call DefWindowProc or DefDlgProc
        virtual BOOL WindowProc(utl::Win32Msg& /*msg*/) { return FALSE;}

    protected:  /// factory to create window
        template<class ProductWindow, class WindowTraits>
        static HWND Create(HINSTANCE hInstance, HWND hParent)
        {
            /// parent should either NULL or valid window
            assert(hParent == NULL || ::IsWindow(hParent));
            if (hParent != NULL && !::IsWindow(hParent)) return NULL;

            /// Create Window object, ProductWindow must be child class
            Window* pWindow = CreateWindowObject<ProductWindow>();
            if (pWindow == NULL) return NULL;

            /// Create the window handle and attach to it.
            WindowFactory<WindowTraits>& factory = 
                WindowFactory<WindowTraits>::Instance(hInstance);
            HWND hWnd = factory.Create(hParent, pWindow);

            /// Check binding, if failed, clear resources
            return CheckBinding(hWnd, pWindow);
        }

        /// factory to create new modaless dialog window
        template<class ProductDialog>
        static HWND Create(HINSTANCE hInstance, HWND hParent, LPCTSTR lpTemplate)
        {
            /// parent should either NULL or valid window
            assert(hParent == NULL || ::IsWindow(hParent));
            if (hParent != NULL && !::IsWindow(hParent)) return NULL;

            /// Create Window object, ProductDialog must be child class
            Window* pWindow = CreateWindowObject<ProductDialog>();
            if (pWindow == NULL) return NULL;

            /// Create the modaless handle and attach to it.
            HWND hWnd = ::CreateDialogParam(
                hInstance,
                lpTemplate,
                hParent,
                DialogProcCallback,
                (LPARAM) pWindow);

            /// Check binding, if failed, clear resources
            return CheckBinding(hWnd, pWindow);
        }

        /// factory to create modal dialog box
        template<class ProductDialog>
        static INT_PTR DoModal(HINSTANCE hInstance, HWND hParent, LPCTSTR lpTemplate)
        {
            /// parent should either NULL or valid window
            assert(hParent == NULL || ::IsWindow(hParent));
            if (hParent != NULL && !::IsWindow(hParent)) return NULL;

            /// Create Window object, ProductDialog must be child class
            Window* pWindow = CreateWindowObject<ProductDialog>();
            if (pWindow == NULL) return NULL;

            /// set the initial hWnd to -1;
            pWindow->m_hWnd = HWND(-1);

            /// Create the modal dialog and attach to it.
            INT_PTR result = ::DialogBoxParam(
                hInstance,
                lpTemplate,
                hParent,
                DialogProcCallback,
                (LPARAM) pWindow);

            if (pWindow->m_hWnd == HWND(-1))
            {
                /// It's not sucessfully attached and detached.
                /// We need to clear the memory
                delete pWindow;
            }

            return result;
        }

    protected:  /// Protected ctor and dtor, only accessed by child class
        Window()
        {
            m_hWnd = NULL;
        }

        virtual ~Window()
        {
            if (::IsWindow(m_hWnd))
            {
                DettachWindow();
                ::DestroyWindow(m_hWnd);
            }
        }

    private:

#pragma region --- Private implementation

        template<class ProductWindow>
        static Window* CreateWindowObject()
        {
            ProductWindow* pProduct = new ProductWindow;
            if (pProduct == NULL) return NULL;

            /// Window Product must be child of Window, or return fail
            Window* pWindow = dynamic_cast<Window*>(pProduct);
            if (pWindow == NULL)
            {
                delete pProduct;
                return NULL;
            }
            return pWindow; 
        }

        /// Checking if binding succeeded
        static HWND CheckBinding(HWND hWnd, Window* pWindow)
        {
            if (!::IsWindow(hWnd) || hWnd != pWindow->m_hWnd)
            {
                delete pWindow;
                if (::IsWindow(hWnd)) ::DestroyWindow(hWnd);
                return NULL;
            }
            return hWnd;
        }

        /// The static function for windows call back
        static LRESULT CALLBACK WindowProcCallback(
            HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
        {
            /// Bind the window (in lParam) with hWnd
            /// If failed, window cannot process the messages later
            /// If succeeded, fall through and let window process WM_CREATE
            if (message == WM_CREATE)
            {
                assert(lParam != NULL);
                if (lParam == NULL) return -1;  /// failed and destroy

                CREATESTRUCT& cs = *(CREATESTRUCT*)lParam;
                Window* pWindow = (Window*) cs.lpCreateParams;
                assert(pWindow != NULL);
                if (pWindow == NULL) return -1; /// failed and destroy

                HRESULT hr = pWindow->AttachWindow(hWnd);
                assert(SUCCEEDED(hr));
                if (FAILED(hr)) return -1; /// failed and destroy
            }

            Window* pWindow = GetWindow<Window>(hWnd);
            if (pWindow != NULL)
            {
                if (message == WM_DESTROY)
                {
                    pWindow->DettachWindow();
                    delete pWindow;
                }
                else
                {
                    utl::Win32Msg msg = { 0, hWnd, message, wParam, lParam};
                    if(pWindow->WindowProc(msg)) return msg.lResult;
                }
            }

            return ::DefWindowProc(hWnd, message, wParam, lParam);
        }

        /// The static function for dialog call back
        static INT_PTR CALLBACK DialogProcCallback(
            HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
        {
            /// Bind the window (in lParam) with hWnd
            /// If failed, window cannot process the messages later
            /// If succeeded, fall through and let window process WM_INITDIALOG
            if (message == WM_INITDIALOG)
            {
                Window* pWindow = ((Window*)(lParam));
                assert(pWindow != NULL);

                if (pWindow != NULL)
                {
                    /// Attach to window handle
                    HRESULT hr = pWindow->AttachWindow(hWnd);
                    assert(SUCCEEDED(hr));
                    if (FAILED(hr)) return FALSE;
                }
            }

            /// Get the binding window, and let it process messages
            Window* pWindow = GetWindow<Window>(hWnd);
            if (pWindow != NULL)
            {
                /// If the dialog is created modelessly, using CreateDialog
                /// DestroyWindow must be called to invoke this message
                if (message == WM_DESTROY)
                {
                    pWindow->DettachWindow();
                    delete pWindow;
                }
                else
                {
                    utl::Win32Msg msg = { 0, hWnd, message, wParam, lParam};
                    if(pWindow->WindowProc(msg)) return msg.lResult;
                }
            }

            return FALSE;
        }

        /// Attach window to HWND user data segment
        HRESULT AttachWindow(HWND hWnd)
        {
            assert (hWnd != NULL);
            if (hWnd == NULL) return E_INVALIDARG;

            m_hWnd = hWnd;
            ::SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG) (LONG_PTR) this);
            return S_OK;
        }

        /// Dettach window and reset HWND user data segment
        HRESULT DettachWindow()
        {
            assert (m_hWnd != NULL);
            if (m_hWnd == NULL) return E_FAIL;
            ::SetWindowLongPtr(m_hWnd, GWL_USERDATA, 0);
            m_hWnd = NULL;
            return S_OK;
        }

#pragma endregion

    private:

#pragma region ---- WindowFactory

        /// WindowFactory is template class to make unique singleton Instance()
        /// Use WindowTraits to select class name and WindowProc
        /// 
        template<class WindowTraits>
        class WindowFactory
        {
        public:
            /// Use singleton to register window class
            static WindowFactory& Instance(HINSTANCE hInstance)
            {
                static WindowFactory factory(hInstance);
                return factory;
            }

            /// Create window with given parent
            /// assume default window size, no window title nor menu
            HWND Create(HWND hParent, LPVOID lpParam)
            {
                return ::CreateWindowEx(
                    WindowTraits::StyleEx,      /// DWORD dwExStyle,
                    WindowTraits::ClassName(),  /// LPCTSTR lpClassName,
                    _T(""),                     /// LPCTSTR lpWindowName,
                    WindowTraits::Style,        /// DWORD dwStyle,
                    0, 0,                       /// int x, y
                    CW_USEDEFAULT, 0,           /// int nWidth, nHeight
                    hParent,                    /// HWND hParent
                    NULL,                       /// HMENU hMenu
                    m_hInstance,                /// HINSTANCE hInstance
                    lpParam);                   /// LPVOID lpParam
            }

        private:
            /// ctor only works for inherited class
            WindowFactory(HINSTANCE hInstance)
            {
                m_hInstance = hInstance;
                m_WindowResult = RegisterWindowResult();
            }

            /// dtor only works for inherited class
            ~WindowFactory()
            {
                UnregisterWindowResult();
            }

            void UnregisterWindowResult()
            {
                if (m_WindowResult != NULL)
                {
                    UnregisterClass(WindowTraits::ClassName(), m_hInstance);
                    m_WindowResult = NULL;
                }
            }

            /// Register window class
            /// assume no extra class or window memory
            /// no icon, no background brush nor menu
            ATOM RegisterWindowResult()
            {
                WNDCLASSEX wcx;

                /// try to find the class in system
                if (GetClassInfoEx(NULL, WindowTraits::ClassName(), &wcx))
                {
                    return NULL;    /// the class is existing in system
                }

                /// try to find the class in module
                if (GetClassInfoEx(m_hInstance, WindowTraits::ClassName(), &wcx))
                {
                    return NULL;
                }

                /// Create a new class for this module
                ZeroMemory(&wcx, sizeof(WNDCLASSEX));
                wcx.cbSize = sizeof(WNDCLASSEX);                /// sizeof structure
                wcx.style = WindowTraits::ClassStyle;           /// redraw if size changes 
                wcx.lpfnWndProc = WindowProcCallback;           /// points to window procedure 
                wcx.cbClsExtra = 0;                             /// no extra class memory 
                wcx.cbWndExtra = 0;                             /// no extra window memory 
                wcx.hInstance = m_hInstance;                    /// handle to instance 
                wcx.hIcon = NULL;                               /// predefined app. icon 
                wcx.hCursor = ::LoadCursor(NULL, IDC_ARROW);    /// predefined arrow 
                wcx.hbrBackground = NULL;                       /// background brush 
                wcx.lpszMenuName =  NULL;                       /// name of menu resource 
                wcx.lpszClassName = WindowTraits::ClassName();  /// name of window class 
                wcx.hIconSm = NULL;                             /// small icon

                /// Register the window class. 
                return RegisterClassEx(&wcx);
            }

        private:
            ATOM m_WindowResult;
            HINSTANCE m_hInstance;
        };

#pragma endregion

    private:
        HWND m_hWnd;
    };

}   // namespace utl
