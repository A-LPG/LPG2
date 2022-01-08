#ifndef NODE_INCLUDED
#define NODE_INCLUDED

#include "tuple.h"

struct Node
{
    Node *next;
    int value;
};


class NodePool
{
    Tuple<Node> pool;
    Node *free_nodes;

public:

    NodePool() : pool(32768),
                 free_nodes(NULL)
    {}

    //
    //   This function allocates a node structure and returns a pointer to it.
    // it there are nodes in the free pool, one of them is returned. Otherwise,
    // a new node is allocated from the global storage pool.
    //
    Node *AllocateNode()
    {
        Node *p = free_nodes;
        if (p != NULL)  // is free list not empty?
             free_nodes = p -> next;
        else p = &(pool.Next());

        return p;
    }


    //
    //  This function frees a linked list of nodes by adding them to the free
    // list.  Head points to head of linked list and tail to the end.
    //
    void FreeNodes(Node *head, Node *tail)
    {
        tail -> next = free_nodes;
        free_nodes = head;

        return;
    }
};
#endif
