#ifndef __XTL_POINTER_HPP__
#define __XTL_POINTER_HPP__

/*************************************************************************\
Oliver Liyin Copyright (c) 2003

Module Name:
    XTL smart pointer support, including
    1. SharedPtr
    2. SharedArray
    3. ScopedPtr
    4. ScopedArray

Abstract:
    SharedPtr, reference counted pointer wrapper
    Only when refCounter is zero, the memory will be actually deleted

    SharedArray is a SharedPtr using operator delete[].

    ScopedPtr is a wrapper of pointer that cannot be copied

    ScopedArray is a ScopedPtr using operator delete[].

Usage:
  1. If you delete the SharedPtr/ScopedPtr, you will get compile error
      e.g. "delete ptr;  /// ambiguous implicit convertion to pointer"

  2. You can use SharedPtr in stl container, but not ScopedPtr
      e.g. "std::vector<SharedPtr<Bitmap> >, std::set<SharedPtr<Point> >"

  3. you cannot use "if(ptr) ..."
      Try "if(ptr != NULL) ..." or "if(!!ptr) ..." instead.

  4. You can use "std::swap(p, q);" safely for SharedPtr, but not ScopedPtr
     You can also use xtl::SwapPtr(p, q), basically same

  5. If you want to explicitly release the pointer,
        you can use "Reset(ptr)" or assign it to NULL pointer.
        e.g. "ptr = NULL;  /// release the reference explicitly"

  4. When you don't want delete the memory when SharedPtr expires,
        you can use "AttachPtr(ptr, p)" function. e.g.
        static int x =  3;          /// somehow, int* p = &x; cannot be deleted
        SharedPtr<int> ptr;         /// somehow, we have to use SharedPtr
        xtl::AttachPtr(ptr, &x);    /// only aliasing, without deletion

Notes:
  1. You should never dispose SharedPtr by yourself.
      e.g. "delete ptr; /// there is a compile error here"
      e.g. "ptr->Release(); /// for COM ptr, this is legal, but will crash"

  2. You must use correct disposer template parameter
      Default uses standard C++ delete operator.
      e.g.  SharedPtr<TCHAR> m_szFilename = new TCHAR[MAX_PATH];
        may crash when dipose the pointer, becase it requires delete[]
        You should use SharedArray, or SharedPtr<TCHAR, diposer_array>.

  3. Some global friend function are defined, to avoid smart pointer member functions
        They include:
        /// works for both shared and scoped
        a) PointerType GetPtr(smart_ptr);
        b) void ResetPtr(smart_ptr, raw_ptr);

        /// works for only shared pointer
        c) void SwapPtr(shared_ptr, shared_ptr);
        d) void AttachPtr(shared_ptr, raw_ptr);

        /// works for only scoped pointer
        e) PointerType DetachPtr();

    Note that these functions are in global namespace.
    Therefore, do not use xtl::GetPtr(ptr).  Just use GetPtr(ptr);

Known potential usage dangeous:
  1. You must use GetPtr(ptr) function when used in in printf(char*, ...);
     e.g.
        SharedArray<TCHAR> m_szFilename = new TCHAR[MAX_PATH];
        printf("File name = %s\n", m_szFilename); /// this will crash
        printf("File name = %s\n", GetPtr(m_szFilename)); /// correct

  2. When you use a SharedPtr<T> as a return value, be sure disposer is same
     e.g.   SharedPtr<T> CreateT()
            {
                return SharedArray<T>(new T[3]);  /// will crash
            }
     These code will crash, because we the copy ctor requires same disposer.
     Here, the SharedArray is converted to a RAW pointer and construct a new
     SharedPtr using this RAW pointer.  Therefore, memory is binded to a different
     disposer and cause crash. So, BE SURE assign or copy to SAME disposer.

TODO:
    1. Make the ref counting thread safe using InterlockedIncrement 
    2. Self assignment issue !!!

History:
    Created  on 2004 Oct. 15 by oliver_liyin

\*************************************************************************/
#include "Xtl_Platform.hpp"

#define DISABLE_CONST_SMART_POINTER

