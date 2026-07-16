#ifndef TUPLE_INCLUDED
#define TUPLE_INCLUDED

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <algorithm>
#include <memory>
#include <type_traits>
#include <utility>

//
// Wrapper for a simple array
//
template <class T>
class Array
{
    int size;
    T *info;

public:
    Array() : size(0),
              info(NULL)
    {}

    explicit Array(int size_) : size(size_)
    {
        assert(size >= 0);
        info = size <= 0 ? nullptr : new T[static_cast<size_t>(size)];
    }

    Array(int size_, T value) : size(size_)
    {
        assert(size >= 0);
        info = size <= 0 ? nullptr : new T[static_cast<size_t>(size)];
        Initialize(value);

        return;
    }

    Array(const Array<T>& rhs) : size(rhs.size),
                                 info(size <= 0 ? nullptr : new T[static_cast<size_t>(size)])
    {
        for (int i = 0; i < size; ++i)
            info[i] = rhs.info[i];
    }

    Array(Array<T>&& rhs) noexcept : size(rhs.size),
                                     info(rhs.info)
    {
        rhs.size = 0;
        rhs.info = nullptr;
    }

    ~Array()
    {
        delete [] info;
    }

    Array<T>& operator=(const Array<T>& rhs)
    {
        if (this != &rhs)
        {
            Array<T> copy(rhs);
            Swap(copy);
        }
        return *this;
    }

    Array<T>& operator=(Array<T>&& rhs) noexcept
    {
        if (this != &rhs)
        {
            delete [] info;
            size = rhs.size;
            info = rhs.info;
            rhs.size = 0;
            rhs.info = nullptr;
        }
        return *this;
    }

    void Swap(Array<T>& rhs) noexcept
    {
        std::swap(size, rhs.size);
        std::swap(info, rhs.info);
    }

    void Initialize(const T& init_value)
    {
        for (int i = 0; i < size; i++)
            info[i] = init_value;
    }

    void Resize(int new_size)
    {
        assert(new_size >= 0);
        if (new_size == size)
            return;

        std::unique_ptr<T[]> new_info(
            new_size <= 0 ? nullptr : new T[static_cast<size_t>(new_size)]);
        const int copy_size = std::min(size, new_size);
        for (int i = 0; i < copy_size; ++i)
            new_info[i] = info[i];

        delete [] info;
        info = new_info.release();
        size = new_size;
    }

    void Reallocate(int new_size)
    {
        assert(new_size >= 0);
        if (new_size != size)
        {
            delete [] info;
            info = new_size <= 0 ? nullptr : new T[static_cast<size_t>(new_size)];
        }
        size = new_size;
    }

    //
    // Set all fields in the array to val.
    //
    void Memset(int val = 0)
    {
        static_assert(std::is_trivially_copyable<T>::value,
                      "Array::Memset requires a trivially copyable type");
        memset(info, val, size * sizeof(T));
    }

    //
    // Set all field in the array to 0 starting at index i.
    //
    void MemReset(int i = 0)
    {
        static_assert(std::is_trivially_copyable<T>::value,
                      "Array::MemReset requires a trivially copyable type");
        if (size == 0)
            return;

        //
        // Use this code when debugging to force a crash instead of
        // allowing the assertion to be raised.
        //
        //if (i < 0 || i >= size)
        //{
        //    Array<T> *a = NULL;
        //    a -> size = 0;
        //}
        assert(! OutOfRange(i));

        int length = size - i;
        memset(&info[i], 0, length * sizeof(T));
    }

    int Size() const { return size; }

    //
    // Can the array be indexed with i?
    //
    inline bool OutOfRange(const int i) const { return (i < 0 || i >= size); }

    //
    // Return the ith element of the array
    //
    T &operator[](const int i)
    {
        //
        // Use this code when debugging to force a crash instead of
        // allowing the assertion to be raised.
        //
        //if (i < 0 || i >= size)
        //{
        //    Array<T> *a = NULL;
        //    a -> size = 0;
        //}
        assert(! OutOfRange(i));

        return info[i];
    }

