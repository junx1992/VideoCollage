#ifndef __XTL_CONTAINER_HPP__
#define __XTL_CONTAINER_HPP__

/*************************************************************************\
Oliver Liyin Copyright (c) 2003

Module Name:
    XTL containers, including 
    1. Intrusive linked lists
    2. Variable sized array

Usage:
    // ----------------------------------------------------------------------------------
    Intrusive linked lists include:
        1. List,  double linked list
        2. Deque, single linked list, can be FIFO, and FILO
        3. Queue, wrapper of Deque, FIFO
        3. Stack, wrapper of Deque, FILO
    All link list stores links (next/prev) in the ITEM intrusively.
        Therefore, no memory operation in these container,
        All items are allocated and deallocated by consumer.
        Containers do not care memory, and always assume itmes are valid.
        Therefore, clear the list before you deallocate all items from memory.
    Access policy defines how to access next pointer in ITEM.
        typically, it defines Next or/and Prev function, for example.
        struct AccessSomething
        {
            static ITEM*& Next(ITEM* ptr) { return ptr->next;}
            static ITEM*& Prev(ITEM* ptr) { return ptr->prev;}
        };
    Note: Only Next is required for single linked list, e.g. Deque, Queue and Stack
        Prev is required for double linked list for backword access, e.g. List
    Note: The containers can be initialized by memset(ptr, 0, sizeof());
        This is important when the container is a member of
        simple structure, which may be initialized without ctor
    Note: List is double linked list, e.g. List
            1. can push and pop from both side
            2. can erase any item in const time
            3. can insert at any position in const time.
        However, a single linked list, e.g. Deque,
            1. can NOT pop_back
            2. can NOT erase
            3. can NOT iterate backward
    Note: All linked lists assume the Next and/or Prev pointer are init as NULL
        Therefore, it will be checked when Push(), and cleared when Pop() and Clear().
        It's dangous to push same item twice, which will break the linkage.
        Use Contains(ptr) to check if the item is already in list.
    // ----------------------------------------------------------------------------------


History:
    Created  on 2004 Jun 10 by oliver_liyin

\*************************************************************************/


#include "xtl/Xtl_Memory.hpp"


namespace xtl
{
    /// Array: Continuous memory block for array of values
    ///        The values should be simple copiable value, struct or value class
    ///        Dynamically increase memory and encapsulate memory management.
    template<class T>
    class Array
    {
    public:
        typedef T ITEM;
        typedef ITEM* PTR;
        typedef const ITEM* CPTR;

        void Resize(size_t);
        void Reserve(size_t);
        size_t Size() const;
        T& operator[] (size_t);
        const T& operator[] (size_t) const;

        PTR Front() const;
        BOOL NotEnd(PTR) const;
        PTR Next(PTR) const;

    private:
        size_t Count, Capacity;
        T* Head;
    };


    /// List: Double linked intrusive list
    ///
    /// Usage:
    ///     List list;              /// Ctor default List
    ///     list.PushFront(ptr);    /// push one ptr in List
    ///     ptr = list.Back();      /// get the top ptr, do not pop
    ///     ptr = list.PopBack();   /// pop the top ptr, and return
    ///     list.Erase(ptr);        /// erase the ptr from List
    ///     list.Contains(ptr);     /// query if ptr in List
    ///     if(list.IsEmpty())      /// query if List is empty
    ///     for(ptr = list.Front(); list.NotEnd(ptr); list.ToNext(ptr))
    ///     {
    ///         /// for each ptr in list, forward iterate
    ///     }
    ///     for(ptr = list.Back(); list.NotEnd(ptr); list.ToPrev(ptr));
    ///     {
    ///         /// for each ptr in list, backward iterate
    ///     }
    ///     while(!list.IsEmpty())
    ///     {
    ///         ptr = list.PopBack();
    ///         /// iterate and pop all ptrs in order,
    ///     }
    template<class T, class Access>
    class List
    {
    public:
        typedef T ITEM;
        typedef ITEM* PTR;
        typedef const ITEM* CPTR;

        /// Default ctor, it can substituted with memset(this, 0, sizeof(Deque))
        List() : Head(NULL), Tail(NULL), Count(0) 
        {
            COMPILE_TIME_ASSERT(sizeof(List) == sizeof(Head) + sizeof(Tail) + sizeof(Count));
        }

        /// Push item to the tail side of list
        void PushBack(PTR ptr)
        {
            assert(ptr != NULL);        /// Assume ptr is valid
            assert(!Contains(ptr));     /// must not be in list, otherwise, break the link

            if(IsEmpty())
            {   /// push the first ptr in list
                Head = Tail = ptr;
            }
            else
            {   /// push it at tail, grow to next of tail
                Access::Next(Tail) = ptr;
                Access::Prev(ptr) = Tail;
                Tail = ptr;
            }
            ++ Count;
        }