#ifndef DISABLE_CONST_SMART_POINTER
#include "Xtl_TraitsEx.hpp"
#endif

#define DISABLE_USE_COUNTER_SMART_POOL

#ifndef DISABLE_USE_COUNTER_SMART_POOL
#include "Xtl_SmartAlloc.hpp"
#endif

#ifndef DISABLE_IMPLICIT_CONVERT_SMARTPTR_TO_RAWPTR

    /// Implicit converted to pointer
    ///       T* p = ptr;    /// this is dangerous to memory leak.
    /// NOTE: 
    ///  1. to avoid delete operation on this pointer
    ///       we add ambiguous implicit converter to both void* and T*
    ///  2. This also disallow operator ++, --, +=, -=, etc.
    ///  3. But this allow q = p + 3; and offset = p - q;
    /// RISKY:
    ///  1. maybe empty pointer.
    ///      SharedPtr<Object> Create() { return SharedPtr<Object>(new Object);}
    ///      Object* p = Create();            /// Error! This p cannot get the object
    ///      SharedPtr<Object> p = Create();  /// Correct! you have to use this
    ///
#   define IMPLICIT_CONVERT_SMARTPTR_TO_RAWPTR_POLICY           \
        operator void* ()       const  { return m_ptr;}         \
        operator PointerType () const  { return m_ptr;}         \

#else //DISABLE_IMPLICIT_CONVERT_SMARTPTR_TO_RAWPTR

    /// If we disalbe implicity convert to pointer
    /// operator [] is supported, but you have to use GetPtr to retrieve the pointer
    /// For example, glVertex3f(GetPtr(v3));
#   define IMPLICIT_CONVERT_SMARTPTR_TO_RAWPTR_POLICY           \
        ReferenceType operator [] (size_t index) const { return m_ptr[index];}

#endif//DISABLE_IMPLICIT_CONVERT_SMARTPTR_TO_RAWPTR

namespace xtl
{
#pragma region Implementation of smart pointer
    //namespace impl
    //{
        /// smart_ptr_basic, basic class for all smart pointers
        /// defines proper PointerType and ReferenceType
        /// defines proper disposer;
        ///      make sure const correct.
        ///      E.g.  smart_ptr<const int> p;  *p = 3; is not legal
        template<class T, template<class> class Disposer>
        class smart_ptr_basic
        {
#ifndef DISABLE_CONST_SMART_POINTER
            const static bool IsConstValue = xtl::TypeTraitsEx<T>::IsConstValue;
            typedef typename xtl::TypeTraitsEx<T>::UnconstValue UnConstT;

        public:
            /// Moves the const qualifier of T type to the pointer type
            typedef typename xtl::Select<IsConstValue,
                typename xtl::TypeTraitsEx<UnConstT>::ConstPointer,
                typename xtl::TypeTraitsEx<T>::PointerType>::Result PointerType;

            /// Moves the const qualifier of T type to the reference type
            typedef typename xtl::Select<IsConstValue,
                typename xtl::TypeTraitsEx<UnConstT>::ConstReference,
                typename xtl::TypeTraitsEx<T>::ReferenceType>::Result ReferenceType;

#else
        public:
            typedef T* PointerType;
            typedef T& ReferenceType;
#endif

        public:
            /// access members, e.g. ptr->member;
            PointerType operator->() const    { return m_ptr;}

            /// dereference the pointer, e.g. (*ptr).member;
            ReferenceType operator* () const    { return *m_ptr;}

            /// access the stored pointer, e.g. GetPtr(ptr);
            /// NOTE: this function is in GLOBAL namespace, not in xtl
            friend PointerType GetPtr(const smart_ptr_basic& ptr) { return ptr.m_ptr;}

            /// Enable "if(!ptr) { assert(ptr_is_not_valide);}"
            bool operator ! () const { return m_ptr == NULL;}

        protected:
            /// protected default ctor, always inherit
            smart_ptr_basic() : m_ptr(NULL) {}
            smart_ptr_basic(PointerType other) : m_ptr(other) {}

            /// The actual pointer to the object
            PointerType m_ptr;

            static Disposer<T> disposer;
        };

