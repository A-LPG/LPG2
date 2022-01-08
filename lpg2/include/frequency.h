#ifndef FREQUENCY_INCLUDED
#define FREQUENCY_INCLUDED

#include "tuple.h"
#include "util.h"

class Frequency
{
public:
    //
    //  SORT sorts the elements of ARRAY and COUNT in the range LOW..HIGH
    // based on the values of the elements of COUNT. Knowing that the maximum
    // value of the elements of count cannot exceed MAX and cannot be lower
    // than zero, we can use a bucket sort technique.
    //
    static void sort(BoundedArray<int> &array, BoundedArray<int> &count, int max)
    {
        int low = array.Lbound(),
            high = array.Ubound();

        int element,
            i,
            k;

        //
        // BUCKET is used to hold the roots of lists that contain the
        // elements of each bucket.  LIST is used to hold these lists.
        //
        Array<int> bucket(max + 1, Util::NIL),
                   list(high - low + 1);

        //
        // We now partition the elements to be sorted and place them in their
        // respective buckets.  We iterate backward over ARRAY and COUNT to
        // keep the sorting stable since elements are inserted in the buckets
        // in stack-fashion.
        //
        //   NOTE that it is known that the values of the elements of ARRAY
        // also lie in the range LOW..HIGH.
        //
        for (i = high; i >= low; i--)
        {
            k = count[i];
            element = array[i];
            list[element - low] = bucket[k];
            bucket[k] = element;
        }

        //
        // Iterate over each bucket, and place elements in ARRAY and COUNT
        // in sorted order.  The iteration is done backward because we want
        // the arrays sorted in descending order.
        //
        k = low;
        for (i = max; i >= 0; i--)
        {
            for (element = bucket[i];
                 element != Util::NIL; element = list[element - low], k++)
            {
                array[k] = element;
                count[k] = i;
            }
        }

        return;
    }

    //
    //                              SORT:
    //
    //  SORT sorts the elements of ARRAY and COUNT in the range LOW..HIGH
    // based on the values of the elements of COUNT. Knowing that the maximum
    // value of the elements of count cannot exceed MAX and cannot be lower
    // than zero, we can use a bucket sort technique.
    //
    static void sort(Array<int> &array, Array<int> &count, int low, int high, int max)
    {
        int element,
            i,
            k;

        //
        // BUCKET is used to hold the roots of lists that contain the
        // elements of each bucket.  LIST is used to hold these lists.
        //
        Array<int> bucket(max + 1, Util::NIL),
                   list(high - low + 1);

        //
        // We now partition the elements to be sorted and place them in their
        // respective buckets.  We iterate backward over ARRAY and COUNT to
        // keep the sorting stable since elements are inserted in the buckets
        // in stack-fashion.
        //
        //   NOTE that it is known that the values of the elements of ARRAY
        // also lie in the range LOW..HIGH.
        //
        for (i = high; i >= low; i--)
        {
            k = count[i];
            element = array[i];
            list[element - low] = bucket[k];
            bucket[k] = element;
        }

        //
        // Iterate over each bucket, and place elements in ARRAY and COUNT
        // in sorted order.  The iteration is done backward because we want
        // the arrays sorted in descending order.
        //
        k = low;
        for (i = max; i >= 0; i--)
        {
            for (element = bucket[i];
                 element != Util::NIL; element = list[element - low], k++)
            {
                array[k] = element;
                count[k] = i;
            }
        }

        return;
    }
};

#endif