    const T &operator[](const int i) const
    {
        assert(! OutOfRange(i));
        return info[i];
    }
};

//
// This Tuple template class can be used to construct a dynamic
// array of arbitrary objects. The space for the array is allocated
// in blocks of size 2**LOG_BLKSIZE. In declaring a tuple the user
// may specify an estimate of how many elements he expects. Based
// on that estimate, suitable values are calculated for log_blksize
// and base_increment. If these estimates are found to be off later,
// more space will be allocated.
//
template <class T>
class Tuple
{
protected:

    enum { DEFAULT_LOG_BLKSIZE = 3, DEFAULT_BASE_INCREMENT = 4 };

    Array<T *> base;
    int top,
        size;

    int log_blksize,
        base_increment;

    inline int Blksize() const { return (1 << log_blksize); }
    inline int BlockOffset(int i) const { return i & (Blksize() - 1); }

    //
    // Allocate another block of storage for the dynamic array.
    //
    inline void AllocateMoreSpace()
    {
        //
        // The variable size always indicates the maximum number of
        // elements that has been allocated for the array.
        // Initially, it is set to 0 to indicate that the array is empty.
        // The pool of available elements is divided into segments of size
        // 2**log_blksize each. Each segment is pointed to by a slot in
        // the array base.
        //
        // By dividing size by the size of the segment we obtain the
        // index for the next segment in base. If base is full, it is
        // reallocated.
        //
        //
        int k = size >> log_blksize; /* which segment? */

        //
        // If the base is overflowed, reallocate it and initialize the new elements to NULL.
        //
        if (k == base.Size())
        {
            base.Resize(k == 0 ? base_increment : k * 2);
            base.MemReset(k); // initialize the new base elements
        }

        //
        // Keep the real allocation address. Older versions adjusted this
        // pointer backwards by `size`, which formed an out-of-bounds pointer
        // before the allocation and was undefined behavior.
        //
        base[k] = new T[Blksize()];

        //
        // Finally, we update SIZE.
        //
        size += Blksize();

        return;
    }

public:

    //
    // This function is invoked with an integer argument n. It ensures
    // that enough space is allocated for n elements in the dynamic array.
    // I.e., that the array will be indexable in the range  (0..n-1)
    //
    // Note that this function can be used as a garbage collector.  When
    // invoked with no argument(or 0), it frees up all dynamic space that
    // was allocated for the array.
    //
    inline void Resize(const int n = 0)
    {
        //if (n < 0)
        //{
        //    Tuple<T> *p = NULL;
        //    p -> top = 0;
        //}
        assert(n >= 0);

        //
        // If array did not previously contain enough space, allocate
        // the necessary additional space. Otherwise, if the array had
        // more blocks than are needed, release the extra blocks.
        //
        if (n > size)
        {
            do
            {
                AllocateMoreSpace();
            } while (n > size);
        }
        else if (n < size)
        {
            // slot is the index of the base element whose block
            // will contain the (n-1)th element.
            int slot = (n <= 0 ? -1 : (n - 1) >> log_blksize);

            for (int k = (size >> log_blksize) - 1; k > slot; k--)
            {
                size -= Blksize();
                delete [] base[k];
                base[k] = NULL;
            }

            if (slot < 0)
                base.Resize(0);
        }

        top  = n;
    }

    //
    // This function is used to reset the size of a dynamic array without
    // allocating or deallocting space. It may be invoked with an integer
    // argument n which indicates the new size or with no argument which
    // indicates that the size should be reset to 0.
    //
    inline void Reset(const int n = 0)
    {
        //
        // Use this code when debugging to force a crash instead of
        // allowing the assertion to be raised.
        //
        // if (! (n >= 0 && n <= top))
        // {
        //     Tuple<T> *p = NULL;
        //     p -> top = 0;
        // }
        //
        assert(n >= 0 && n <= top);

        top = n;
    }

    //
    // Return length of the dynamic array.
    //
    inline int Length() const { return top; }