        template<class T, template<class> class Disposer>
        Disposer<T> smart_ptr_basic<T, Disposer>::disposer;

#ifndef DISABLE_USE_COUNTER_SMART_POOL
        /// The memory pool for use counter in shared pointer
        template<class CounterType, bool IsPod>
        xtl::SmartPool<CounterType, IsPod>& GetUseCountPool()
        {
            static xtl::SmartPool<CounterType, IsPod> thePool;
            return thePool;
        }
#endif

        /// implementation of reference counted pointer
        /// must be inherited.  defines logic of reference counting.
        /// include basic SharedPtr interface,
        ///      except implicit ptr convertion and comparison
        template<class T, template<class> class Disposer>
        class shared_ptr : public smart_ptr_basic<T, Disposer>
        {
        public:
            /// Reset to new pointer, release old one
            /// NOTE: this function is in GLOBAL namespace, not in xtl
            friend void ResetPtr(shared_ptr& ptr, PointerType other = NULL)
            {
                ptr.Release();
                ptr.m_ptr = other;
                ptr.AddRef();
            }

            /// NOTE: this function is in GLOBAL namespace, not in xtl
            friend void SwapPtr(shared_ptr& ptr, shared_ptr& other)
            {
                xtl::swap(ptr.m_ptr, other.m_ptr);
                xtl::swap(ptr.m_use, other.m_use);
            }

            /// Attach to given pointer, and never dispose it
            /// it can be used to use shared_ptr on static data, e.g.
            ///      static int x = 3;
            ///      shared_ptr<int> ptr;
            ///      xtl::AttachPtr(ptr, &x);
            /// NOTE: this function is in GLOBAL namespace, not in xtl
            friend void AttachPtr(shared_ptr& ptr, PointerType other)
            {
                ptr.Release();
                ptr.m_ptr = other;
                ptr.SetAliasing();
            }

        protected:
            shared_ptr() : m_use(NULL)
            {
            }

            ~shared_ptr()
            {
                Release();
            }

            /// allow implicit ctor to enable following assignment
            /// shared_ptr<SomeClass> ptr = new SomeClass;
            shared_ptr(PointerType other) : m_use(NULL)
            {
                m_ptr = other;
                AddRef();
            }

            /// copy ctor, copy pointer and counter as well
            shared_ptr(const shared_ptr& other)
            {
                m_ptr = other.m_ptr;
                m_use = other.m_use;
                AddRef();
            }

            /// copy assignment, lite copy, copy only pointer and counter
            shared_ptr& operator = (const shared_ptr& other)
            {
                Release();
                m_ptr = other.m_ptr;
                m_use = other.m_use;
                AddRef();
                return *this;
            }

            /// assign raw pointer
            shared_ptr& operator = (PointerType other)
            {
                Release();
                m_ptr = other;
                AddRef();
                return *this;
            }

        private:
            /// Make sure there is a counter and it's increased
            void AddRef()
            {
                if (IsCounting())
                {
                    assert(m_ptr != NULL);
                    ++ (*m_use);
                }
                else if (!IsAliasing() && m_ptr != NULL)
                {
                    m_use = CreateUseCounter();
                    if (m_use == NULL) return;     /// Should throw out of memory
                    (*m_use) = 1;
                }

                assert(m_ptr == NULL || m_use != NULL);
            }

            /// make sure the counter is decreased,
            /// After release, m_ptr, and m_use are guaranteed to be NULL
            void Release()
            {
                if (m_ptr != NULL)
                {
                    if (IsCounting())
                    {
                        assert((*m_use) > 0);
                        -- (*m_use);
                        if ((*m_use) == 0)
                        {
                            disposer.Dispose(m_ptr);
                            DestoryUseCounter(m_use);
                        }
                    }
                    m_use = NULL;
                    m_ptr = NULL;
                }
                else
                {
                    assert(m_use == NULL);
                }
            }

#ifdef DISABLE_USE_COUNTER_SMART_POOL
            static long* CreateUseCounter()
            {
                return new long;
            }

            static void DestoryUseCounter(long* ptr)
            {
                delete ptr;
            }
#else
            static long* CreateUseCounter()
            {
                return GetUseCountPool<long, true>().Create();
            }

