//This implementation use the idea of diff lists all the way
#include <errno.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <sys/stat.h>
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

boolean sort_ascend = TRUE;

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
double extend_time=0, list_intersect_time=0, read_dbase_time=0;
double tid_difference_time=0, super_set_time=0, t11=0, t10=0;
double t9=0, t8=0, t7=0, t6=0, t5=0, t50=0, t40=0;
double L2TIME=0, find_time;

int  INTERSECT_THRESHOLD;
int DBASE_NUM_TRANS;
int DBASE_MAXITEM, yesize_old;
float DBASE_AVG_TRANS_SZ;
int DBASE_MINTRANS; //works only with 1 partition
int DBASE_MAXTRANS;
double MINSUP_PER;
int MINSUPPORT;
List<int> *F1;
extern int num_partitions;


void parse_args(int argc, char **argv)
{
  extern char * optarg;
   int c;
   
   if (argc < 2)
      cout << "usage: a.out -i<infile>  -s<support>\n";
   else{
      while ((c=getopt(argc,argv,"abcCe:fi:ors:St:w:m"))!=-1){
         switch(c){
         case 'a':
            use_auto_maxthr = 0;
            break;
         case 'C':
            use_char_extl2 = 1;
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

         }
      }
   }

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
bool has_sup(List<int> *L, List<List<int> *> *U)
{
  Listnode<List<int> *> *temptr = U -> head();
  while (temptr){
      if ((temptr->getdata())->Isubset(L)) return true;
      temptr = temptr -> getnext();
  }
  return false;
}

// This function finds all lists in the family M which contain the 
// element i and add them to the family Z.
void find_sets(List<List<int> *> *Z, List<List<int> *> *M, int i )
{
  Listnode<List<int> *> *temptr = M ->head();
  while (temptr){
      if ((temptr->getdata())-> Ismember(i)){
	                        Z -> addatback(temptr->getdata());
      }
      temptr = temptr -> getnext();
  }
}


void Diff1(Itemset *join, Itemset *it1, Itemset *it2)
{ 
   num_intersect++;
   int i,j, f;
   for (i=0,j=0; i < it1->support() && j < it2->support();){
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
      }
   } 
   for (f=i; f < it1->support(); f++)  join->add_tid(it1->tid(f));
   join ->set_support(it1->support()- join->tidsize());
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
   if (breakflg == 1)   join->set_support(0);
   else {
              for (f=i; f < it1->tidsize(); f++)  join->add_tid(it1->tid(f));
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

void Extend(Itemset *I,List<Itemset *> *X, List<List<int> *> *Y, List<int> *t)

{
   
         List<int> *G, *tail_list;
         Listnode<int> *cptr;
         Listnode<Itemset *> *temptr, *ptr, *xxptr;
         List<Itemset *> *NewX, *Xtem;
         List<List<int> *> *NewY;
         Itemset *NewI, *it1;
         Listnode <List<int> *> *ttptr;
         int m, f, j, size_old; 

         tail_list =  new List<int>;
         cptr = t -> head();
         while (cptr){
               tail_list -> addatback(cptr ->getdata());
               cptr = cptr -> getnext();
         }
         //seconds(t7);
         Xtem = new List<Itemset *>;
         ptr = X->head();
         for (; ptr; ptr = ptr -> getnext()){
                  m= ptr->getdata()->itemset()->head()->getdata();
                  it1 = new Itemset(ptr->getdata()->tidtotsize());
                  Diff2(it1, ptr->getdata(), I);                  
                  if (it1->tidsize()== 0){
                                 tail_list -> addatfront(m);
                                 delete it1;
		  }
                  else if (it1 -> support() >= MINSUPPORT){ 
                               it1 -> add_item(m);
                               Xtem -> sorted_descend(it1, it1->tidsize());
		       }
                       else delete it1;
 	 }
         //seconds(t8);
         //tid_difference_time += t8 - t7;

         if (Xtem ->size()< 2){ 
                   if (Xtem ->size()== 1)  tail_list ->addatfront(Xtem ->head()->getdata()->itemset()->head()->getdata()); 
                  cptr = I -> itemset() ->head();
                  for (; cptr; cptr = cptr ->getnext()){
                          tail_list -> addatfront(cptr -> getdata());
                  }
                  if (has_sup(tail_list, Y))  delete tail_list;
                  else {
                       Y -> addatback(tail_list); 
		  }
         }
         else {
           temptr = Xtem ->head();
           for (; temptr; temptr = temptr->getnext()){
               if (Y -> size() > 0 ){
                  //seconds(t40);
                            G = new List<int>; 
                            ptr = temptr;
                            while (ptr){
                                 G ->addatback(ptr -> getdata() -> itemset()->head()->getdata());
                                 ptr = ptr ->getnext();
	                    }
                            cptr = tail_list ->head();
                            while (cptr){
                                     G -> addatback(cptr->getdata());
                                     cptr = cptr -> getnext();
	                    }
                            if (has_sup(G, Y)){
                                          delete G;        
                                          break;
	                    }
                            delete G;  
                            //seconds(t50);
                            //super_set_time += t50 - t40;
               }
               j = temptr -> getdata()->itemset()-> head() -> getdata();
               NewI = new Itemset(temptr -> getdata()->tidtotsize());
               for(int h=0; h < temptr->getdata()->tidsize(); h++){
                              NewI->add_tid(temptr ->getdata()->tid(h));
               }
               NewI->set_support(temptr -> getdata()->support());
               cptr = I -> itemset() ->head();
               for (; cptr; cptr = cptr ->getnext()){
                          NewI -> add_item(cptr -> getdata());
               }
               NewI ->add_item(j);
               NewY = new List<List<int> *>;
               //seconds(t40); 
               find_sets(NewY, Y, j);
               //seconds(t50);
               //find_time += (t50-t40);
               size_old = NewY->size();
               NewX = new List<Itemset *>; 
               xxptr = temptr -> getnext();
               while(xxptr){
                      NewX -> addatback(xxptr  -> getdata());
                      xxptr = xxptr -> getnext(); 
               }

               Extend (NewI, NewX, NewY, tail_list);

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
               delete NewX;
               delete NewI; 
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

   // To fill in the list F1 by comblnth(i) values and delete items with 
   // comblist values equal to 0, these items are maximal itemsets. 
   // Also compute the maximum and minimum levels.

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
int i, j, f, m, zsize_old, ysize_old;
Listnode <List<int> *> *ttptr;
    List<int> *H, *tail_list;
    Listnode<int> *temptr, *temptr1;
    Itemset *I, *it1;
    List<Itemset *> *X1, *X2;
    Listnode <Itemset *> *xptr, *ptr1, *xxptr;
    List <List<int> *> *M, *Z, *Y;

    M = new List<List<int> *>;
    temptr = F1 -> head();
    for (; temptr; temptr = temptr -> getnext()){
             i = temptr -> getdata();
             Memman::read_from_disk(item1,i); 
             X1 = new List<Itemset *>; 
             //seconds(t5);
             temptr1 = eqgraph2[i] -> head();
             for (; temptr1; temptr1 = temptr1 -> getnext()){
                     m = temptr1 -> getdata();
                     Memman::read_from_disk(item2, m);
                     it1 = new Itemset(DBASE_NUM_TRANS - item2 -> support());
                     Diff1(it1, item1, item2);
                     //stats.incrcand(item1->itemsetsize());
                     it1 -> add_item(m);
                     X1-> sorted_descend(it1, it1->tidsize());
             }
             //seconds(t6);
             //read_dbase_time += t6 - t5;
             Z= new List<List<int> *>;
             //seconds(t40);
             find_sets(Z, M, i);
             //seconds(t50);
             //find_time += (t50-t40);
             zsize_old = Z->size();
             xptr = X1 -> head();
             for (; xptr; xptr = xptr -> getnext()){
                 if ( Z -> size() > 0){   
                       H = new List<int>; 
                       ptr1 = xptr;
                       while (ptr1){
                            H ->addatback(ptr1 ->getdata()->itemset()->head()->getdata());
                            ptr1 = ptr1 ->getnext();
	               }
                       if (has_sup(H, Z)){ 
                              delete H;        
                              break;               
	               }
                       delete H; 
	         }
                 j = xptr -> getdata()->itemset()->head()->getdata();
                 //cout << "j is " << j<<endl;
                 I = new Itemset(xptr -> getdata()->tidtotsize());
                 for(int h=0; h < xptr->getdata()->tidsize();h++){
                         I -> add_tid(xptr->getdata()->tid(h));
		  }
                  I->set_support(xptr -> getdata()->support());
                  I ->add_item(i);
                  I ->add_item(j);
                  Y = new List<List<int> *>;
                  //seconds(t40); 
                  find_sets(Y, Z, j);
                  //seconds(t50);
                  //find_time += (t50-t40);
                  ysize_old = Y->size();
                  tail_list = new List<int>;
                  X2 = new List<Itemset *>; 
                  xxptr = xptr -> getnext();
                  while(xxptr){
                      X2 -> addatback(xxptr  -> getdata());
                      xxptr = xxptr -> getnext();
	          }
                  //seconds(t1); 
                  Extend (I, X2, Y, tail_list);
                  //seconds(t2); 
                  //extend_time += (t2-t1);
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
                  delete X2;
                  delete tail_list;
                  if (I) delete I;
	     }
             Listnode <List<int> *> *zptr = Z -> head();
             f = 1;
             while (zptr){
                     if (f > zsize_old) M -> addatback(zptr -> getdata());
                     f += 1;
                     zptr = zptr -> getnext();
             }
             delete Z;
             clear_list_itemset(X1);
    }
 
    ttptr = M->head();
    if (print_output) cout << "\nM contains:" << endl;
    while (ttptr){
       if (print_output) cout <<*(ttptr->getdata()) << endl;
       Listnode <int> *mptr = ttptr->getdata()->head();
       int mcnt=0;
       while (mptr){
          mcnt++;
          mptr = mptr->getnext();
       }
       stats.incrlarge(mcnt-1);       
       ttptr = ttptr -> getnext();       
    }
    
    cout <<";Number of maximal frequent itemsets : "<< M->size()<< endl;  
}

int main(int argc, char **argv)
{
  double t20,t30, Sorting_time;

  F1 = new List<int>;

  seconds(ts);

  parse_args(argc, argv);
  partition_alloc(dataf, idxf);
  read_files(F1);

  //cout << "F1 before sorting is : " << *F1 << endl;
  seconds(t20);
  sorting(F1);
  seconds(t30);
  Sorting_time = t30 -t20;

  //cout << "F1 after sorting is : " << *F1 << endl;
  cout << "F1 Size is : " << F1->size() << endl;
  //Listnode<int> *lptr = F1->head();
  //while (lptr){
            //cout << "sup("<< lptr->getdata() << ") = "<< lptr->sup()<<" ";
            //lptr = lptr->getnext();
  //}
  //cout << "F1 Sorting time is  : " << Sorting_time << endl;

  procces_comblists(F1);

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
  
  summary << "GENMAX " << dataf << " " << MINSUP_PER << " "
          << DBASE_NUM_TRANS << " " << MINSUPPORT << " ";  

   seconds(te);

   cout << " Time used for extend: " << extend_time << endl;
   cout << " Time used for find operation: " << find_time << endl;
   cout << " Time used for Differences: " << tid_difference_time 
        << endl;
   cout << " Time used for supper set checking in extend:  "<< super_set_time 
        <<endl;
   cout << " Time used for reading external data base and" << endl 
        << "      Computing lower level Diferences:   " << read_dbase_time 
        << endl;
   cout << ";Total elapsed time " << te-ts 
        << ", NumIntersect " << num_intersect <<"\n";

   stats.tottime = te-ts;
   summary << stats << " " << num_intersect << endl;
   
  exit(0);
}

