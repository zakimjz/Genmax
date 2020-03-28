//This implementation use the idea of diff lists all the way
#include <errno.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <string.h>
#include <strings.h>
#include <sys/mman.h>
#ifdef SGIf
#endif

#include "temlist.h"
#include "memman.h"
#include "itemset.h"
#include "partition.h"
#include "extl2.h"
#include "eqgrnode.h"
#include "assoc.h"
#include "Graph.h"
 
#define NONMAXFLG -2 
struct timeval tp;   

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

//--------------------------------------
#include "stats.h" //MJZ
double Stats::tottime = 0;
int Stats::totcand = 0;
int Stats::totlarge = 0;
Stats stats;
ofstream summary("summary.out", ios::app);
//--------------------------------------

char dataf[300];
char idxf[300];
char conf[300];
char tempf[300];

char it2f[300];
long TOTMEM=0;

extern Graph *F2Graph;

int data_fd, idx_fd, it2_fd;  
int idxflen, it2flen;         
//int *it2_cnt; //counts for 2-itemsets
int maxitemsup, min1, max1;
//int *offsets;
Itemset *item1, *item2; // for use in reading external dbase
List<int> **eqgraph2;
double MAXIMAL_THRESHOLD = 1.0;
int use_auto_maxthr = 1;

boolean sort_ascend = TRUE, check_status;

int num_intersect=0;
double ts, te;
FILE *out;
int maxiter = 1;
long tmpflen=0;
char use_char_extl2=1;
char ext_l2_pass=1;  
char use_simple_l2=0;
char print_output=0; 
char diff_input=0;
char use_diff = 0;
char use_diff_f2=0;

double extend_time=0, list_intersect_time=0, read_dbase_time=0;
double tid_difference_time=0, super_set_time=0, t11=0, t10=0;
double t9=0, t8=0, t7=0, t6=0, t5=0, t50=0, t40=0;
double L2TIME=0, find_time, copy_time=0, h1, h2, a1, a2;

//int  INTERSECT_THRESHOLD;
int DBASE_NUM_TRANS;
int DBASE_MAXITEM;
float DBASE_AVG_TRANS_SZ;
int DBASE_MINTRANS; //works only with 1 partition
int DBASE_MAXTRANS;
double MINSUP_PER;
int MINSUPPORT;
List<int> *F1;
List<Array *> *M;
extern int num_partitions;


void parse_args(int argc, char **argv)
{
  extern char * optarg;
   int c;
   
   sprintf(tempf,"/tmp/tmp");
   
   if (argc < 2)
      cout << "usage: a.out -i<infile>  -s<support>\n";
   else{
      while ((c=getopt(argc,argv,"abcCdDe:fi:lors:St:w:mx:z"))!=-1){
         switch(c){
         case 'a':
            use_auto_maxthr = 0;
            break;
         case 'C':
            use_char_extl2 = 1;
            break;
         case 'd':
            use_diff=1;
            break;            
         case 'D':
            diff_input = 1;
            use_diff = 1;
            break;            
         case 'e':
            num_partitions = atoi(optarg);
            ext_l2_pass = 1;
            break;            
         case 'i':
            sprintf(dataf,"%s.tpose", optarg);
            sprintf(idxf,"%s.idx", optarg);
            sprintf(conf,"%s.conf", optarg);
            sprintf(it2f,"%s.2it", optarg);
            break;
         case 'l':
            use_diff_f2 = 1;
            use_diff =1;            
            break;            
         case 'o':
            print_output = 1;
            break;
         case 's':
            MINSUP_PER = atof(optarg);
            break;
         case 'S':
            use_simple_l2=1;
            break;
         case 't':
            MAXIMAL_THRESHOLD = atof(optarg);
            break;
         case 'x':
            sprintf(tempf, "%s", optarg);
            break;
         case 'z':
            sort_ascend=FALSE;
            break;            
         }
      }
   }
   if (diff_input) use_diff_f2 = 0;
   
  c = open(conf, O_RDONLY);
  if (c < 0){
      perror("ERROR: invalid conf file\n");
      exit(errno);
  }
  read(c,(char *)&DBASE_NUM_TRANS,ITSZ);
  MINSUPPORT = (int)(MINSUP_PER*DBASE_NUM_TRANS+0.5);
  //ensure that support is at least 2
  if (MINSUPPORT < 2) MINSUPPORT = 2;
  read(c,(char *)&DBASE_MAXITEM,ITSZ);
  read(c,(char *)&DBASE_AVG_TRANS_SZ,sizeof(float));
  read(c,(char *)&DBASE_MINTRANS,ITSZ);  
  read(c,(char *)&DBASE_MAXTRANS,ITSZ);
  close(c);
  cout << "CONF " << DBASE_NUM_TRANS << " " << DBASE_MAXITEM << " "
        << DBASE_AVG_TRANS_SZ << endl;

}


