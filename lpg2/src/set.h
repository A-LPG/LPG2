#ifndef SET_INCLUDED
#define SET_INCLUDED

#include <limits.h>
#include <assert.h>

//
// This Bitset template class can be used to construct sets of
// integers. The template takes as argument the address of an integer
// variable, set_size, which indicates that the universe of the sets
// is: {0..*set_size}.
//
class BitSet
{
protected:
    typedef unsigned CELL;

    CELL *s;
    int set_size;

public:

    enum { EMPTY, UNIVERSE, cell_size = sizeof(CELL) * CHAR_BIT };

    //
    // Produce the empty set.
    //
    void SetEmpty()
    {
        for (int i = (set_size - 1) / cell_size; i >= 0; i--)
            s[i] = 0;
    }

    //
    // Produce the empty set.
    //
    bool IsEmpty()
    {
        for (int i = (set_size - 1) / cell_size; i >= 0; i--)
            if (s[i] != 0)
                return false;
        return true;
    }

    //
    // Produce the universe set.
    //
    void SetUniverse()
    {
        for (int i = (set_size - 1) / cell_size; i >= 0; i--)
            s[i] = ~((CELL) 0);
    }

    //
    // Produce the universe set.
    //
    bool IsUniverse()
    {
        for (int i = (set_size - 1) / cell_size; i >= 0; i--)
            if (s[i] != ~((CELL) 0))
                return false;
        return true;
    }

    //
    // This function takes as argument the size of a hash table, table_size.
    // It hashes a bitset into a location within the range <1..table_size-1>.
    //
    unsigned Hash(int table_size)
    {
        unsigned hash_address = 0;

        for (int i = (set_size - 1) / cell_size; i >= 0; i--)
            hash_address += s[i];

        return hash_address % table_size;
    }

    //
    // Assignment of a bitset to another.
    //
    BitSet& operator=(const BitSet& rhs)
    {
if (rhs.set_size != set_size)
{
    BitSet *b = NULL;
    b -> set_size = 0;
}
        assert(rhs.set_size == set_size);

        for (int i = (set_size - 1) / cell_size; i >= 0; i--)
            s[i] = rhs.s[i];

        return *this;
    }

    //
    // Constructor of an uninitialized bitset.
    //
    BitSet()
    {
        set_size = 0;
        s = NULL;
    }

    void SetSize(int size)
    {
        //
        // Note that we comment out the -1 because some C++ compilers
        // do not know how to allocate an array of size 0. Note that
        // we assert that set_size >= 0.
        //
        set_size = size;
        s = new CELL[(set_size + cell_size /* - 1 */) / cell_size];
    }

    void Initialize(int size, int init = EMPTY)
    {
        //
        // Note that we comment out the -1 because some C++ compilers
        // do not know how to allocate an array of size 0. Note that
        // we assert that set_size >= 0.
        //
        set_size = size;
        s = new CELL[(set_size + cell_size /* - 1 */) / cell_size];
        if (init == UNIVERSE)
             SetUniverse();
        else SetEmpty();
    }

    //
    // Constructor of an uninitialized bitset.
    //
    BitSet(int set_size_) : set_size(set_size_)
    {
        //
        // Note that we comment out the -1 because some C++ compilers
        // do not know how to allocate an array of size 0. Note that
        // we assert that set_size >= 0.
        //
        s = new CELL[(set_size + cell_size /* - 1 */) / cell_size];
    }

    //
    // Constructor of an initialized bitset.
    //
    BitSet(int set_size_, int init) : set_size(set_size_)
    {
        //
        // Note that we comment out the -1 because some C++ compilers
        // do not know how to allocate an array of size 0. Note that
        // we assert that set_size >= 0.
        //
        s = new CELL[(set_size + cell_size /* - 1 */) / cell_size];
        if (init == UNIVERSE)
             SetUniverse();
        else SetEmpty();
    }

