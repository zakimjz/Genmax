#include <iostream>
#include <stdio.h>
#include <errno.h>
#include "itemset.h"

// the constructor
Itemset::Itemset(int list_sz){
  theitemset = new List<int>;
  if ( theitemset == NULL){
       perror ("memory::Itemset");
       exit(errno);
  }
  thetidlist = new Array(list_sz);
  if (thetidlist == NULL){
     perror ("memory::Itemset");
     exit(errno);
  }
  
  thesupport = 0;

}

// the destructor
Itemset::~Itemset()
{
  if (theitemset) delete theitemset;
  if (thetidlist) delete thetidlist;

  theitemset = NULL;
  thetidlist = NULL;
  thesupport = 0;
}
ostream &operator << (ostream &outputStream, Itemset &thitemset)
{

  outputStream << "ISET: ";
  //output << theitemset -> print() << " "; 
  outputStream << *thitemset.itemset() << " "; 
  outputStream << "SUP: ";
  outputStream << thitemset.thesupport<< " ";
  //outputStream << "\n";
  return  outputStream;
}