// It is used for super set test.
bool has_sup(Array *L, List<Array *> *U, int s)
{
  Listnode<Array *> *temptr = U -> head();
  while (temptr){
      if ((temptr->getdata())->Isubset(L, s)) return true;
      temptr = temptr -> getnext();
  }
  return false;
}

// This function finds all lists in the family M which contain the 
// element i and add them to the family Z.
void find_sets(List<Array *> *Z, List<Array *> *M, int i )
{
  Listnode<Array *> *temptr = M ->head();
  while (temptr){
      if (temptr -> getdata() -> Ismember(i)){
                          
	                        Z -> addatback(temptr->getdata());
      }
      temptr = temptr -> getnext();
  }
}

void get_intersect(Itemset *join, Itemset *it1, Itemset *it2)
{
   int i,j;
   num_intersect++;
   if (it2->itemsetsize() > 1) stats.incrcand(it2->itemsetsize());
   
   int dc1 = it1->support()-MINSUPPORT;
   int dc2 = it2->support()-MINSUPPORT;
   int df1=0;
   int df2=0;
   int breakflg=0;
   
   for (i=0,j=0; i < it1->support() && j < it2->support();){
      if (df1 > dc1 || df2 > dc2){
         breakflg=1;        
         break;
      }
      
      if (it1->tid(i) > it2->tid(j)){
         j++;
         df2++;
      }
      else if (it1->tid(i) == it2->tid(j)){
         join->add_tid(it1->tid(i));
         join->support()++;
         j++;
         i++;
      }
      else{
         df1++;
         i++;
      }
   }
   if (breakflg == 1) join->set_support(0);   
}


void Diff1(Itemset *join, Itemset *it1, Itemset *it2)
{ 
   num_intersect++;
   if (it2->itemsetsize() > 1) stats.incrcand(it2->itemsetsize());
   int i,j, f, k=0, breakflg=0;
   int dc1 = it1 -> support()-  MINSUPPORT;   
   for (i=0,j=0; i < it1->support() && j < it2->support();){
      if (k > dc1) {
         breakflg = 1;
         break;
      }
      if (it1->tid(i) > it2->tid(j)){
         j++;
      }
      else if (it1->tid(i) == it2->tid(j)){
         j++;
         i++;
      }
      else{
         join ->add_tid(it1->tid(i));
         i++;
         k++;
      }
   } 
   for (f=i; f < it1->support() && k <= dc1; f++){
      join->add_tid(it1->tid(f));
      k++;
   }
   if (k > dc1) join->set_support(0);
   else{      
      join ->set_support(it1->support()- join->tidsize());
   }   
}

void Diff2(Itemset *join, Itemset *it1, Itemset *it2)
{ 
   num_intersect++;
   //cout << "SZZ " << it1->itemsetsize() << " "<<it2->itemsetsize() << endl;
   
   if (it2->itemsetsize() > 1) stats.incrcand(it2->itemsetsize());
   int i,j, f, k=0, breakflg=0;
   int dc1 = it2 -> support()-  MINSUPPORT;
   for (i=0,j=0; i < it1->tidsize() && j < it2->tidsize();){
      if (k > dc1) {breakflg = 1; break;}
      if (it1->tid(i) > it2->tid(j)){
         j++;
      }
      else if (it1->tid(i) == it2->tid(j)){
         j++;
         i++;
      }
      else{
         k++;
         join ->add_tid(it1->tid(i));
         i++;
      }
   } 
   //if (breakflg == 1)   join->set_support(0);
   //else {
   //   for (f=i; f < it1->tidsize(); f++)  join->add_tid(it1->tid(f));
   //   join ->set_support(it2->support()- join->tidsize());
   // }
   for (f=i; f < it1->tidsize() && k <= dc1; f++){
      join->add_tid(it1->tid(f));
      k++;
    }
   if (k > dc1) join->set_support(0);
    else{      
      join ->set_support(it2->support()- join->tidsize());
   }      
}

