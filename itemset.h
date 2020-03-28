// this file define the class Itemset 

#ifndef ITEMSET_H 
#define ITEMSET_H 

#include "temlist.h"
#include "Array1.h" 

class Itemset{ 

friend ostream &operator << (ostream &, Itemset &); 

public:
    Itemset(int list_sz);
    ~Itemset();

    int tid(int pos){return thetidlist -> item(pos);};
    void set_itemset (List<int> *lst){theitemset = lst;};
    void set_tidlist (Array *lst){thetidlist = lst;};
    List<int> *itemset(){return theitemset;};
    Array *tidlist(){return thetidlist;};
    void add_item(int val){theitemset -> addatback(val);};
    void add_tid(int val){thetidlist -> add(val);};
    int itemsetsize(){return theitemset -> size();};
    int tidsize(){return thetidlist -> size();};
    int tidtotsize(){return thetidlist ->totsize();};
    int& support(){return thesupport;};
    void set_support(int sup){ thesupport = sup;};
    void increment_support(){thesupport++;};

private:

List<int> *theitemset;
Array *thetidlist;
int thesupport;
};
#endif









