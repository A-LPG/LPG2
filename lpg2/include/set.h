#ifndef SET_INCLUDED
#define SET_INCLUDED

#include <limits.h>
#include <assert.h>
#include <algorithm>
#include <stdexcept>
#include <utility>

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

public:
    enum { EMPTY, UNIVERSE, cell_size = sizeof(CELL) * CHAR_BIT };

protected:
    CELL *s;
    int set_size;

    static int CellCountFor(int size)
    {
        return size <= 0 ? 0 : (size + cell_size - 1) / cell_size;
    }

    int CellCount() const { return CellCountFor(set_size); }

    void CheckIndex(int i) const
    {
        if (i < 0 || i >= set_size)
            throw std::out_of_range("BitSet index out of range");
    }

    void CheckCompatible(const BitSet& rhs) const
    {
        if (rhs.set_size != set_size)
            throw std::invalid_argument("BitSet sizes do not match");
    }

    void Allocate(int size)
    {
        if (size < 0)
            throw std::invalid_argument("BitSet size must be non-negative");
        delete [] s;
        set_size = size;
        s = CellCount() == 0 ? nullptr : new CELL[CellCount()];
    }

public:

    //
    // Produce the empty set.
    //
    void SetEmpty()
    {
        for (int i = CellCount() - 1; i >= 0; i--)
            s[i] = 0;
    }

    //
    // Produce the empty set.
    //
    bool IsEmpty()
    {
        for (int i = CellCount() - 1; i >= 0; i--)
            if (s[i] != 0)
                return false;
        return true;
    }

    //
    // Produce the universe set.
    //
    void SetUniverse()
    {
        for (int i = CellCount() - 1; i >= 0; i--)
            s[i] = ~((CELL) 0);
    }

    //
    // Produce the universe set.
    //
    bool IsUniverse()
    {
        for (int i = CellCount() - 1; i >= 0; i--)
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
        if (table_size <= 0)
            throw std::invalid_argument("BitSet hash table size must be positive");
        unsigned hash_address = 0;

        for (int i = CellCount() - 1; i >= 0; i--)
            hash_address += s[i];

        return hash_address % table_size;
    }

    //
    // Assignment of a bitset to another.
    //
    BitSet& operator=(const BitSet& rhs)
    {
        if (this != &rhs)
        {
            if (rhs.set_size != set_size)
                Allocate(rhs.set_size);
            for (int i = CellCount() - 1; i >= 0; i--)
                s[i] = rhs.s[i];
        }
        return *this;
    }

    BitSet& operator=(BitSet&& rhs) noexcept
    {
        if (this != &rhs)
        {
            delete [] s;
            s = rhs.s;
            set_size = rhs.set_size;
            rhs.s = nullptr;
            rhs.set_size = 0;
        }
        return *this;
    }

    //
    // Constructor of an uninitialized bitset.
    //
    BitSet() : s(nullptr), set_size(0) {}

    void SetSize(int size)
    {
        //
        // Note that we comment out the -1 because some C++ compilers
        // do not know how to allocate an array of size 0. Note that
        // we assert that set_size >= 0.
        //
        Allocate(size);
    }

    void Initialize(int size, int init = EMPTY)
    {
        //
        // Note that we comment out the -1 because some C++ compilers
        // do not know how to allocate an array of size 0. Note that
        // we assert that set_size >= 0.
        //
        Allocate(size);
        if (init == UNIVERSE)
             SetUniverse();
        else SetEmpty();
    }

    //
    // Constructor of an uninitialized bitset.
    //
    explicit BitSet(int set_size_) : s(nullptr), set_size(0)
    {
        //
        // Note that we comment out the -1 because some C++ compilers
        // do not know how to allocate an array of size 0. Note that
        // we assert that set_size >= 0.
        //
        Allocate(set_size_);
    }

    //
    // Constructor of an initialized bitset.
    //
    BitSet(int set_size_, int init) : s(nullptr), set_size(0)
    {
        //
        // Note that we comment out the -1 because some C++ compilers
        // do not know how to allocate an array of size 0. Note that
        // we assert that set_size >= 0.
        //
        Allocate(set_size_);
        if (init == UNIVERSE)
             SetUniverse();
        else SetEmpty();
    }

    //
    // Constructor to clone a bitset.
    //
    BitSet(const BitSet& rhs) : s(nullptr), set_size(0)
    {
        //
        // Note that we comment out the -1 because some C++ compilers
        // do not know how to allocate an array of size 0. Note that
        // we assert that set_size >= 0.
        //
        Allocate(rhs.set_size);
        for (int i = CellCount() - 1; i >= 0; i--)
            s[i] = rhs.s[i];
    }

    BitSet(BitSet&& rhs) noexcept : s(rhs.s), set_size(rhs.set_size)
    {
        rhs.s = nullptr;
        rhs.set_size = 0;
    }

    //
    // Destructor of a bitset.
    //
    ~BitSet() { delete [] s; }

    //
    // Return size of a bit set.
    //
    int Size() const { return set_size; }

    //
    // Can the set be indexed with i?
    //
    inline bool OutOfRange(const int i) const { return (i < 0 || i >= set_size); }

    //
    // Return a boolean value indicating whether or not the element i
    // is in the bitset in question.
    //
    bool operator[](const int i) const
    {
        CheckIndex(i);

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
        CheckIndex(i);

        s[i / cell_size] |= ((i + cell_size) % cell_size
                                             ? (CELL) 1 << ((i + cell_size) % cell_size)
                                             : (CELL) 1);
    }

    //
    // Remove an element i from the bitset in question.
    //
    void RemoveElement(int i)
    {
        CheckIndex(i);

        s[i / cell_size] &= ~((i + cell_size) % cell_size
                                              ? (CELL) 1 << ((i + cell_size) % cell_size)
                                              : (CELL) 1);
    }

    //
    // Yield a boolean result indicating whether or not two sets are
    // identical.
    //
    bool operator==(const BitSet& rhs) const
    {
        if (rhs.set_size != set_size)
            return false;

        for (int i = CellCount() - 1; i >= 0; i--)
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
    bool operator!=(const BitSet& rhs) const
    {
        return ! (*this == rhs);
    }

    //
    // Union of two bitsets.
    //
    BitSet operator+(const BitSet& rhs) const
    {
        CheckCompatible(rhs);
        BitSet result(set_size);

        for (int i = CellCount() - 1; i >= 0; i--)
            result.s[i] = s[i] | rhs.s[i];

        return result;
    }

    //
    // Union of an lvalue bitset and a rhs bitset.
    //
    BitSet& operator+=(const BitSet& rhs)
    {
        CheckCompatible(rhs);
        for (int i = CellCount() - 1; i >= 0; i--)
            s[i] |= rhs.s[i];
        return *this;
    }

    //
    // Intersection of two bitsets.
    //
    BitSet operator*(const BitSet& rhs) const
    {
        CheckCompatible(rhs);
        BitSet result(set_size);

        for (int i = CellCount() - 1; i >= 0; i--)
            result.s[i] = s[i] & rhs.s[i];

        return result;
    }

    //
    // Intersection of an lvalue bitset and a rhs bitset.
    //
    BitSet& operator*=(const BitSet& rhs)
    {
        CheckCompatible(rhs);
        for (int i = CellCount() - 1; i >= 0; i--)
            s[i] &= rhs.s[i];

        return *this;
    }

    //
    // Difference of two bitsets.
    //
    BitSet operator-(const BitSet& rhs) const
    {
        CheckCompatible(rhs);
        BitSet result(set_size);

        for (int i = CellCount() - 1; i >= 0; i--)
            result.s[i] = s[i] & (~ rhs.s[i]);

        return result;
    }

    //
    // Difference of an lvalue bitset and a rhs bitset.
    //
    BitSet& operator-=(const BitSet& rhs)
    {
        CheckCompatible(rhs);
        for (int i = CellCount() - 1; i >= 0; i--)
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

    BitSetWithOffset(BitSetWithOffset&& rhs) noexcept
        : BitSet(std::move(rhs)),
          offset(rhs.offset)
    {
        rhs.offset = 0;
    }

    BitSetWithOffset& operator=(const BitSetWithOffset& rhs)
    {
        if (this != &rhs)
        {
            BitSet::operator=(rhs);
            offset = rhs.offset;
        }
        return *this;
    }

    BitSetWithOffset& operator=(BitSetWithOffset&& rhs) noexcept
    {
        if (this != &rhs)
        {
            BitSet::operator=(std::move(rhs));
            offset = rhs.offset;
            rhs.offset = 0;
        }
        return *this;
    }

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
    bool operator[](const int k) const
    {
        return BitSet::operator[](k - offset);
    }

    //
    // Insert an element i in the bitset in question.
    //
    void AddElement(const int k)
    {
        BitSet::AddElement(k - offset);
    }

    //
    // Remove an element i from the bitset in question.
    //
    void RemoveElement(const int k)
    {
        BitSet::RemoveElement(k - offset);
    }
};
#endif
