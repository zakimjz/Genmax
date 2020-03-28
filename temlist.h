// this file presents the template class List 

#ifndef TEMLIST_H 
#define TEMLIST_H

#include <iostream>
//#include <assert.h>
#include "temlistnode.h"

using namespace std;

template <class Nodetype>
ostream &operator << (ostream &outputStream, List<Nodetype> &thlist);

template <class Nodetype>

class List {
friend ostream &operator << <Nodetype>(ostream &outputStream, List<Nodetype> &thlist);
public:
    List();
    ~List();
    void clear();
    void set_head (Listnode<Nodetype> *hd){firstptr = hd;};
    void set_last (Listnode<Nodetype> *lst){lastptr = lst;};
    //void set_loc (Listnode<Nodetype> *l) {loc = l;};
    //void set_com (int n){firstptr->combl = n;};
    Listnode <Nodetype> *head(){return firstptr;};
    Listnode <Nodetype> *last(){return lastptr;}; 
    Nodetype getfromback(){return lastptr -> data;}
    bool Isempty() {return firstptr == 0;};
    //Listnode <Nodetype> *node(int);
    int size (){ return thesize; };
    void addatfront(Nodetype );
    void sorted_descend(Nodetype, int);
    void sorted_ascend(Nodetype, int);
    void addatback(Nodetype);
    void  removefromfront();
    void removefromback();
    void deleted(Nodetype &);
    bool Ismember(Nodetype );
    bool Isubset(List<Nodetype> *, int);
    //bool Isubset(List<Nodetype> *, int );
    //bool Isublist(List<Nodetype> *);
    //bool Is_subset(List<Nodetype> *);
    //bool Is_sublist(List<Nodetype> *);
     //bool subsetnew(List<Nodetype> *);
    //bool subsetnew2(List<Nodetype> *);

private:
    Listnode <Nodetype> *firstptr;
    Listnode <Nodetype> *lastptr;
    //Listnode <Nodetype> *loc;
    int thesize;
};

//constructor
template <class Nodetype>
List<Nodetype>::List()
{
    firstptr = 0;
    lastptr = 0;
    //loc = 0;
    thesize = 0;
}

template <class Nodetype> 
List<Nodetype>::~List()
{
   clear();
}

template <class Nodetype> 
void List<Nodetype>::clear()
{
      Listnode <Nodetype> *node = firstptr;
      while (node){
         firstptr = firstptr -> nextptr;
         node -> clear();
         delete node;
         node = firstptr;
      }
      firstptr = NULL;
      lastptr = NULL;
      //loc = NULL;
      thesize = 0;
}
     
//insert a node at the front of the list
template <class Nodetype>
void List<Nodetype>::addatfront(Nodetype item) 
{
    Listnode<Nodetype>  *node;
    thesize++;
    node = new Listnode<Nodetype> (item, NULL);
    if (node == NULL){
         cout << "MEMORY EXCEEDED\n";
         exit(-1);
    }
    if (firstptr == 0){
         firstptr = node;
         lastptr = node;
    }
    else{
         node ->set_next(firstptr);
         firstptr = node;
    }
}


// remove a node from the front of the list and get its data into value
template <class Nodetype> 
void List<Nodetype>::removefromfront()
{

    Listnode <Nodetype> *temptr = firstptr;
    if (firstptr == lastptr)
         firstptr = lastptr = 0;
    else 
       firstptr = firstptr -> nextptr;

   if (temptr->getdata()) delete temptr->getdata();
   delete temptr;
   thesize--;
}


template <class Nodetype> 
void List<Nodetype>::sorted_descend(Nodetype item, int a)
{
   Listnode <Nodetype> *node;
   Listnode <Nodetype> *temp = firstptr;

   thesize++;
   node = new  Listnode <Nodetype>(item, NULL);
   if (node == NULL){
      cout << "MEMORY EXCEEDED\n";
      exit(-1);
   }
   node -> combl = a;
   if (firstptr == 0){
      firstptr = node;
      lastptr = node;
   }
   else if ( a > firstptr->combl){
      node->nextptr = firstptr;
      firstptr = node;
   }
   else{
      while (temp->nextptr){
         if ( a > temp->nextptr->combl){
            node->nextptr = temp->nextptr;
            temp->nextptr = node;
            return;
         }
         temp = temp->nextptr;
      }
      lastptr->nextptr = node;
      lastptr = node;
   }
   
}
   

