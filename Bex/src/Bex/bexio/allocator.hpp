#ifndef __BEX_IO_ALLOCATOR_HPP__
#define __BEX_IO_ALLOCATOR_HPP__

//////////////////////////////////////////////////////////////////////////
// 内存分配器
/*
* @Concept:
*   Allocator
*/

// \get: new delete
#include <new>

// \get: malloc free
#include <cstdlib>

#include <boost/shared_ptr.hpp>

namespace Bex { namespace bexio
{
    // @todo: 内部改成内存池

    template <typename T>
    class allocator
    {
    public:
        typedef T value_type;
        typedef value_type* pointer;
        typedef const value_type* const_pointer;
        typedef value_type& reference;
        typedef value_type const& const_reference;
        typedef std::size_t size_type;
        typedef std::ptrdiff_t difference_type;

        template <typename U>
        struct rebind
        {
            typedef allocator<U> other;
        };

        //////////////////////////////////////////////////////////////////////////
        /// allocator construct
        allocator() throw() 
        {
        }

        allocator(const allocator<T>&) throw()
        {
        }
        
        template <typename U>
        allocator(const allocator<U>&) throw()
        {
        }

        template <typename U>
        allocator& operator=(const allocator<U>&) throw()
        {
            return (*this);
        }
        //////////////////////////////////////////////////////////////////////////

        pointer allocate(size_type size) const throw()
        {
            ::operator new(size);
        }

        pointer allocate(size_type size, const void *) const throw()
        {
            return (this->allocate(size));
        }

        void deallocate(void * ptr) const throw()
        {
            ::operator delete(ptr);
        }

        void deallocate(void * ptr, size_type) const throw()
        {
            this->deallocate(ptr);
        }

        void construct(T * ptr) const throw()
        {
            ::new ((void*)ptr) T();
        }

        template <typename Arg>
        void construct(T * ptr, Arg const& arg) const throw()
        {
            ::new ((void*)ptr) T(arg);
        }

        template <class U>
        void destroy(U * ptr) const throw()
        {
            ptr->~U();
        }
    };

    template <typename T, class Allocator>
    T * allocate()
    {
        typedef typename Allocator::template rebind<T>::type alloc_t;
        alloc_t alloc;
        T * pointer = alloc.allocate(sizeof(T));
        alloc.construct(pointer);
        return pointer;
    }

    template <typename T, class Allocator, typename Arg>
    T * allocate(Arg const& arg)
    {
        typedef typename Allocator::template rebind<T>::type alloc_t;
        alloc_t alloc;
        T * pointer = alloc.allocate(sizeof(T));
        alloc.construct(pointer, arg);
        return pointer;
    }

    template <typename T, class Allocator, typename Arg>
    T * allocate(Arg & arg)
    {
        typedef typename Allocator::template rebind<T>::type alloc_t;
        alloc_t alloc;
        T * pointer = alloc.allocate(sizeof(T));
        alloc.construct(pointer, arg);
        return pointer;
    }

    template <typename T, class Allocator, typename Arg1, typename Arg2, typename Arg3>
    T * allocate(Arg1 & arg1, Arg2 & arg2, Arg3 & arg3)
    {
        typedef typename Allocator::template rebind<T>::type alloc_t;
        alloc_t alloc;
        T * pointer = alloc.allocate(sizeof(T));
        alloc.construct(pointer, arg1, arg2, arg3);
        return pointer;
    }

    template <class Allocator, typename T>
    void deallocate(T * pointer)
    {
        typedef typename Allocator::template rebind<T>::type alloc_t;
        alloc_t alloc;
        alloc.destroy(pointer);
        alloc.deallocate(pointer);
    }

    template <typename T, class Allocator>
    struct deallocator
    {
        inline void operator()(T * pointer) const
        {
            deallocate<Allocator>(pointer);
        }
    };

    template <typename T, class Allocator>
    boost::shared_ptr<T> make_shared_ptr()
    {
        return boost::shared_ptr<T>(allocate<T, Allocator>(),
            deallocator<T, Allocator>(), Allocator());
    }

    template <typename T, class Allocator, typename Arg>
    boost::shared_ptr<T> make_shared_ptr(Arg const& arg)
    {
        return boost::shared_ptr<T>(allocate<T, Allocator>(arg),
            deallocator<T, Allocator>(), Allocator());
    }

    template <typename T, class Allocator, typename Arg>
    boost::shared_ptr<T> make_shared_ptr(Arg & arg)
    {
        return boost::shared_ptr<T>(allocate<T, Allocator>(arg),
            deallocator<T, Allocator>(), Allocator());
    }

    template <typename T, class Allocator, typename Arg1, typename Arg2, typename Arg3>
    boost::shared_ptr<T> make_shared_ptr(Arg1 & arg1, Arg2 & arg2, Arg3 & arg3)
    {
        return boost::shared_ptr<T>(allocate<T, Allocator>(arg1, arg2, arg3),
            deallocator<T, Allocator>(), Allocator());
    }

} //namespace bexio
} //namespace Bex

#endif //__BEX_IO_ALLOCATOR_HPP__