    //
    // Can the tuple be indexed with i?
    //
    inline bool OutOfRange(const int i) const { return (i < 0 || i >= top); }

    //
    // Return a reference to the ith element of the dynamic array.
    //
    // Note that no check is made here to ensure that 0 <= i < top.
    // Such a check might be useful for debugging and a range exception
    // should be thrown if it yields true.
    //
    inline T& operator[](const int i)
    {
        //
        // Use this code when debugging to force a crash instead of
        // allowing the assertion to be raised.
        //
        //if (! (i >= 0 && i < top))
        //{
        //    Tuple<T> *p = NULL;
        //    p -> top = 0;
        //}
        assert(! OutOfRange(i));

        return base[i >> log_blksize][BlockOffset(i)];
    }

    inline const T& operator[](const int i) const
    {
        assert(! OutOfRange(i));
        return base[i >> log_blksize][BlockOffset(i)];
    }

    //
    // Add an element to the dynamic array and return the top index.
    //
    inline int NextIndex()
    {
        int i = top++;
        if (i == size)
            AllocateMoreSpace();
        return i;
    }

    inline void Push(T elt) { this -> Next() = elt; }

    //
    // Not "return (*this)[--top]" because that may violate an invariant
    // in operator[].
    //
    inline T Pop() { assert(top!=0); top--; return base[top >> log_blksize][BlockOffset(top)]; }
    inline T Top() { assert(top!=0); return (*this)[top-1]; }

    //
    // Add an element to the dynamic array and return a reference to
    // that new element.
    //
    inline T& Next() { int i = NextIndex(); return base[i >> log_blksize][BlockOffset(i)]; }

    //
    // Assignment of a dynamic array to another.
    //
    inline Tuple<T>& operator=(const Tuple<T>& rhs)
    {
        if (this != &rhs)
        {
            Resize(rhs.top);
            for (int i = 0; i < rhs.top; i++)
                (*this)[i] = rhs[i];
        }

        return *this;
    }

    //
    // Equality comparison of a dynamic array to another.
    //
    bool operator==(const Tuple<T>& rhs) const
    {
        if (this != &rhs)
        {
            if (this -> top != rhs.top)
                return false;
            for (int i = 0; i < rhs.top; i++)
            {
                if ((*this)[i] != rhs[i])
                    return false;
            }
        }
        return true;
    }

    //
    // Constructor of a Tuple
    //
    Tuple(unsigned estimate = 0)
    {
        if (estimate == 0)
        {
            log_blksize = DEFAULT_LOG_BLKSIZE;
            base_increment = DEFAULT_BASE_INCREMENT;
        }
        else
        {
            for (log_blksize = 1; (((unsigned) 1 << log_blksize) < estimate) && (log_blksize < 31); log_blksize++)
                ;
            if (log_blksize <= DEFAULT_LOG_BLKSIZE)
                base_increment = 1;
            else if (log_blksize < 13)
            {
                base_increment = (unsigned) 1 << (log_blksize - 4);
                log_blksize = 4;
            }
            else
            {
                base_increment = (unsigned) 1 << (log_blksize - 8);
                log_blksize = 8;
            }
            base_increment++; // add a little margin to avoid reallocating the base.
        }

        size = 0;
        top = 0;
    }

    //
    // Constructor of a Tuple
    //
    Tuple(int log_blksize_, int base_increment_) : log_blksize(log_blksize_),
                                                   base_increment(base_increment_)
    {
        size = 0;
        top = 0;
    }

    //
    // Initialization of a dynamic array.
    //
    Tuple(const Tuple<T>& rhs) : log_blksize(rhs.log_blksize),
                                 base_increment(rhs.base_increment)
    {
        size = 0;
        top = 0;
        *this = rhs;
    }

    //
    // Destructor of a dynamic array.
    //
    ~Tuple() { Resize(0); }

    void Initialize(const T& value)
    {
        for (int i = 0; i < top; i++)
            (*this)[i] = value;
        return;
    }