            static void DestoryUseCounter(long* ptr)
            {
                GetUseCountPool<long, true>().Destroy(ptr);
            }
#endif

        private:
            /// reference counter.
            ///  1. == NULL, no counter and no aliasing, m_ptr == NULL
            ///  2. ==  0x1, no counter but aliasing. m_ptr might have something.
            ///              shared_ptr does not release the memory of object.
            ///  3. >   0x1, counting, and m_ptr must have something.
            ///              shared_ptr guaranteed to free the memory for the object
            long* m_use;

            void SetAliasing()      { m_use = (long*) 0x1;}
            bool IsAliasing() const { return (UINT_PTR) m_use == 0x1;}
            bool IsCounting() const { return (UINT_PTR) m_use >  0x1;}
        };


        template<class T, template<class> class Disposer>
        class scoped_ptr : public smart_ptr_basic<T, Disposer>
        {
        protected:
            /// the only way to assign the pointer, explicitly ctor
            explicit scoped_ptr(PointerType other)
                : smart_ptr_basic<T, Disposer>(other)
            {
            }

            ~scoped_ptr()
            {
                disposer.Dispose(m_ptr);
            }

            /// Reset to given new pointer, release previous one
            friend void ResetPtr(scoped_ptr& ptr, PointerType other = NULL)
            {
                ptr.disposer.Dispose(ptr.m_ptr);
                ptr.m_ptr = other;
            }

            /// release the ownership of the pointer
            /// replace it with NULL and return current one without delete
            /// The caller takes the ownership and should delete afterwards.
            friend PointerType DetachPtr(scoped_ptr& ptr)
            {
                PointerType result = ptr.m_ptr;
                ptr.m_ptr = NULL;
                return result;
            }

            /// prohibit copy assignment
        private:
            scoped_ptr();
            scoped_ptr(const scoped_ptr& other);
            scoped_ptr& operator = (const scoped_ptr& other);
        };

    //}   /// namespace impl
#pragma endregion

    /// ----------------------------------------------------------------
    /// various delete functors for pointer deletion in SharedPtr
    /// ----------------------------------------------------------------

    template<class T> struct disposer_null
    {
        void Dispose (const T* ptr) const {}
    };

    template<class T> struct disposer_delete
    {
        void Dispose (const T* ptr) const { if(ptr != NULL) delete ptr;}
    };

    template<class T> struct disposer_array
    {
        void Dispose (const T* ptr) const { if(ptr != NULL) delete[] ptr;}
    };

    template<class T> struct disposer_free
    {
        void Dispose (const T* ptr) const { if(ptr != NULL) free((void*)ptr);}
    };

    template<class T> struct disposer_aligned_free
    {
        void Dispose (const T* ptr) const { if(ptr != NULL) _aligned_free_dbg((void*)ptr);}
    };

    template<class T> struct disposer_release
    {
        void Dispose (T* ptr) const { if(ptr != NULL) ptr->Release();}
    };

    template<class T> struct disposer_fclose
    {
        void Dispose (FILE* fp) const { if(fp != NULL) fclose(fp);}
    };

    /// Define the comparison between SharedPtr or to raw pointer T*
    /// Include both template version and T* version
#define POINTER_COMPARISON(PTR, OP)                                         \
    template<class U> bool operator OP (const U* q) const                   \
        { return m_ptr OP q;}                                               \
    bool operator OP (const PTR& p) const                                   \
        { return m_ptr OP p.m_ptr;}                                         \
    bool operator OP (const PointerType q) const                            \
        { return m_ptr OP q;}                                               \
    template<class U> friend bool operator OP (const U* q, const PTR& p)    \
        { return p.m_ptr OP q;}                                             \
    friend bool operator OP (const PointerType q, const PTR& p)             \
        { return p.m_ptr OP q;}                                             \


    /// SharedPtr, the pointer can be shared by multiple owners
    /// It will dispose by itself when no owner references to it.
    template<class T, template<class> class Disposer = disposer_delete >
    class SharedPtr : public shared_ptr <T, Disposer>
    {
        typedef shared_ptr<T, Disposer> super;