void clear_list_itemset(List<Itemset *> *X)
{
      Listnode <Itemset *> *node = X->head();
      while (node){
         X->set_head(node -> getnext());
          if (node -> getdata()) delete (node -> getdata());
         delete node;
         node = X->head();
      }
      delete X;
}

// Each node in the list it1 has two functions  comblnth() and sup().   
//Sorting in increasing order of comblnth() and then in increasing order 
//of sup().
//Radix sort on the key comblnth(i) and while doing this we insert a 
//new item in ascending order on the key sup(i).

List<int> *sorting (List<int> *it1)
{
   int digit;
   int Minlevel =  min1;
   int Maxlevel =  max1;
   //List<int> **level; 
   //level = new List<int> *[Maxlevel+1];
   List <int> *level[Maxlevel+1];
   for (int i = Minlevel; i<= Maxlevel; i++) 
                   level[i]= new List<int>;

   Listnode <int> *currentptr = it1 -> head();
   Listnode <int> *ptr;

   while (currentptr) {
     ptr = currentptr -> getnext();
     digit = currentptr -> comblnth();
     if ( level[digit]->Isempty()){
       level[digit]->set_head(currentptr); 
       level[digit]->set_last(currentptr); 
     }
     else {
           Listnode<int> *temptr = level[digit]->head();
           if ((currentptr -> sup())<= (temptr -> sup())){
             currentptr -> set_next(temptr);
             level[digit]->set_head(currentptr);
	   }
           else {
	     while (temptr != level[digit]->last()){
                  if ((currentptr -> sup())<= (temptr -> getnext()->sup())){
                     currentptr -> set_next (temptr -> getnext());
                     temptr -> set_next(currentptr);
                     break;
		  }
                  temptr = temptr -> getnext();
	     }
             if (temptr == level[digit]->last()){ 
                level[digit]->last()->set_next(currentptr);
                level[digit]->set_last(currentptr);
	     }
	   }
     }
     currentptr = ptr;
   }     
   //concatenate the lists level[i] to produce it1.
   currentptr = NULL; 
   for (int i = Maxlevel; i>= Minlevel; i-- ) 
      if (level[i]->head()){ 
              level[i]->last() -> set_next(currentptr);
              currentptr = level[i]->head();  
      }
   it1->set_head(currentptr);
   return it1;
}


//Sorting the combine lists in the order of F1
// eqgraph2[i] is a pointer to the combine lists c(i).
//eqgraph_temp is a pointer to the new sorted combine list eqgraph[i].

void procces_comblists(List<int> *F1)
{
   int a,c;
   List <int> *eqgraph2_temp;
   Listnode <int> *ptr1, *ptr2;
   Listnode <int> *currentptr = F1 -> head();
   while (currentptr){
         ptr1 = F1 -> head();
         c = currentptr -> getdata();
         while (ptr1 != currentptr){
                   a = ptr1->getdata();
                   eqgraph2[c] -> deleted(a);
                   ptr1 = ptr1 -> getnext();        
	 }
         ptr2 = currentptr -> getnext(); 
         eqgraph2_temp = new List<int>;
         while (ptr2){
               a = ptr2 -> getdata();
               if (eqgraph2[c]->Ismember(a)) eqgraph2_temp ->addatback(a);
               ptr2 = ptr2 -> getnext(); 
	 }
         delete eqgraph2[c];    
         eqgraph2[c] = NULL;
         eqgraph2[c] = eqgraph2_temp;  
         currentptr = currentptr -> getnext();
   }
}