    void Reverse()
    {
        for (int head = 0, tail = Length() - 1; head < tail; head++, tail--)
        {
            T temp = (*this)[head];
            (*this)[head] = (*this)[tail];
            (*this)[tail] = temp;

        }

        return;
    }

    // ********************************************************************************************** //

    //
    // Return the total size of temporary space allocated.
    //
    inline size_t SpaceAllocated(void)
    {
        return ((base.Size() * sizeof(T **)) + (size * sizeof(T)));
    }

    //
    // Return the total size of temporary space used.
    //
    inline size_t SpaceUsed(void)
    {
        return (((size >> log_blksize) * sizeof(T **)) + (top * sizeof(T)));
    }
};


//
//
//
template <class T>
class ConvertibleArray : public Tuple<T>
{
public:

    ConvertibleArray(int estimate = 0) : Tuple<T>(estimate),
                                         array(NULL)
    {}

    ConvertibleArray(int log_blksize, int base_increment) : Tuple<T>(log_blksize, base_increment),
                                                            array(NULL)
    {}

    ~ConvertibleArray() { delete [] array; }

    //
    // This function converts a tuple into a regular array and destroys the
    // original tuple.
    //
    inline T *&Array()
    {
        if ((! array) && Tuple<T>::top > 0)
        {
            array = new T[Tuple<T>::top];

            int i = 0,
                processed_size = 0,
                n = (Tuple<T>::top - 1) >> Tuple<T>::log_blksize; // the last non-empty slot!
            while (i < n)
            {
                for (int j = 0; j < Tuple<T>::Blksize(); ++j)
                    array[processed_size + j] = Tuple<T>::base[i][j];
                delete [] Tuple<T>::base[i];
                i++;
                processed_size += Tuple<T>::Blksize();
            }
            for (int j = 0; j < Tuple<T>::top - processed_size; ++j)
                array[processed_size + j] = Tuple<T>::base[n][j];
            delete [] Tuple<T>::base[n];
            Tuple<T>::base.Resize(0);
            Tuple<T>::size = 0;
        }

        return array;
    }

    inline T& operator[](const int i)
    {
        //
        // Use this code when debugging to force a crash instead of
        // allowing the assertion to be raised.
        //
        // if (! (i >= 0 && i < top))
        // {
        //     ConvertibleArray<T> *p = NULL;
        //     p -> array = 0;
        // }
        assert(! Tuple<T>::OutOfRange(i));

        return (array
                    ? array[i]
                    : Tuple<T>::base[i >> Tuple<T>::log_blksize]
                                    [Tuple<T>::BlockOffset(i)]);
    }

    void Resize(int = 0) = delete;
    void Reset(int = 0) = delete;
    inline T& Next()
    {
        assert(! array);

        int i = Tuple<T>::NextIndex();
        return Tuple<T>::base[i >> Tuple<T>::log_blksize][i];
    }
    Tuple<T>& operator=(const Tuple<T>&) = delete;

private:

    T *array;
};

template <class T>
class Stack : private Tuple<T>
{
public:
    bool IsEmpty() { return Length() == 0; }

    int Length() { return Tuple<T>::Length(); }

    void Reset(int n = 0) { Tuple<T>::Reset(n); }

    void Push(T t)
    {
        Tuple<T>::Next() = t;
    }

    T Pop()
    {
        assert(! IsEmpty());
        int last_index = Length() - 1;
        T t = (*this)[last_index];
        Reset(last_index);

        return t;
    }

    void Pop(T &t)
    {
        assert(! IsEmpty());
        int last_index = Length() - 1;
        t = (*this)[last_index];
        Reset(last_index);

        return;
    }

    T &Top()
    {
        assert(! IsEmpty());
        return (*this)[Length() - 1];
    }
};

template <class T>
class BoundedArray
{
    int lbound,
        ubound;
    T *info;

public:

    BoundedArray() : lbound(0),
                     ubound(-1),
                     info(NULL)
    {}

    BoundedArray(int lbound_, int ubound_) : lbound(lbound_),
                                             ubound(ubound_)
    {
        assert(ubound >= lbound);
        info = new T[Size()];
    }