    public:
        using super::operator->;
        using super::operator*;
        using super::operator!;

        IMPLICIT_CONVERT_SMARTPTR_TO_RAWPTR_POLICY;

    public:
        SharedPtr() {}
        SharedPtr(PointerType other) : super(other) {}
        SharedPtr(const SharedPtr& other) : super(other) {}

        SharedPtr& operator = (const SharedPtr& other)
        {
            super::operator = (other);
            return *this;
        }

        POINTER_COMPARISON(SharedPtr, ==);
        POINTER_COMPARISON(SharedPtr, !=);
        POINTER_COMPARISON(SharedPtr, <);
        POINTER_COMPARISON(SharedPtr, >);
        POINTER_COMPARISON(SharedPtr, <=);
        POINTER_COMPARISON(SharedPtr, >=);
    };

    /// SharedArray , a SharedPtr using delete[] as disposer
    template<class T>
    class SharedArray : public shared_ptr<T, disposer_array>
    {
        typedef shared_ptr<T, disposer_array> super;

    public:
        using super::operator->;
        using super::operator*;
        using super::operator!;

        IMPLICIT_CONVERT_SMARTPTR_TO_RAWPTR_POLICY;

    public:
        SharedArray() {}
        SharedArray(PointerType other) : super(other) {}
        SharedArray(const SharedArray& other) : super(other) {}

        SharedArray& operator = (const SharedArray& other)
        {
            super::operator = (other);
            return *this;
        }

        POINTER_COMPARISON(SharedArray, ==);
        POINTER_COMPARISON(SharedArray, !=);
        POINTER_COMPARISON(SharedArray, <);
        POINTER_COMPARISON(SharedArray, >);
        POINTER_COMPARISON(SharedArray, <=);
        POINTER_COMPARISON(SharedArray, >=);
    };

    /// ScopedPtr,  a smart pointer that disallows copy assignment
    /// the only way to assign a pointer is its explicit ctor
    /// It will dispose itself when the variable is out of its scope
    /// It cannot be copied, and therefore cannot go out of its scope
    template<class T, template<class> class Disposer = disposer_delete>
    class ScopedPtr : public scoped_ptr<T, Disposer>
    {
        typedef scoped_ptr<T, Disposer> super;

    public:
        using super::operator->;
        using super::operator*;
        using super::operator!;

        IMPLICIT_CONVERT_SMARTPTR_TO_RAWPTR_POLICY;

    public:
        /// the only way to assign the pointer, explicitly ctor
        explicit ScopedPtr(PointerType other) : super(other) {}

        POINTER_COMPARISON(ScopedPtr, ==);
        POINTER_COMPARISON(ScopedPtr, !=);
        POINTER_COMPARISON(ScopedPtr, <);
        POINTER_COMPARISON(ScopedPtr, >);
        POINTER_COMPARISON(ScopedPtr, <=);
        POINTER_COMPARISON(ScopedPtr, >=);
    };

    /// ScopedArray , a ScopedPtr using delete[] as disposer
    template<class T>
    class ScopedArray : public scoped_ptr<T, disposer_array>
    {
        typedef scoped_ptr<T, disposer_array> super;

    public:
        using super::operator->;
        using super::operator*;
        using super::operator!;

        IMPLICIT_CONVERT_SMARTPTR_TO_RAWPTR_POLICY;

    public:
        /// the only way to assign the pointer, explicitly ctor
        explicit ScopedArray(PointerType other) : super(other) {}

        POINTER_COMPARISON(ScopedArray, ==);
        POINTER_COMPARISON(ScopedArray, !=);
        POINTER_COMPARISON(ScopedArray, <);
        POINTER_COMPARISON(ScopedArray, >);
        POINTER_COMPARISON(ScopedArray, <=);
        POINTER_COMPARISON(ScopedArray, >=);
    };

#undef POINTER_COMPARISON
#undef IMPLICIT_CONVERT_SMARTPTR_TO_RAWPTR_POLICY

}   /// namespace xtl

#endif//__XTL_POINTER_HPP__