    //
    // Constructor to clone a bitset.
    //
    BitSet(const BitSet& rhs) : set_size(rhs.set_size)
    {
        //
        // Note that we comment out the -1 because some C++ compilers
        // do not know how to allocate an array of size 0. Note that
        // we assert that set_size >= 0.
        //
        s = new CELL[(set_size + cell_size /* - 1 */) / cell_size];
        for (int i = (set_size - 1) / cell_size; i >= 0; i--)
            s[i] = rhs.s[i];
    }

    //
    // Destructor of a bitset.
    //
    ~BitSet() { delete [] s; }

    //
    // Return size of a bit set.
    //
    int Size() { return set_size; }

    //
    // Can the set be indexed with i?
    //
    inline bool OutOfRange(const int i) { return (i < 0 || i >= set_size); }

    //
    // Return a boolean value indicating whether or not the element i
    // is in the bitset in question.
    //
    bool operator[](const int i)
    {
if (! (i >= 0 && i < set_size))
{
    BitSet *b = NULL;
    b -> set_size = 0;
}
        assert(! OutOfRange(i));

        return (s[i / cell_size] &
                ((i + cell_size) % cell_size
                          ? (CELL) 1 << ((i + cell_size) % cell_size)
                          : (CELL) 1)) != 0;
    }

    //
    // Insert an element i in the bitset in question.
    //
    void AddElement(int i)
    {
if (! (i >= 0 && i < set_size))
{
    BitSet *b = NULL;
    b -> set_size = 0; // force a crash !!!
}
        assert(! OutOfRange(i));

        s[i / cell_size] |= ((i + cell_size) % cell_size
                                             ? (CELL) 1 << ((i + cell_size) % cell_size)
                                             : (CELL) 1);
    }

    //
    // Remove an element i from the bitset in question.
    //
    void RemoveElement(int i)
    {
if (! (i >= 0 && i < set_size))
{
    BitSet *b = NULL;
    b -> set_size = 0; // force a crash !!!
}
        assert(! OutOfRange(i));

        s[i / cell_size] &= ~((i + cell_size) % cell_size
                                              ? (CELL) 1 << ((i + cell_size) % cell_size)
                                              : (CELL) 1);
    }

    //
    // Yield a boolean result indicating whether or not two sets are
    // identical.
    //
    bool operator==(const BitSet& rhs)
    {
if (rhs.set_size != set_size)
{
    BitSet *b = NULL;
    b -> set_size = 0; // force a crash !!!
}
        assert(rhs.set_size == set_size);

        for (int i = (set_size - 1) / cell_size; i >= 0; i--)
        {
            if (s[i] != rhs.s[i])
                return false;
        }

        return true;
    }

    //
    // Yield a boolean result indicating whether or not two sets are
    // identical.
    //
    bool operator!=(const BitSet& rhs)
    {
        return ! (*this == rhs);
    }

    //
    // Union of two bitsets.
    //
    BitSet operator+(const BitSet& rhs)
    {
if (rhs.set_size != set_size)
{
    BitSet *b = NULL;
    b -> set_size = 0; // force a crash !!!
}
        assert(rhs.set_size == set_size);
        BitSet result(set_size);

        for (int i = (set_size - 1) / cell_size; i >= 0; i--)
            result.s[i] = s[i] | rhs.s[i];

        return result;
    }

    //
    // Union of an lvalue bitset and a rhs bitset.
    //
    BitSet& operator+=(const BitSet& rhs)
    {
if (rhs.set_size != set_size)
{
    BitSet *b = NULL;
    b -> set_size = 0; // force a crash !!!
}
        assert(rhs.set_size == set_size);
        for (int i = (set_size - 1) / cell_size; i >= 0; i--)
            s[i] |= rhs.s[i];

        return *this;
    }

