// this file defines the class Array.

#ifndef ARRAY1_H 
#define ARRAY1_H 
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <malloc.h>

using namespace std;

class Array {
friend ostream &operator<< (ostream &, Array &);

public:

Array(int sz); 
~Array();          

void set_array(int *ary){ptr = ary;};
void set_size(int sz){ thesize = sz;};
void set_totsize(int sz){totSize = sz;};
void setitem(int pos, int val){ptr[pos] = val;};
void reset(){ thesize = 0;};
int *get_array(){ return ptr;};
unsigned int size() {return thesize;};
int totsize(){ return totSize;};
int item (int index) {return ptr[index];};
// int &support(){ return sup;  }
 
void add(int);
int operator[] (unsigned int);
//void Quick_Sort();
//void qsort1(int, int);
bool Ismember(int);
bool Isubset(Array *, int);


private:
   unsigned int totSize; 
   unsigned int thesize;
   int *ptr; 
   //   int sup;
};

#endif