void Extend(Itemset *I, Listnode<Itemset *> *X, List<Array *> *Y, Array *t,
            int iter, int l)
{
   
   //cout << "ITER " << iter << " " << *I << endl;
         Array  *G, *tail_list;
         Listnode<int> *cptr;
         Listnode<Itemset *> *temptr, *ptr;
         List<Itemset *> *Xtem;
         List<Array *> *NewY;
         Itemset *it1;
         //int it1sup;
         Listnode <Array *> *ttptr;
         int m, f, j, size_old, s; 

         tail_list = new Array(t->size() + I->itemsetsize() + l + 1);
         //tail_list = new Array(t->size() + I->itemsetsize() + X->size() + 1);
         for (int g = 0; g < t -> size(); g++) tail_list -> add((*t)[g]); 

         seconds(t7);
         Xtem = new List<Itemset *>;
         ptr = X;
         for (; ptr; ptr = ptr -> getnext()){
                  m = ptr->getdata()->itemset()->head()->getdata();
                                    
                  if (use_diff_f2 || diff_input || (use_diff && iter > 2)){
                     it1 = new Itemset(min(I->support()-MINSUPPORT+1,
                                           ptr->getdata()->tidsize()));
                     Diff2(it1, ptr->getdata(), I);
                  }                  
                  else if (use_diff && iter == 2){      
                     it1 = new Itemset(I->support()- MINSUPPORT +1);
                     //cout << "DIF1 " << *I << endl;
                     //cout << "\t " << *(ptr->getdata()) << endl;
                     
                     Diff1(it1,I,ptr->getdata());                  
                  }                  
                  else{
                     it1 = new Itemset(min(ptr->getdata()->support(),
                                           I->support()));
                     get_intersect(it1,ptr->getdata(), I);                   
                  }

                  //it1sup = it1->support();
                  
                  if ((use_diff_f2 || diff_input || 
                       (use_diff && I->itemsetsize() > 2))  
                      && it1->tidsize()== 0){

                     tail_list -> add(m);
                     delete it1;

		  }
                  else if ((!use_diff || (!use_diff_f2 && iter == 2))
                           && it1->tidsize() == I->tidsize()){

                     tail_list -> add(m);
                     delete it1;

                  }                  
                  else if (it1 -> support() >= MINSUPPORT){ 
                     
                     it1 -> add_item(m);
                     if (use_diff)
                        Xtem -> sorted_descend(it1, it1->tidsize());
                     else 
                        Xtem -> sorted_ascend(it1, it1->tidsize());
                     //Xtem -> sorted_descend(it1, it1->tidsize());
                     
                  }
                  else delete it1;
 	 }
         seconds(t8);
         tid_difference_time += t8 - t7;

         //tail_list->support() = it1sup;
         if (Xtem ->size()< 2){ 

                  if (Xtem ->size()== 1){   
                          tail_list ->add(Xtem ->head()->getdata()->itemset()->head()->getdata());

                 }
                 seconds(t40);
                 if (has_sup(tail_list, Y, I->itemsetsize())) delete tail_list;
                 else {
                       cptr = I -> itemset() ->head();
                       for (; cptr; cptr = cptr ->getnext())
                               tail_list -> add(cptr -> getdata());
                       Y -> addatback(tail_list); 

		 } 
                 seconds(t50);
                 super_set_time += t50 - t40;
                 check_status = TRUE;
         }
         else {
         
           s = Xtem -> size();
           temptr = Xtem ->head();
           for (; temptr; temptr = temptr -> getnext()){

               if (Y -> size() > 0 && check_status == TRUE){

                     seconds(t40);
                     G = new Array(Xtem->size() + tail_list->size() + 1);
                     ptr = temptr;
                     while (ptr){

                         G ->add(ptr->getdata()->itemset()->head()->getdata());
                         ptr = ptr ->getnext();
	             }
                     for (int g=0; g < tail_list->size(); g++) 
                                           G -> add((*tail_list)[g]);

                     if (has_sup(G, Y, I->itemsetsize())){

                                          delete G;        
                                          break;

	             }
                     delete G;  
                     seconds(t50);
                     super_set_time += t50 - t40;
               }
               seconds(h1);
               j = temptr -> getdata()->itemset()-> head() -> getdata();
               cptr = I -> itemset() ->head();
               for (; cptr; cptr = cptr ->getnext())
                         temptr->getdata() -> add_item(cptr -> getdata());
         
               seconds(h2);
               copy_time += (h2 - h1);
               
               NewY = new List<Array *>;
               seconds(t40); 
               find_sets(NewY, Y, j);
               seconds(t50);
               find_time += (t50-t40);
               size_old = NewY->size();
               s = s-1;
               check_status = FALSE;

          Extend (temptr->getdata(),temptr->getnext(),NewY,tail_list,iter+1,s);

               ttptr = NewY->head();
               f = 1;
               while (ttptr){
                    if (f > size_old){
                                 Y -> addatback(ttptr -> getdata());
	            }
                    f +=1;
                    ttptr = ttptr -> getnext();
               }
               delete NewY; 
           }
           delete tail_list;
         }
         if (Xtem->size() == 0) delete Xtem; 
         else clear_list_itemset(Xtem);
    
}