template <class Nodetype> 
void List<Nodetype>::sorted_ascend(Nodetype item, int a)
{
   Listnode <Nodetype> *node;
   Listnode <Nodetype> *temp = firstptr;

   thesize++;
   node = new  Listnode <Nodetype>(item, NULL);
   if (node == NULL){
      cout << "MEMORY EXCEEDED\n";
      exit(-1);
   }
   node -> combl = a;
   if (firstptr == 0){
      firstptr = node;
      lastptr = node;
   }
   else if ( a < firstptr->combl){
      node->nextptr = firstptr;
      firstptr = node;
   }
   else{
      while (temp->nextptr){
         if ( a < temp->nextptr->combl){
            node->nextptr = temp->nextptr;
            temp->nextptr = node;
            return;
         }
         temp = temp->nextptr;
      }
      lastptr->nextptr = node;
      lastptr = node;
   }
   
}

//insert a node at the back of the list 
template <class Nodetype> 
void List<Nodetype>::addatback (Nodetype item)
{
   Listnode <Nodetype> *newptr;
   newptr = new Listnode<Nodetype> (item, NULL);
   if (newptr == NULL){
         cout << "MEMORY EXCEEDED\n";
         exit(-1);
    }
 if (firstptr == 0){
      firstptr = newptr;
      lastptr = newptr;
 }
 else {
      //lastptr -> nextptr = newptr;
      lastptr -> set_next(newptr);
      lastptr = newptr;
 }
 thesize++;
}

/*
template <class Nodetype>
Listnode<Nodetype> *List<Nodetype>::node(int pos) 
{

   Listnode<Nodetype> *head = firstptr;
   for (int i=0; i < pos && head; head = head->nextptr,i++);
   return head;
}
*/

template <class Nodetype> 
void List<Nodetype>::removefromback()
{

       Listnode <Nodetype> *temptr = lastptr;
       if (firstptr == lastptr)
            firstptr = lastptr = 0;
       else {
          Listnode <Nodetype> *currentptr = firstptr;
          while (currentptr -> nextptr != lastptr)
                currentptr = currentptr -> nextptr;
          lastptr = currentptr;
          currentptr -> nextptr = NULL;
       }
       delete temptr;
       thesize--;
}

//delete a given value from the list if it is found.
template <class Nodetype> 
void List<Nodetype>::deleted(Nodetype &value)
{
       Listnode <Nodetype> *currentptr = firstptr;
       Listnode <Nodetype> *ptr=NULL;
       while (currentptr){
                if  (currentptr -> data == value ){
		    if (currentptr == firstptr){
                                            firstptr = firstptr -> nextptr;
                                            thesize--;
                                            break;
		    }
                    else{
                        ptr -> nextptr = currentptr -> nextptr;
                        if (currentptr == lastptr)
                                            lastptr = ptr; 
                        thesize--;
                        break;
		    }
		}
                ptr = currentptr;
                currentptr = currentptr -> nextptr;
       }
}

// Is element in the list
template <class Nodetype>
bool List<Nodetype>::Ismember(Nodetype element)
{

  Listnode<Nodetype> *temptr = firstptr; 
  for (; temptr; temptr = temptr -> nextptr) {
       if ( element  == temptr -> data) {
	 return true;
       }
  }

  return false;
} 

// display the contents of the list 
template <class Nodetype>
ostream &operator << (ostream &outputStream, List<Nodetype> &thlist)
{

  if (thlist.Isempty()){ 
   outputStream << "empty";
   return outputStream;
  }
  Listnode<Nodetype> *currentptr = thlist.firstptr;
  while (currentptr != NULL) {
            outputStream << currentptr -> getdata() << ' ';
            currentptr = currentptr -> getnext();
  }      
  return outputStream;
}


//It assumes no ordering on the two lists
template <class Nodetype> 
bool List<Nodetype>::Isubset(List<Nodetype> *A, int s) 
{
     if (A -> thesize > thesize - s) return false;
     int existflg; 
     Nodetype a;
     Listnode<Nodetype> *temptr1,  *temptr2;
     temptr1 = A -> firstptr;
     temptr2 = firstptr;
     for (; temptr1; temptr1 = temptr1 -> nextptr){
           a = temptr1 -> data;
           existflg = 0;
           for (;temptr2; temptr2 = temptr2 -> nextptr){
                 if ( a == temptr2 -> data){
                             existflg = 1; 
                             break;
	         }
	   }
           if (existflg == 0)  return false;
           temptr2 = firstptr;
     }
     return true;
 }


