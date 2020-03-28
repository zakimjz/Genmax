#ifndef __MEMMAN_H
#define __MEMMAN_H

#include "itemset.h"

class Memman{
private:
public:
   static void read_from_disk(Itemset *iset, int it);
};

#endif //__MEMMAN_H
