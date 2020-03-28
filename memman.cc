#include "memman.h"
#include "partition.h"

void Memman::read_from_disk(Itemset *iset, int it)
{
   int supsz = partition_get_idxsup(it);   
   partition_read_item(iset->tidlist()->get_array(), it);
   iset->set_support(supsz);
}