/*

template <class Nodetype> 
bool List<Nodetype>::Isublist(List<Nodetype> *A)  
{
   int existflg; 
   Nodetype a;
   //int f=0;
   Listnode<Nodetype> *temptr1,  *temptr2;
   if (A -> thesize > thesize) return false;
   temptr1 = A -> firstptr; 
   temptr2 = firstptr->nextptr->nextptr;
   for (; temptr1; temptr1 = temptr1 -> nextptr){
        //f++;
        a = temptr1 -> data;
        existflg = 0;
        for (;temptr2; temptr2 = temptr2 -> nextptr){
              if ( a == temptr2 -> data){
                          existflg = 1; 
                          temptr2 = temptr2 -> nextptr;
                          break;
	      }
	}
        if (existflg == 0){
                      //if (A->firstptr->combl < f) A->firstptr ->combl = f; 
                      return false;
	}
        //temptr2 = firstptr;
   }
   return true;
}
template <class Nodetype> 
bool List<Nodetype>::Is_subset(List<Nodetype> *A)  
{
   int existflg; 
   Nodetype a;
   Listnode<Nodetype> *temptr1,  *temptr2;
   //if this if statement is used the  of returned itemsets is the maximal 
   //ones plus more.
   //if ((A->thesize - 2) >(thesize-firstptr ->combl)) return false;
   temptr1 = A -> firstptr->nextptr ->nextptr; 
   temptr2 = loc;
   int c1 = 2;
   for (; temptr1; temptr1 = temptr1 -> nextptr){
        c1++;
        a = temptr1 -> data;
        existflg = 0;
        for (;temptr2; temptr2 = temptr2 -> nextptr){
                 if (a == temptr2 -> data){
                          existflg = 1; 
                          temptr2 = temptr2 -> nextptr;
                          break;
	         }
	}
        if (existflg == 0){
                        if (A->firstptr->combl < c1) A->firstptr ->combl = c1; 
                        return false;
	}
   }
   return true;
}

template <class Nodetype> 
bool List<Nodetype>::Is_sublist(List<Nodetype> *A)  
{
   int existflg; 
   Nodetype a;
   Listnode<Nodetype> *temptr1,  *temptr2;
   //if this if statement is used the # of returned itemsets is the maximal 
   //ones plus more.
   //if ((A->thesize - 2) >(thesize-firstptr ->combl)) return false;
   temptr1 = A -> firstptr->nextptr ->nextptr;
   temptr2 = firstptr;
   int c1 = 2;
   for (; temptr1; temptr1 = temptr1 -> nextptr){
        a = temptr1 -> data;
        c1++;
        existflg = 0;
        for (;temptr2; temptr2 = temptr2 -> nextptr){
                 if (a == temptr2 -> data){
                          existflg = 1; 
                          //c1++;
                          break;
	         }
	}
        if (existflg == 0){
                         //c1++;
                         if (A->firstptr->combl < c1) A->firstptr ->combl = c1;
                         return false;
	}
        temptr2 = firstptr;
   }
   return true;
}

template <class Nodetype> 
bool List<Nodetype>::subsetnew(List<Nodetype> *A)  
{
   int existflg; 
   Nodetype a;
   Listnode<Nodetype> *temptr1,  *temptr2;
   //if this if statement is used the # of returned itemsets is the maximal 
   //ones plus more.
   //if ((A->thesize - 2) >(thesize-firstptr ->combl)) return false;
   temptr1 = A -> firstptr->nextptr->nextptr->nextptr;
   temptr2 = loc;
   int c1 = 3;
   for (; temptr1; temptr1 = temptr1 -> nextptr){
        a = temptr1 -> data;
        c1++;
        existflg = 0;
        for (;temptr2; temptr2 = temptr2 -> nextptr){
                 if (a == temptr2 -> data){
                          existflg = 1; 
                          //c1++;
                          break;
	         }
	}
        if (existflg == 0){
                         //c1++;
                         if (A->firstptr->combl < c1) A->firstptr ->combl = c1;
                         return false;
	}
        temptr2 = loc;
   }
   return true;
}

template <class Nodetype> 
bool List<Nodetype>::subsetnew2(List<Nodetype> *A)  
{
   int existflg; 
   Nodetype a;
   Listnode<Nodetype> *temptr1,  *temptr2;
   //if this if statement is used the # of returned itemsets is the maximal 
   //ones plus more.
   if ( A->thesize  >(thesize-firstptr ->combl)) return false;
   temptr1 = A -> firstptr;
   temptr2 = loc;
   //int c1 = 3;
   for (; temptr1; temptr1 = temptr1 -> nextptr){
        a = temptr1 -> data;
        //c1++;
        existflg = 0;
        for (;temptr2; temptr2 = temptr2 -> nextptr){
                 if (a == temptr2 -> data){
                          existflg = 1; 
                          //c1++;
                          break;
	         }
	}
        if (existflg == 0){
                         //c1++;
                       //if (A->firstptr->combl < c1) A->firstptr ->combl = c1;
                         return false;
	}
        temptr2 = loc;
   }
   return true;
}

*/

#endif