void read_files(List<int> *F1)
{

   int i,j,a,b;
   double te,ts;
   seconds(ts);
   
   maxitemsup = make_l1_pass(F1);
   
   seconds(te);
   
   iterstat *is = new iterstat(DBASE_MAXITEM, 0, te-ts);
   stats.add(is);
   
   ts = te;
   
   cout << "maxitemsup: " << maxitemsup <<endl;
   item1 = new Itemset(maxitemsup);
   item2 = new Itemset(maxitemsup);
 
   int l2cnt = make_l2_pass(ext_l2_pass, it2f);
   seconds(te);
   L2TIME = te-ts;
   cout << "\nL2TIME = " << te-ts << endl;
   
   is = new iterstat(DBASE_MAXITEM*(DBASE_MAXITEM-1)/2,0,te-ts);
   stats.add(is);
   
   ts = te;
 
   eqgraph2 = new List<int> *[DBASE_MAXITEM];
   bzero((char *)eqgraph2, DBASE_MAXITEM*sizeof(List<int> *));  

   for (i=0; i < DBASE_MAXITEM; i++){
      eqgraph2[i]= new List<int>;
   }

   cout << " MINSUPPORT is : " <<  MINSUPPORT;

   int idx;   
   int it1, it2;   
   int lcnt;   
   GrNode *grn;   
   for (i=0; i < Graph::numF1; i++){
      //cout << "\nITEM " << i;
      grn = (*F2Graph)[i];      
      it1 = grn->item();
      lcnt = grn->size();       
      if (lcnt > 0){
         for (j=0; j < grn->size(); j++){
            it2 = (*F2Graph)[(*grn)[j]->adj()]->item();            
            eqgraph2[it1]->addatback(it2);
            eqgraph2[it2]->addatback(it1);
         }
      }
   }
   //delete offsets;

   // To fill in the list F1 by comblnth(i) values for each i in F1 
   // and delete the items which have  
   // comblist values equal to 0, these items are maximal itemsets. 
   // Also compute the maximum and minimum levels required for the sorting 
   // routine.

   Listnode<int> *ptr = F1 ->head();
   Listnode<int> *temptr = NULL;
   min1 = F1 ->size();
   max1 = 0;
   while (ptr){
       a = ptr -> getdata();
       b = eqgraph2[a]->size();
       if (b > 0){
                  ptr -> set_comblnth(b);
                  if (b < min1)   min1 = b;
                  if (b > max1)   max1 = b;
                  temptr = ptr;
       }
       else {
             if (!temptr)  F1 -> set_head(ptr->getnext());
             else  temptr ->set_next(ptr -> getnext());
             Array *sptr = new Array(2);
             sptr -> add(a);
             M -> addatback(sptr);
       }
       ptr = ptr ->getnext();
   }
   seconds(te);

   cout << "\nConstruction time for the combine lists = " << te-ts << endl;

    //cout << "The combine Lists before sorting are: " << endl;
   //for (i=0; i < DBASE_MAXITEM; i++){   
     //  if (eqgraph2[i]->size()>0) cout <<"Combine list of item ("<<i<<") is "
       //                     <<*eqgraph2[i]<< endl;
   //} 
}

