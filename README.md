# Genmax Algorithm

Genmax mines all maximal frequent itemsets via a backtracking approach with progressive focusing.
See [2001-genmax, 2005-genmax:dmkd]

[2001-genmax] Karam Gouda and Mohammed J. Zaki. Efficiently mining maximal frequent itemsets. In 1st IEEE International Conference on Data Mining. November 2001.

[2005-genmax:dmkd] Karam Gouda and Mohammed J. Zaki. Genmax: an efficient algorithm for mining maximal frequent itemsets. Data Mining and Knowledge Discovery: An International Journal, 11(3):223–242, November 2005. doi:10.1007/s10618-005-0002-x.


How to run genmax
----------------
input file must have XXX.data extension

        genmax -i XXX -d -l -e 1 -s <MINSUP>

        other flags
         -o output the patterns found

        NOTE:
         -d uses diffsets instead of tidsets (from length 3 onwards)

         -l uses diffsets for pass 2 as well
                (this should NOT be used for sparse datasets, since tidset
                 size of pass 2 is smaller than diffset size for
                 sparse sets.)
         
MINSUP is in fractions, i.e., specify 0.5 if you want 50% minsup or
0.01 if you want 1% support. 
You can use the -S flag to give absolute support (in which case omit -s flag)

the input database must be in binary, with the following format per row:

        CID TID #ITEMS LIST_OF_ITEMS
        1   1   4       0 1 4 6
        2   2   3       4 7 9

items in the list must be sorted in increasing order

Finally the summary of the run is stored in the summary.out
file. The format of this file is as follows:

GENMAX (other options) DB_FILENAME MINSUP NUMTRANS_IN_DB ACTUAL_SUPPORT
      [ ITER_i |Ci| |Fi| 0 ] 
      [TOT total_cands tot_freq tot_elapsed_time] 
      NumberofIntersections  user_time sys_time


Note3: -e 1 option is a flag indicating genmax to compute the support
of 2-itemsets from scratch. The number 1 says there is only one DB
partition that will be inverted entirely in main memory. If the
original DB is large then this inversion will obviously take too much
time. So in this case I recommend dividing the DB into chunks of size
roughly 5MB (assuming there is 32MB available to the process). The
exttpose program is equiped to handle this case. If you specify a <-p
NUMPART> flag to exttpose it will divide the DB into NUMPART
chunks. Now you can run charm with -e NUMPART option. You must do this
if the DB is large otherwise the timings for charm will be
skewed. Generally, the more the partitions the better the running time
for charm. For example:

        exttpose -i XXX -o XXX -l -a 0 -s LMINSUP -p 10
        genmax -i XXX -s MINSUP -e 10


In summary run:

    for dense datasets: genmax -i XXX -d -l -e 1 -s <MINSUP> 
    for sparse datasets: genmax -i XXX -d -e 1 -s <MINSUP>


conversion utilities
--------------------

Look at https://github.com/zakimjz/tposedb

1) Generate a data file using the IBM data generator program, gen (see
https://github.com/zakimjz/IBMGenerator)

   OR

   Start with an ascii file (say chess.ascii)

The format of the ascii/binary file should be

    \<cid\> \<tid\> \<numitem\> \<item list\>

2) If ascii file, first convert to binary using makebin

        makebin chess.ascii chess.data

Binary file MUST have .data extension

3) Get configuration by running getconf
(gen automatically generates conf file, so this step can be skipped)

       ./getconf -i chess -o chess -a


Before running the rest you should now have the following files

        chess.data
        chess.conf

4)  run: exttpose -i XXX -o XXX -l -s LMINSUP -a 0

        example: exttpose -i chess -o chess -l -s 0.2 -a 0

    or  exttpose -i chess -o chess -l -s 0 -a 0
        (this allows any minsup to be used later)

note: this produces the files XXX.tpose, and XXX.idx

The XXX.tpose file is the DB in vertical format, and
XXX.idx is an index file specifying where the tid-list for each item
begins.


You can specify a value of LMINSUP to be the same as the one you will use to
run genmax below, in which case you will have to rerun exttpose each time you
use a new lower MINSUP. Alternatively, you can use a small value for LMINSUP,
and it will continue to work for all values of MINSUP >= LMINSUP when you
run genmax.

So exttpose -i chess -o chess -l -s 0 -a 0 will work for any value of 
support you'll use later.

The time for inverting is stored in summary.out. The format is:

    TPOSE DB_FILENAME X NUMITEMS TOTAL_TIME

(see note one TOTAL_TIME below)

You should now have the following files:

        chess.data
        chess.conf
        chess.tpose
        chess.idx

