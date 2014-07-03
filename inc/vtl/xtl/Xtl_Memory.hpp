#ifndef __XTL_MEMORY_HPP__
#define __XTL_MEMORY_HPP__

/*************************************************************************\
Oliver Liyin Copyright (c) 2003

Module Name:
    XTL Memory support

Abstract:
    1. SmartAllocator, make smarter, faster heap allocation for small blocks

    2. CppAllocator, wrapper of Std C++ Lib ::new and ::delete, 

    3. MallocAllocator, wrapper of C style malloc and free.

    4. AlignedAllocator<size_t alignment>, wrapper of _aligned_malloc

Notes:
    1. Support win32 only


Usage:

History:
    Created  on 2004 June 8 by oliver_liyin

\*************************************************************************/

#include <utility>
#include <crtdbg.h>
#include <malloc.h>

#include "Xtl_Utility.hpp"

/// When the block size is larger than MAX_BLOCK_WIDTH
/// use this allocator to allocate memory
/// It cannot be SmartAllocator
#ifndef DEFAULT_ALLOCATOR
#   define DEFAULT_ALLOCATOR xtl::CppAllocator
#endif // DEFAULT_ALLOCATOR


/// To disable compile warning C4291: no matching operator delete found
/// 1. When ctor throw in MFC DEBUG mode, the following might be invoked:
///    static void operator delete(void* p, const char * szFileName, int nLine);
/// 2. This function is too expensive for SmartAllocator.
///    Therefore you should never throw exceptions in ctor
/// 3. By disabling this warning, no delete code is generated by compiler.
///    Hence, there must be a memeory leakage for corresponding new operator
///    which throws.
/// 4. This leakage does not exists since compiler will give a
///    default delete for corresponding new
/// 5. By disabling this warning, we can avoid the delete function in (1),
///    which is very expensive for SmartAllocator.
#if defined(_DEBUG)
#   pragma warning(disable:4291)
#endif

/// To track memory in debug mode
/// if in debug mode, expand it as ", a, b"
/// if in release, ignore a, b
/// NOTE: there should be NO comma "," ahead of this macro
/// For example: Something.Allocate(16 _DEBUG_ONLY_AB(__FILE__, __LINE));
#if defined(_DEBUG)
#   define _DEBUG_ONLY_AB(a, b) , a, b
#else
#   define _DEBUG_ONLY_AB(a, b)
#endif

namespace xtl
{

/// undefine the debug version of new operator, which is a MACRO with 3 param 
#pragma push_macro("new")
#undef new

    /// CppAllocator policy
    /// wrap allocation to system new and delete
    /// NOTE:
    ///   it's 6% slower to use std::nothrow version new and delete
    ///   throw version is faster.
    class CppAllocator
    {
    public:
        static void* Allocate(size_t size
            _DEBUG_ONLY_AB(const char* szFileName = __FILE__, int nLine = __LINE__) )
        {
#ifdef DEBUG_NEW
            /// ::operator new(size_t, const TCHAR*, int); is defined in MFC
            return ::operator new(size _DEBUG_ONLY_AB(szFileName, nLine));
#else
            /// ::operator new(size_t, int, const TCHAR*, int); is defined in <crtdbg.h>
    #ifdef _DEBUG
            return ::operator new(size, _NORMAL_BLOCK, szFileName, nLine);
    #else
            return ::operator new(size);
    #endif
#endif
        }

        static void Deallocate(void *p, size_t /*size*/)
        {
            if (p == NULL) return;
            ::operator delete(p);
        }

    private:
        /// no one should create instance of CppAllocator
        CppAllocator();
        CppAllocator(CppAllocator&);
        ~CppAllocator();
    };

    /// MallocAllocator policy
    /// wrap allocation to system malloc and free
    class MallocAllocator
    {
    public:
        static void* Allocate(size_t size
            _DEBUG_ONLY_AB(const char* szFileName = __FILE__, int nLine = __LINE__) )
        {
            return _malloc_dbg(size, _NORMAL_BLOCK, szFileName, nLine);
        }

        static void Deallocate(void *p, size_t /*size*/)
        {
            if (p == NULL) return;
            _free_dbg(p, _NORMAL_BLOCK);
        }

    private:
        /// no one should create instance of MallocAllocator
        MallocAllocator();
        MallocAllocator(const MallocAllocator&);
        ~MallocAllocator();
    };

    /// AlignedAllocator policy
    /// wrap aligned_malloc, ensure the allocated ptr is aligned
    template<size_t alignment>
    class AlignedAllocator
    {
        COMPILE_TIME_ASSERT(
            alignment == 1 ||
            alignment == 2 ||
            alignment == 4 ||
            alignment == 8 ||
            alignment == 16||
            alignment == 32 );

    public:
        const static size_t uAlignment = alignment;
        const static unsigned int uAlignmentMask = (1 << (unsigned int)alignment) - 1;

        static void* Allocate(size_t size
            _DEBUG_ONLY_AB(const char* szFileName = __FILE__, int nLine = __LINE__) )
        {
	        return _aligned_malloc_dbg(size, alignment, szFileName, nLine);
        }

        static void Deallocate(void *p, size_t /*size*/)
        {
            if (p == NULL) return;
            _aligned_free_dbg(p);
        }

    private:
        /// no one should create instance of AlignedAllocator
        AlignedAllocator();
        AlignedAllocator(const AlignedAllocator&);
        ~AlignedAllocator();
    };