    BoundedArray(int lbound_, int ubound_, T init_value) : lbound(lbound_),
                                                           ubound(ubound_)
    {
        assert(ubound >= lbound);
        info = new T[Size()];
        Initialize(init_value);
    }

    BoundedArray(const BoundedArray<T>& rhs) : lbound(rhs.lbound),
                                                ubound(rhs.ubound),
                                                info(rhs.Size() == 0
                                                         ? nullptr
                                                         : new T[rhs.Size()])
    {
        for (int i = 0; i < rhs.Size(); ++i)
            info[i] = rhs.info[i];
    }

    BoundedArray(BoundedArray<T>&& rhs) noexcept : lbound(rhs.lbound),
                                                   ubound(rhs.ubound),
                                                   info(rhs.info)
    {
        rhs.lbound = 0;
        rhs.ubound = -1;
        rhs.info = nullptr;
    }

    ~BoundedArray()
    {
        delete [] info;
    }

    BoundedArray<T>& operator=(const BoundedArray<T>& rhs)
    {
        if (this != &rhs)
        {
            BoundedArray<T> copy(rhs);
            Swap(copy);
        }
        return *this;
    }

    BoundedArray<T>& operator=(BoundedArray<T>&& rhs) noexcept
    {
        if (this != &rhs)
        {
            delete [] info;
            lbound = rhs.lbound;
            ubound = rhs.ubound;
            info = rhs.info;
            rhs.lbound = 0;
            rhs.ubound = -1;
            rhs.info = nullptr;
        }
        return *this;
    }

    void Swap(BoundedArray<T>& rhs) noexcept
    {
        std::swap(lbound, rhs.lbound);
        std::swap(ubound, rhs.ubound);
        std::swap(info, rhs.info);
    }

    void Initialize(const T& init_value)
    {
        for (int i = lbound; i <= ubound; i++)
            (*this)[i] = init_value;
    }

    void Resize(int new_lbound, int new_ubound)
    {
        if (new_ubound < new_lbound)
        {
            delete [] info;
            info = NULL;
            lbound = new_lbound;
            ubound = new_ubound;
        }
        else
        {
            int new_size = new_ubound - new_lbound + 1;
            std::unique_ptr<T[]> new_info(new T[new_size]);
            if (info)
            {
                const int copy_lbound = std::max(lbound, new_lbound);
                const int copy_ubound = std::min(ubound, new_ubound);
                for (int i = copy_lbound; i <= copy_ubound; ++i)
                    new_info[i - new_lbound] = info[i - lbound];
            }

            delete [] info;
            info = new_info.release();
            lbound = new_lbound;
            ubound = new_ubound;
        }

        return;
    }

    void Memset(int val = 0)
    {
        static_assert(std::is_trivially_copyable<T>::value,
                      "BoundedArray::Memset requires a trivially copyable type");
        memset(info, val, Size() * sizeof(T));
    }

    void MemReset()
    {
        static_assert(std::is_trivially_copyable<T>::value,
                      "BoundedArray::MemReset requires a trivially copyable type");
        memset(info, 0, Size() * sizeof(T));
    }

    int Lbound() const { return lbound; }
    int Ubound() const { return ubound; }
    int Size() const { return ubound - lbound + 1; }

    //
    // Can the tuple be indexed with i?
    //
    inline bool OutOfRange(const int i) const { return (i < lbound || i > ubound); }

    //
    // Return the ith element of the array
    //
    T &operator[](const int i)
    {
        //
        // Use this code when debugging to force a crash instead of
        // allowing the assertion to be raised.
        //
        //if (i < lbound || i > ubound)
        //{
        //    BoundedArray<T> *a = NULL;
        //    a -> lbound = 0;
        //}
        assert(! OutOfRange(i));

        return info[i - lbound];
    }

    const T &operator[](const int i) const
    {
        assert(! OutOfRange(i));
        return info[i - lbound];
    }
};

#endif /* #ifndef TUPLE_INCLUDED */
