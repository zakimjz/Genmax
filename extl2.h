#ifndef __EXT_H_
#define __EXT_H_
#include "GArray.h"
#include "assoc.h"
#include "temlist.h"

class invdb: public GArray<GArray<int> *>{
public:
   invdb(int sz);
   ~invdb();
   void add_db(int tid, int it);
};

extern int make_l1_pass(List<int> *);
extern int make_l2_pass(boolean ext_l2_pass, char *it2f);

#endif //__EXT_H_