    namespace impl
    {
        template<class T, bool IsPod>
        class PoolHelper
        {
        protected:
            PoolHelper() {}

            template<bool> void Ctor(T*);
            template<bool> void Dtor(T*);

            /// PoD type does not need Ctor and Dtor
            template<> void Ctor<true>(T*) {}
            template<> void Dtor<true>(T*) {}

            /// Initialize none-Pod type using inplace ctor
            template<> void Ctor<false>(T* p)
            {
                /// new T() will zero the buffer
                new (p) T();
            }

            /// Finalize none-Pod type using dtor
            template<> void Dtor<false>(T* p)
            {
                UNREFERENCED_PARAMETER(p);
                (p)->~T();
            }

        private:
            /// Disable copy ctor and assign copy
            PoolHelper(const PoolHelper&);
            PoolHelper& operator = (const PoolHelper&);
        };

    }// namespace impl


    /// BlockPool is a class to allocate blocks
    /// It different from SmartPool that it can only create without destroy
    /// You can only clear entire pool.  Cannot remove any one in it.
    /// Create function allocate a block in a sequencial way.
    /// Reset function deallocate all blocks.
    /// If T is PoD, no ctor or dtor will be called
    template<class T, bool IsPod>
    class BlockPool : impl::PoolHelper<T, IsPod>
    {
    public:
        /// construct the pool using number of items in each block
        /// note that the block width is always the size of T
        BlockPool(size_t blockHeight = DEFAULT_BLOCK_HEIGHT)
            : m_blockHeight(blockHeight)
            , m_firstBlock(NULL), m_lastBlock(NULL)
        {
        }

        ~BlockPool()
        {
            Clear();
        }

        /// allocate one item
        T* Create()
        {
            if (m_lastBlock == NULL ||
                m_lastBlock->nextItem == m_lastBlock->blockEnd)
            {
                if(!GrowBlock()) return NULL;
            }

            assert(m_firstBlock != NULL && m_lastBlock != NULL &&
                m_lastBlock->nextItem >= m_lastBlock->items &&
                m_lastBlock->nextItem < m_lastBlock->blockEnd);

            T* p = m_lastBlock->nextItem ++;
            Ctor<IsPod>(p);
            return p;
        }

        /// release all blocks and deallocate memory
        void Clear()
        {
            DestroyItems<IsPod>();

            while (m_firstBlock != NULL)
            {
                Block* block = m_firstBlock;
                m_firstBlock = block->nextBlock;
                delete block;
            }

            m_firstBlock = m_lastBlock = NULL;
        }

    public:
        struct Block;

        /// Iterate the pool
        struct iterator
        {
        public:
            bool operator == (const iterator& it) const
            {
                return m_current == it.m_current;
            }

            bool operator != (const iterator& it) const
            {
                return m_current != it.m_current;
            }

            T* operator->() { return m_current;}
            T& operator*() { return *m_current;}

            void operator ++ ()
            {
                assert(m_current != NULL);
                ++ m_current;
                if (m_current == m_block->nextItem)
                {
                    m_block = m_block->nextBlock;
                    m_current = (m_block != NULL) ? &m_block->items[0] : NULL;
                }

                assert(m_current == NULL || (m_block != NULL && 
                    m_current >= m_block->items && m_current < m_block->nextItem));
            }

        private:
            friend class BlockPool;
            Block* m_block;
            T* m_current;
        };

        iterator end()
        {
            iterator it;
            it.m_current = NULL;
            return it;
        }

        iterator begin()
        {
            if (m_firstBlock == NULL)
            {
                return end();
            }
            else
            {
                iterator it;
                it.m_block = m_firstBlock;
                it.m_current = &m_firstBlock->items[0];
                return it;
            }
        }

    private:
        BOOL GrowBlock()
        {
            /// allocate the new block, raw byte buffer
            const size_t blockSize = sizeof(Block) + (m_blockHeight-1) * sizeof(T);
            Block* newBlock = (Block*) new BYTE[blockSize];
            if (newBlock == NULL) return FALSE;

            /// add new block to tail of list
            if (m_lastBlock == NULL)
            {
                /// when the list is empty
                assert(m_firstBlock == NULL);
                m_firstBlock = m_lastBlock = newBlock;
            }
            else
            {
                m_lastBlock->nextBlock = newBlock;
                m_lastBlock = newBlock;
            }

            /// init a new block
            m_lastBlock->nextItem = newBlock->items;
            m_lastBlock->blockEnd = newBlock->items + m_blockHeight;
            m_lastBlock->nextBlock = NULL;
            return TRUE;
        }

        template<bool>
        void DestroyItems();

        template<>
        void DestroyItems<true>()
        {
        }

        template<>
        void DestroyItems<false>()
        {
            Block* block = m_firstBlock;
            while (block != NULL)
            {
                T* p = block->items;
                for (size_t i = 0; i < m_blockHeight; i ++)
                {
                    Dtor<IsPod>(p++);
                }
                block = block->nextBlock;
            }
        }

    private:
        struct Block
        {
            Block* nextBlock;
            T* nextItem;
            T* blockEnd;
            T items[1];
        };

        const size_t m_blockHeight;

        Block* m_firstBlock;
        Block* m_lastBlock;
    };

#pragma pop_macro("new")

}   /// namespace xtl

#endif//__XTL_MEMORY_HPP__
