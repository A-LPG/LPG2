#ifndef CTC_INCLUDED
#define CTC_INCLUDED

#include "tuple.h"
#include "set.h"
#include "control.h"

//
// CTC (Class transitive closure)
//
// For each interface I associated with a nonterminal, compute the 
// transitive closure of the set of classes that may implement this
// interface.
//
class CTC
{
    BoundedArray< Tuple<int> > extends,
                               interface_map;

    BoundedArray<BitSet> closure;
    BoundedArray<int> index_of;
    Stack<int> stack;

    Tuple<ClassnameElement> &classname;
    Array<const char *> &typestring;

    void ComputeClosure(int nt)
    {
        stack.Push(nt);
        int indx = stack.Length();
        index_of[nt] = indx;

        for (int i = 0; i < extends[nt].Length(); i++)
        {
            int dependent = extends[nt][i];
            if (index_of[dependent] == Util::OMEGA)
                ComputeClosure(dependent);
            closure[nt] += closure[dependent];
            index_of[nt] = Util::Min(index_of[nt], index_of[dependent]);
        }

        if (index_of[nt] == indx)
        {
            for (int dependent = stack.Top(); dependent != nt; dependent = stack.Top())
            {
                closure[dependent] = closure[nt];
                index_of[dependent] = Util::INFINITY_;
                stack.Pop();
            }

            index_of[nt] = Util::INFINITY_;
            stack.Pop();
        }

        return;
    }

public:

    CTC(Tuple<ClassnameElement> &classname_, Array<const char *> &typestring_, BoundedArray< Tuple<int> > &is_extended_by)
        : classname(classname_),
          typestring(typestring_)
    {
        //
        // Reverse the original extension map.
        //
        extends.Resize(is_extended_by.Lbound(), is_extended_by.Ubound());
        {
            for (int i = is_extended_by.Lbound(); i <= is_extended_by.Ubound(); i++)
                for (int k = 0; k < is_extended_by[i].Length(); k++)
                    extends[is_extended_by[i][k]].Next() = i;
        }

        index_of.Resize(extends.Lbound(), extends.Ubound());
        index_of.Initialize(Util::OMEGA);

        closure.Resize(extends.Lbound(), extends.Ubound());
        for (int nt = closure.Lbound(); nt <= closure.Ubound(); nt++)
            closure[nt].Initialize(classname.Length());

        for (int i = 0; i < classname.Length(); i++)
        {
            Tuple<int> &interface = classname[i].interface_;
            for (int k = 0; k < interface.Length(); k++)
                closure[interface[k]].AddElement(i);
        }

        //
        //
        //
        {
            for (int nt = closure.Lbound(); nt <= closure.Ubound(); nt++)
                if (index_of[nt] == Util::OMEGA)
                    ComputeClosure(nt);
        }

        interface_map.Resize(extends.Lbound(), extends.Ubound());
        {
            for (int i = interface_map.Lbound(); i <= interface_map.Ubound(); i++)
                for (int k = 0; k < closure[i].Size(); k++)
                {
                    if (closure[i][k])
                        interface_map[i].Next() = k;
                }
        }
    }

    BoundedArray< Tuple<int> > &GetInterfaceMap() { return interface_map; }

    bool IsTerminalClass(int interface)
    {
        bool is_terminal_class = true;
    	
        for (int k = 0; k < interface_map[interface].Length(); k++)
        {
            int class_index = interface_map[interface][k];
            is_terminal_class = is_terminal_class && classname[class_index].is_terminal_class;
        }

        return is_terminal_class;
    }
    const char* FindUniqueTypeFor(int interface,const char* default_type)
    {
        auto temp_type = FindUniqueTypeFor(interface);
    	if(!temp_type)
    	{
            temp_type = default_type;
    	}
        return  temp_type;
    }
    const char *FindUniqueTypeFor(int interface)
    {
        if (interface == 0) // Not a grammar symbol?
             return typestring[0]; // The default (root) Ast
        else if (interface < interface_map.Lbound()) // a terminal symbol?
             return classname[0].real_name; //ast_token_classname
        else if (interface_map[interface].Length() == 1)
             return classname[interface_map[interface][0]].GetAllocationName(interface); // .real_name;
        else return NULL;
    }

    const char *FindBestTypeFor(int interface)
    {
        if (interface == 0) // The default (root) Ast
             return typestring[0];
        else if (interface < interface_map.Lbound()) // a terminal symbol?
             return classname[0].real_name; //ast_token_classname
        else if (interface_map[interface].Length() == 1)
             return classname[interface_map[interface][0]].GetAllocationName(interface); // .real_name;
        else return typestring[interface];
    }
    bool IsInterface(int interface)
    {
        if (interface == 0) // The default (root) Ast
            return false;
        else if (interface < interface_map.Lbound()) // a terminal symbol?
            return false; //ast_token_classname
        else if (interface_map[interface].Length() == 1)
            return false; // .real_name;
        else return true;
    }
};
#endif /* CTC_INCLUDED */