void Max_itset_algo()
{
    double t1,t2;
    int i, j, f, m, zsize_old, ysize_old, s;
    Listnode <Array *> *ttptr;
    Array *H, *tail_list, *cp;
    Listnode<int> *temptr, *temptr1;
    Itemset *it1;
    List<Itemset *> *X1;
    Listnode <Itemset *> *xptr, *ptr1;
    List <Array *> *Z, *Y;

    temptr = F1 -> head();
    for (; temptr; temptr = temptr -> getnext()){

       i = temptr -> getdata();

       Memman::read_from_disk(item1,i); 
       X1 = new List<Itemset *>; 
             
       cp = new Array ( eqgraph2[i] -> size()+1);
       cp -> add(i);
       temptr1 = eqgraph2[i] -> head();
       for (;temptr1;temptr1=temptr1->getnext()) cp->add(temptr1->getdata());
       if (!has_sup(cp, M, 0)){

             Z= new List<Array *>;
             seconds(t40);
             find_sets(Z, M, i);
             seconds(t50);
             find_time += (t50-t40);
             zsize_old = Z->size();             
             seconds(t5);
             temptr1 = eqgraph2[i] -> head();
             for (; temptr1; temptr1 = temptr1 -> getnext()){
                     m = temptr1 -> getdata();
                     Memman::read_from_disk(item2, m);
                     if (use_diff_f2){
                        it1 = new Itemset(item1 -> support());
                        Diff1(it1, item1, item2);
                     }                     
                     else if (diff_input){
                        Diff2(it1, item2, item1);
                        it1 = new Itemset(
                                          min(item1->support()-MINSUPPORT+1,
                                              item2 -> tidsize()));
                     }                     
                     else{
                        it1 = new Itemset(min(item1->support(),
                                              item2->support()));    
                        get_intersect(it1, item1, item2);
                     }                     
                     //stats.incrcand(item1->itemsetsize());
                     it1 -> add_item(m);
                     //cout << "TT " << i << " "<< m << " " << *it1 << endl;
                     if (use_diff_f2 || diff_input) 
                        X1-> sorted_ascend(it1, it1->support());
                     else X1-> sorted_ascend(it1, it1->tidsize());
                     //X1-> sorted_descend(it1, it1->tidsize());
                     
             }
             seconds(t6);
             read_dbase_time += t6 - t5;

             s = X1 -> size();
             xptr = X1 -> head();
             for (; xptr; xptr = xptr -> getnext()){

                  if ( Z -> size() > 0){

                       seconds(t40);
                       H = new Array(X1->size());
                       ptr1 = xptr;
                       while (ptr1){

                            H ->add(ptr1->getdata()->itemset()->head()->getdata());
                            ptr1 = ptr1 ->getnext();

	               }
                       if (has_sup(H, Z, 1)){ 

                              delete H;        
                              break;               
	               }
                       delete H; 
                       seconds(t50);
                       super_set_time += t50 - t40;

	          }

                  seconds(a1);
                  j = xptr -> getdata()->itemset()->head()->getdata();
                  xptr -> getdata() -> add_item(i);
                  seconds(a2);
                  copy_time += (a2-a1);

                  Y = new List<Array *>;
                  seconds(t40); 
                  find_sets(Y, Z, j);
                  seconds(t50);
                  find_time += (t50-t40);
                  ysize_old = Y->size();

                  tail_list = new Array(1);
                  s = s-1;
                  check_status = FALSE;
                  seconds(t1); 
                  Extend (xptr->getdata(),xptr->getnext(),Y,tail_list,2,s);
                  seconds(t2); 
                 extend_time += (t2-t1);

                  //cout << "\nY after extend contains:" << endl;
                  ttptr = Y->head();
                  f = 1;
                  while (ttptr){
                        //cout <<*(ttptr->getdata()) << endl;
                        if (f >ysize_old){
                                 Z -> addatback(ttptr -> getdata());
			}
                        f +=1;
                        ttptr = ttptr -> getnext();
	          }
                  delete Y; 
                  delete tail_list;
	     }
             ttptr = Z -> head();
             f = 1;
             while (ttptr){

                     if (f > zsize_old) M -> addatback(ttptr -> getdata());
                     f += 1;
                     ttptr = ttptr -> getnext();
             }
             delete Z;
             clear_list_itemset(X1);
       }
    }
 
    ttptr = M->head();
    if (print_output) cout << "\nM contains:" << endl;
    while (ttptr){
       if (print_output) cout <<*(ttptr->getdata()) << endl;
       //Listnode <int> *mptr = ttptr->getdata()->head();
       //int mcnt=0;
       //while (mptr){
       //   mcnt++;
       //   mptr = mptr->getnext();
       //}
       int mcnt = ttptr->getdata()->size();       
       stats.incrlarge(mcnt-1);       
       ttptr = ttptr -> getnext();       
    }
    
    cout <<";Number of maximal frequent itemsets : "<< M->size()<< endl;  
}


