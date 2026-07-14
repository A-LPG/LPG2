#include "set.h"
#include "tuple.h"

#include <stdexcept>

namespace {
void require(bool condition)
{
    if (!condition)
        throw std::runtime_error("container regression assertion failed");
}

void test_array_of_bitsets()
{
    Array<BitSet> values(2);
    values[0].Initialize(65);
    values[1].Initialize(65);
    values[0].AddElement(0);
    values[0].AddElement(64);
    values[1].AddElement(12);

    values.Resize(8);
    require(values[0][0]);
    require(values[0][64]);
    require(values[1][12]);

    values[0].RemoveElement(0);
    require(!values[0][0]);
    values.Resize(1);
    require(values.Size() == 1);
    require(values[0][64]);
}

void test_array_of_tuples()
{
    Array<Tuple<int> > values(2);
    for (int i = 0; i < 40; ++i)
        values[0].Next() = i * 3;
    values[1].Next() = 99;

    values.Resize(6);
    require(values[0].Length() == 40);
    for (int i = 0; i < 40; ++i)
        require(values[0][i] == i * 3);
    require(values[1][0] == 99);

    values[0][0] = -1;
    require(values[0][0] == -1);
}

void test_bounded_array_of_nontrivial_values()
{
    BoundedArray<BitSet> values(-2, 1);
    for (int i = values.Lbound(); i <= values.Ubound(); ++i)
    {
        values[i].Initialize(32);
        values[i].AddElement(i + 2);
    }

    values.Resize(-4, 4);
    for (int i = -2; i <= 1; ++i)
        require(values[i][i + 2]);

    values.Resize(-1, 0);
    require(values[-1][1]);
    require(values[0][2]);
}

void test_bitset_failures_are_exceptions()
{
    BitSet small(8, BitSet::EMPTY);
    BitSet large(16, BitSet::EMPTY);

    bool out_of_range = false;
    try
    {
        small.AddElement(8);
    }
    catch (const std::out_of_range&)
    {
        out_of_range = true;
    }
    require(out_of_range);

    bool incompatible = false;
    try
    {
        (void)(small + large);
    }
    catch (const std::invalid_argument&)
    {
        incompatible = true;
    }
    require(incompatible);

    small = large;
    require(small.Size() == 16);

    BitSetWithOffset offset(4, 10);
    offset.SetEmpty();
    offset.AddElement(12);
    require(offset[12]);

    out_of_range = false;
    try
    {
        (void)offset[9];
    }
    catch (const std::out_of_range&)
    {
        out_of_range = true;
    }
    require(out_of_range);
}
} // namespace

int main()
{
    test_array_of_bitsets();
    test_array_of_tuples();
    test_bounded_array_of_nontrivial_values();
    test_bitset_failures_are_exceptions();
    return 0;
}