        /// Push item to the head side of list
        void PushFront(PTR ptr)
        {
            assert(ptr != NULL);        /// Assume ptr is valid
            assert(!Contains(ptr));     /// must not be in list, otherwise, break the link

            if(IsEmpty())
            {   /// push the first ptr in list
                Head = Tail = ptr;
            }
            else
            {   /// push it at head, grow to prev of head
                Access::Next(ptr) = Head;
                Access::Prev(Head) = ptr;
                Head = ptr;
            }
            ++ Count;
        }

        /// Pop the first item in the list
        PTR PopFront()
        {
            assert(!IsEmpty());
            PTR Result = Head;
            if(Head == Tail)
            {
                assert(Count == 1);
                Head = Tail = NULL;
            }
            else
            {
                Head = Access::Next(Head);
                Access::Prev(Head) = NULL;
            }
            assert(Access::Prev(Result) == NULL);
            Access::Next(Result) = NULL;
            -- Count;
            return Result;
        }

        /// Pop the last item in the list
        PTR PopBack()
        {
            assert(!IsEmpty());
            PTR Result = Tail;
            if(Head == Tail)
            {   
                assert(Count == 1);
                Head = Tail = NULL;
            }
            else
            {
                Tail = Access::Prev(Tail);
                Access::Next(Tail) = NULL;
            }
            assert(Access::Next(Result) == NULL);
            Access::Next(Result) = NULL;
            -- Count;
            return Result;
        }

        /// erase the given pointer from list, if it's in list
        void Erase(PTR ptr)
        {
            assert(ptr != NULL);

            if(!Contains(ptr)) return;            
            if(Head == ptr) Head = Access::Next(ptr);
            if(Tail == ptr) Tail = Access::Prev(ptr);
            if(Access::Next(ptr) != NULL) Access::Prev(Access::Next(ptr)) = Access::Prev(ptr);
            if(Access::Prev(ptr) != NULL) Access::Next(Access::Prev(ptr)) = Access::Next(ptr);
            Access::Next(ptr) = Access::Prev(ptr) = NULL;
            -- Count;
        }

        /// Query the first item in the list
        /// Can be used to iterate the list
        PTR Front() const
        {
            return Head;
        }

        /// Query the last item in the list
        /// Can be used to iterate the list
        PTR Back() const
        {
            return Tail;
        }

        /// The given pointer is not end, can continue to iterate
        BOOL NotEnd(PTR ptr) const
        {
            ptr != NULL;
        }

        /// Move the pointer to next item
        void ToNext(PTR& p) const
        {
            p = Access::Next(p);
        }
        
        /// Move the pointer to previous item
        void ToPrev(PTR& p) const
        {
            p = Access::Prev(p);
        }

        /// Query the number of items in the list
        size_t Size() const
        {
            return Count;
        }

        /// Query if ptr is in the list,
        /// It assumes ptr->next and ptr->prev has been initialized by NULL
        bool Contains(PTR ptr) const
        {
            assert(ptr != NULL);
            /// ptr is in list when ptr is tail, or ptr->next is not null
            return (Access::Next(ptr) != NULL) || (ptr == Tail);
        }

        /// Query if there is no item in the deque
        bool IsEmpty() const
        {
            assert((Head == NULL) == (Tail == NULL));
            assert((Head == NULL) == (Count == 0));
            return Count == 0;
        }

        /// Clear all items in deque, make deque empty
        /// It will clear the Next pointer one by one
        void Clear(bool resetNextPtr = false)
        {
            if (resetNextPtr)
            {
                while(Head != NULL)
                {
                    PTR ptr = Head;
                    Head = Access::Next(Head);  /// pop front
                    Access::Next(ptr) = Access::Prev(ptr) = NULL;
                };
            }
            Head = Tail = NULL;
            Count = 0;
        }

    private:
        size_t Count;
        PTR Head;
        PTR Tail;
    };

    /// Deque: Intrusive single linked list, can be FIFO, or FILO
    ///        can PushBack() and PushFront(), but can only PopFront()
    ///        there is no Back() or PopBack();
    /// Usage:
    ///     Deque deque;            /// Ctor default
    ///     deque.PushFront(ptr);   /// push one ptr in queue
    ///     ptr = deque.Front();    /// get the top ptr, do not pop
    ///     ptr = deque.PopFront(); /// pop the top ptr, and return
    ///     deque.Contains(ptr);    /// query if ptr in queue
    ///     if(deque.IsEmpty())     /// query if deque is empty
    ///     deque.Clear();          /// clear all ptrs in queue
    ///     for(ptr = deque.Front(); deque.NotEnd(ptr); deque.ToNext(ptr))
    ///     {
    ///         /// for each ptr in queue, forward iterate
    ///     }
    ///     while(!deque.IsEmpty())
    ///     {
    ///         ITEM* ptr = deque.Pop();
    ///         /// iterate and pop all ptrs in order,
    ///     }
    /// Note:
    ///     Tail is not used for Stack like usage, i.e. only PushFront and PopFront.
    ///     However, Tail is important to know whether a pointer is contained in list.
    ///     The item at tail behaves exactly as free node, unless we know it's the tail.
    ///
    template<class T, class Access>
    struct Deque
    {
    public:
        typedef T ITEM;
        typedef ITEM* PTR;
        typedef const ITEM* CPTR;