int main(int argc, char **argv)
{

  double t20,t30, Sorting_time, h3, h4, procces_comblists_time;

  F1 = new List<int>;
  M = new List<Array *>;

  seconds(ts);

  parse_args(argc, argv);
  partition_alloc(dataf, idxf);
  read_files(F1);

  //cout << "F1 before sorting is : " << *F1 << endl;
  seconds(t20);
  sorting(F1);
  seconds(t30);
  Sorting_time = t30 -t20;

  cout << "F1 Size is : " << F1->size() << endl;

  //cout << "F1 after sorting is : " << *F1 << endl;
  //Listnode<int> *lptr = F1->head();
  //while (lptr){
            //cout << "sup("<< lptr->getdata() << ") = "<< lptr->sup()<<" ";
            //lptr = lptr->getnext();
  //}
  //cout << "F1 Sorting time is  : " << Sorting_time << endl;

  seconds(h3);
  procces_comblists(F1);
  seconds(h4);
  procces_comblists_time =(h4 - h3);

  //cout << "The combine Lists are: " << endl;
  //Listnode<int> *fptr= F1 -> head();
  //for (;fptr; fptr = fptr->getnext()){    
   //    int b = fptr ->getdata(); 
     //  if (eqgraph2[b]->size()>0) cout <<"combine set of item ("<<b<<") is "
       //                     << *eqgraph2[b]<<endl;
   //} 

  Max_itset_algo();
   
  //close(it2_fd);
  //munmap((char *)it2_cnt, it2flen);
  
  partition_dealloc();
  
  summary << "GENMAX ";
  if (diff_input) summary << "DIFFIN ";
  else if (use_diff_f2) summary << "DIFF2 ";
  else if (use_diff) summary << "DIFF ";
  summary << dataf << " " << MINSUP_PER << " "
          << DBASE_NUM_TRANS << " " << MINSUPPORT << " ";  

   seconds(te);

   cout << ";Time used for extend: " << extend_time << endl;
   cout << ";Copy Time: " << copy_time << endl;
   cout << ";Procces_comblists_time: " <<  procces_comblists_time << endl;
   cout << ";Time used for find operation: " << find_time << endl;
   cout << ";Time used for Differences: " << tid_difference_time 
        << endl;
   cout << ";Time used for supper set checking in extend:  "<< super_set_time 
        <<endl;
   cout << ";Time used for reading external data base and" << endl 
        << "          Computing lower level Diferences:   " << read_dbase_time 
        << endl;
   cout << ";Total elapsed time " << te-ts 
        << "; NumIntersect " << num_intersect <<"\n";

   stats.tottime = te-ts;
   summary << stats << " " << num_intersect;
   struct rusage ruse;   
   getrusage(RUSAGE_SELF,&ruse);   
   summary << " " << getsec(ruse.ru_utime) << " "
           << getsec(ruse.ru_stime) << endl;  
   
  exit(0);
}








