// this file defines the class Array member functions 
#include <errno.h>
#include "Array1.h"

// the constructor
Array::Array(int sz)
{
  totSize = sz;
  thesize = 0;
  //  sup =0;
  ptr = NULL;
  if (sz > 0){
     ptr = new int[totSize]; 
     if (ptr == NULL){    
         perror("memory:: Array");
         exit(errno);
     }
  }
}


// the destructor 
Array::~Array()
{
  if (ptr) delete [] ptr;
  ptr = NULL;
}


void Array::add(int tr)
{

  ptr[thesize] = tr;
  thesize++;

}     


int Array::operator[] (unsigned int subscript)
{
  return ptr[subscript]; 
}


//this function does not assume any ordering on the array
bool Array::Ismember(int element)
{

  int i;
  for ( i=0; i< thesize; i++) if (element == ptr[i])  return true;
  return false; 
  
} 

//this function does not assume any ordering on the array
bool Array::Isubset(Array *A, int s)

{
   if (A -> thesize  > thesize - s) return false;
   int existflg; 
   int i, j;
   for (i=0; i < A->thesize; i++){
        existflg = 0;
        for (j=0; j<thesize; j++){
              if ( (A->ptr)[i] == ptr[j]){
                          existflg = 1; 
                          break;
	      }
	}
        if (existflg == 0)  return false;
   }
   return true;
}

ostream &operator<< (ostream &outputStream, Array &init)
{
 for (int i=0; i< init.thesize; i++)
    outputStream << init.ptr[i]<< ' '; 
 // outputStream << " - " << init.sup;
 return outputStream;
}



/*
// Sorting the array using quicksort
void Array::Quick_Sort()
{
  if (thesize > 1) qsort1(0, thesize-1);
}

void Array::qsort1(int left, int right)
{
     if (left < right){
        int i = left, j = right;
        int pivot = ptr[i];

        while (i < j){
	      while (ptr[i] <= pivot && i < right) i++;
	      while (ptr[j] >= pivot && j > left) j--;

	      if (i < j){
                       //swap(v, i, j)
                       int temp = ptr[i];
                       ptr[i] = ptr[j];
                       ptr[j] = temp;
              }
	}
        //swap(v, left, j)
        int temp = ptr[left];
        ptr[left] = ptr[j];
        ptr[j] = temp;

	qsort1(left, j-1);
	qsort1(j+1, right);
     }
}

//Ismember based on binary search
bool Array::Ismember(int element)
{

       if (element < ptr[0] || element > ptr[thesize - 1]) return false;

       int left= 0, 
            right = thesize - 1, 
            middle;
     
       while (left<= right){
               middle = (left + right) / 2;
               if (element == ptr[middle]) return true;
               else if (element > ptr[middle]) left = middle + 1;
               else right = middle - 1;
       }
       return false;

}

// Isubset based on binary search. the array A is not sorted.
bool Array::Isubset(Array *A, int s)

{
   if (A -> thesize  > thesize - s) return false;
   int i, j;
   for (i=0; i < A->thesize; i++){
        if ( !Ismember((A->ptr)[i])) return false;
   }
   return true;
}

//this function works if the two arrays are sorted in increasing order
bool Array::Isubset(Array *A, int s)

{
   if (A -> thesize  > thesize - s) return false;
   int existflg; 
   int i, j;
   for (i=0, j=0; i < A->thesize && j<thesize; ){
        if ((A->ptr)[i] == ptr[j]){
                i++;
                j++;
        }
        else if ((A->ptr)[i] > ptr[j]) j++;
        else if ((A->ptr)[i] < ptr[j] ) return false;

   }
   return true;
}

*/