        /// Default ctor, it can substituted with memset(this, 0, sizeof(Deque))
        Deque() : Head(NULL), Tail(NULL), Count(0) 
        {
            COMPILE_TIME_ASSERT(sizeof(Deque) == sizeof(Head) + sizeof(Tail) + sizeof(Count));
        }

        /// Push item to the tail side of list
        /// Usually used as Queue, FIFO
        void PushBack(PTR ptr)
        {
            assert(ptr != NULL);        /// Assume ptr is valid
            assert(!Contains(ptr));     /// must not be in list, otherwise, break the link

            if(IsEmpty())
            {   /// push the first ptr in list
                Head = ptr;
            }
            else
            {   /// push it at tail, grow to next of tail
                Access::Next(Tail) = ptr;
            }
            Tail = ptr;
            ++ Count;
        }

        /// Push item to the head side of list
        /// Ususally used as Stack, FILO
        void PushFront(PTR ptr)
        {
            assert(ptr != NULL);        /// Assume ptr is valid
            assert(!Contains(ptr));     /// must not be in list, otherwise, break the link

            if(IsEmpty())
            {   /// push the first ptr in list
                Tail = ptr;
            }
            else
            {   /// push it at tail, grow to next of tail
                Access::Next(ptr) = Head;
            }
            Head = ptr;
            ++ Count;
        }

        /// Pop the first item in the deque
        PTR PopFront()
        {
            assert(!IsEmpty());
            PTR Result = Head;
            if(Head == Tail)
            {   
                assert(Count == 1);
                Head = Tail = NULL;
            }
            else
            {
                Head = Access::Next(Head);
            }
            Access::Next(Result) = NULL;
            -- Count;
            return Result;
        }

        /// Query the first item in the deque
        /// Can be used to iterate the list
        PTR Front() const
        {
            return Head;
        }

        /// Query the last item in the deque.
        /// However, since it's single linked list
        /// Back() can NOT be be used to iterate without Prev
        PTR Back() const
        {
            return Tail;
        }

        /// The given pointer is not end, can continue to iterate
        BOOL NotEnd(PTR ptr) const
        {
            return ptr != NULL;
        }

        /// Move the pointer to next item
        void ToNext(PTR& p)
        {
            p = Access::Next(p);
        }

        /// Query the number of items in the deuqe
        size_t Size() const
        {
            return Count;
        }

        /// Query if ptr is in the deque,
        /// It assumes ptr->next has been initialized by NULL
        bool Contains(PTR ptr) const
        {
            assert(ptr != NULL);
            /// ptr is in deque when ptr is tail, or ptr->next is not null
            return (Access::Next(ptr) != NULL) || (ptr == Tail);
        }

        /// Query if there is no item in the deque
        bool IsEmpty() const
        {
            assert((Head == NULL) == (Tail == NULL));
            assert((Head == NULL) == (Count == 0));
            return Count == 0;
        }

        /// Clear all items in deque, make deque empty
        /// Usually used before deallocate all items
        void Clear(bool resetNextPtr = false)
        {
            if (resetNextPtr)
            {
                while(Head != NULL)
                {
                    PTR ptr = Head;
                    Head = Access::Next(Head);
                    Access::Next(ptr) = NULL;
                };
            }
            Head = Tail = NULL;
            Count = 0;
        }

    private:
        size_t Count;
        PTR Head;
        PTR Tail;
    };

    /// Stack: Intrusive single linked list, works as stack FILO
    ///        Push() is push_front() and Pop() is pop_front()
    template<class T, class Access>
    class Stack : private Deque<T, Access>
    {
        typedef Deque<T, Access> super;
    public:
        using super::ITEM;
        using super::PTR;
        using super::CPTR;
        using super::Size;
        using super::IsEmpty;
        using super::Front;
        using super::NotEnd;
        using super::ToNext;
        using super::Clear;
        using super::Contains;

        void Push(PTR ptr)
        {
            super::PushFront(ptr);
        }

        PTR Pop()
        {
            return super::PopFront();
        }
    };

    /// Queue: Intrusive single linked list, works as stack FIFO
    ///        Push() is push_back() and Pop() is pop_front()
    template<class T, class Access>
    class Queue : private Deque<T, Access>
    {
        typedef Deque<T, Access> super;
    public:
        using super::ITEM;
        using super::PTR;
        using super::CPTR;
        using super::Size;
        using super::IsEmpty;
        using super::Front;
        using super::NotEnd;
        using super::ToNext;
        using super::Clear;
        using super::Contains;

        void Push(PTR ptr)
        {
            super::PushBack(ptr);
        }

        PTR Pop()
        {
            return super::PopFront();
        }
    };

}   // namespace xtl

#endif//__XTL_CONTAINER_HPP__