    //
    // Intersection of two bitsets.
    //
    BitSet operator*(const BitSet& rhs)
    {
if (rhs.set_size != set_size)
{
    BitSet *b = NULL;
    b -> set_size = 0; // force a crash !!!
}
        assert(rhs.set_size == set_size);
        BitSet result(set_size);

        for (int i = (set_size - 1) / cell_size; i >= 0; i--)
            result.s[i] = s[i] & rhs.s[i];

        return result;
    }

    //
    // Intersection of an lvalue bitset and a rhs bitset.
    //
    BitSet& operator*=(const BitSet& rhs)
    {
if (rhs.set_size != set_size)
{
    BitSet *b = NULL;
    b -> set_size = 0; // force a crash !!!
}
        assert(rhs.set_size == set_size);
        for (int i = (set_size - 1) / cell_size; i >= 0; i--)
            s[i] &= rhs.s[i];

        return *this;
    }

    //
    // Difference of two bitsets.
    //
    BitSet operator-(const BitSet& rhs)
    {
if (rhs.set_size != set_size)
{
    BitSet *b = NULL;
    b -> set_size = 0; // force a crash !!!
}
        assert(rhs.set_size == set_size);
        BitSet result(set_size);

        for (int i = (set_size - 1) / cell_size; i >= 0; i--)
            result.s[i] = s[i] & (~ rhs.s[i]);

        return result;
    }

    //
    // Difference of an lvalue bitset and a rhs bitset.
    //
    BitSet& operator-=(const BitSet& rhs)
    {
if (rhs.set_size != set_size)
{
    BitSet *b = NULL;
    b -> set_size = 0; // force a crash !!!
}
        assert(rhs.set_size == set_size);
        for (int i = (set_size - 1) / cell_size; i >= 0; i--)
            s[i] &= (~ rhs.s[i]);

        return *this;
    }
};

class BitSetWithOffset : public BitSet
{
    int offset;

public:

    //
    // Constructor to clone a bitset.
    //
    BitSetWithOffset(const BitSetWithOffset& rhs) : BitSet(rhs),
                                                    offset(rhs.offset)
    {}

    BitSetWithOffset(int set_size_, int offset_) : BitSet(set_size_),
                                                   offset(offset_)
    {}

    BitSetWithOffset() : offset(0) {}

    void Initialize(int set_size_, int offset_, int init = EMPTY)
    {
        offset = offset_;
        BitSet::Initialize(set_size_, init);
    }

    //
    // Return a boolean value indicating whether or not the element i
    // is in the bitset in question.
    //
    bool operator[](const int k)
    {
        int i = k - offset;
if (! (i >= 0 && i < set_size))
{
    BitSetWithOffset *b = NULL;
    b -> offset = 0;
}
        assert(! OutOfRange(i));

        return (s[i / cell_size] &
                ((i + cell_size) % cell_size
                          ? (CELL) 1 << ((i + cell_size) % cell_size)
                          : (CELL) 1)) != 0;
    }

    //
    // Insert an element i in the bitset in question.
    //
    void AddElement(const int k)
    {
        int i = k - offset;

if (! (i >= 0 && i < set_size))
{
    BitSetWithOffset *b = NULL;
    b -> offset = 0;
}
        assert(! OutOfRange(i));

        s[i / cell_size] |= ((i + cell_size) % cell_size
                                             ? (CELL) 1 << ((i + cell_size) % cell_size)
                                             : (CELL) 1);
    }

    //
    // Remove an element i from the bitset in question.
    //
    void RemoveElement(const int k)
    {
        int i = k - offset;

if (! (i >= 0 && i < set_size))
{
    BitSetWithOffset *b = NULL;
    b -> set_size = 0; // force a crash !!!
}
        assert(! OutOfRange(i));

        s[i / cell_size] &= ~((i + cell_size) % cell_size
                                              ? (CELL) 1 << ((i + cell_size) % cell_size)
                                              : (CELL) 1);
    }
};
#endif
