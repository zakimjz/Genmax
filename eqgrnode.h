#ifndef _EQGRNODE_H
#define _EQGRNODE_H

#include <iostream>
#include <stdio.h>

class EqGrNode {
friend ostream &operator << (ostream &, EqGrNode &);
public:
   EqGrNode(int);
   ~EqGrNode();
   int getflg(){return theFlg;};
   void setflg(int val){theFlg=val;};
   int * elements(){return theElements;};
   int num_elements(){return numElements;};
   void add_element(int el, int pos){theElements[pos] = el;};
   int get_element(int pos){return theElements[pos];};
   void remove_el(int);
private:
   int *theElements;
   int numElements;
   int theFlg; //indicates if class is in memory
   
};


// constructor 
EqGrNode::EqGrNode(int sz)
   {
      numElements = sz;
      theElements = new int[sz];
      theFlg = 0;
   }
   
EqGrNode::~EqGrNode()
   {
      delete [] theElements;
      theElements = NULL;
      theFlg = 0;
   }

inline void EqGrNode::remove_el(int pos)
   {
      for (int i=pos; i < numElements-1; i++)
         theElements[i] = theElements[i+1];
      numElements--;
   }

   
ostream &operator << (ostream &outputStream, EqGrNode &EQ)
{
      cout << "ELEMENTS : ";
      for (int i = 0; i < EQ.numElements; i++){
         cout << EQ.theElements[i] << " ";
      }
      cout << "(" << EQ.numElements << ")";
      cout << endl;
      return outputStream;
}
   
#endif



















