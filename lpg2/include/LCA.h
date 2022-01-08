#ifndef LCA_INCLUDED
#define LCA_INCLUDED

#include "util.h"
#include "tuple.h"
#include "set.h"

//
// Least-Common-Ancestor
//
class LCA
{
    BoundedArray< Tuple<int> > &ancestors;
    BoundedArray<BitSetWithOffset> ancestorClosure;
    BoundedArray<int> index_of;
    Stack<int> stack;

    enum { TABLE_SIZE = 509 }; // 509 is a prime
    Array< Tuple<int> > table;

    //
    // ComputeAncestorClosure computes ancestorClosure[nt] for some non-terminal NT using the
    // digraph algorithm.  ancestorClosure[NT] is the transitive closure of the ancestors
    // of NT.
    //
    void ComputeAncestorClosure(int nt)
    {
        //
        // ancestorClosure[NT] was initialized to { NT } for each non-terminal NT.
        //
        stack.Push(nt);
        int indx = stack.Length();
        index_of[nt] = indx;

        //
        // iterate over all items of NT
        //
        for (int i = 0; i < ancestors[nt].Length(); i++)
        {
            int ancestor = ancestors[nt][i];
            if (index_of[ancestor] == Util::OMEGA)
                ComputeAncestorClosure(ancestor);
            ancestorClosure[nt] += ancestorClosure[ancestor];
            index_of[nt] = Util::Min(index_of[nt], index_of[ancestor]);
        }

        if (index_of[nt] == indx)
        {
            for (int ancestor = stack.Top(); ancestor != nt; ancestor = stack.Top())
            {
                ancestorClosure[ancestor] = ancestorClosure[nt];
                index_of[ancestor] = Util::INFINITY_;
                stack.Pop();
            }

            index_of[nt] = Util::INFINITY_;
            stack.Pop();
            table[ancestorClosure[nt].Hash(TABLE_SIZE)].Next() = nt;
        }

        return;
    }

public:

    LCA(BoundedArray< Tuple<int> > &ancestors_) : ancestors(ancestors_),
                                                  table(TABLE_SIZE)
    {
        index_of.Resize(ancestors.Lbound(), ancestors.Ubound());
        index_of.Initialize(Util::OMEGA);
        ancestorClosure.Resize(ancestors.Lbound(), ancestors.Ubound());
        for (int nt = ancestors.Lbound(); nt <= ancestors.Ubound(); nt++)
        {
            ancestorClosure[nt].Initialize(ancestors.Size() + 1, ancestors.Lbound() - 1);
            ancestorClosure[nt].AddElement(nt);
        }
 
        {
            for (int nt = ancestors.Lbound(); nt <= ancestors.Ubound(); nt++)
                if (index_of[nt] == Util::OMEGA)
                    ComputeAncestorClosure(nt);
        }
    }

    int Find(int nt1, int nt2)
    {
        if (nt1 != 0 && nt2 != 0)
        {
            BitSetWithOffset lca_set(ancestorClosure[nt1]);
            lca_set *= ancestorClosure[nt2];
            int hash_index = lca_set.Hash(TABLE_SIZE);
            for (int i = 0; i < table[hash_index].Length(); i++)
            {
                int ancestor = table[hash_index][i];
                if (ancestorClosure[ancestor] == lca_set)
                    return ancestor;
            }
        }

        return 0;
    }
};
#endif /* LCA_INCLUDED */
