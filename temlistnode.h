// this file presents a definition to the template clss Listnode 

#ifndef TEMLISTNODE_H
#define TEMLISTNODE_H

#include <iostream>
template <class Nodetype> class List;
template < class Nodetype> 
class Listnode {
friend class List<Nodetype>;
public:
   Listnode( Nodetype ,  Listnode <Nodetype> *);
   ~Listnode();
   Listnode <Nodetype> *getnext (){return nextptr;};
   void set_sup (int sp){support = sp;};
   void set_comblnth (int cm){combl = cm;};
   int com(){return combl;};
   int sup() {return support;};
   int comblnth() {return combl;};
   void set_next(Listnode *next){nextptr = next;};
   void set_data(Nodetype value){data = value;};
   Nodetype getdata() {return data;};
   void clear();

private:
   Nodetype data;
   Listnode <Nodetype> *nextptr;
   int support;
   int combl;
};

// constructor 
template <class Nodetype> 
Listnode <Nodetype>::Listnode(Nodetype info, Listnode<Nodetype> *next)
{
  data = info;
  nextptr = next;
  support=0;
  combl=0;
}
 

//destructor
template <class Nodetype> 
Listnode <Nodetype>::~Listnode() 
{
  nextptr = NULL;
  data = 0;
  support = 0;
  combl = 0;
}

template <class Nodetype> 
void Listnode<Nodetype>::clear(){
   nextptr = NULL;
   data = 0;
   support = 0;
   combl = 0;
} 

#endif





