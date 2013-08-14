#include   <stdio.h>
#include   <time.h>
void   main() 
{ 
    time_t   lt;   /*define   a   longint   time   varible*/ 
    lt=time(NULL);/*system   time   and   date*/ 
    printf(ctime(&lt));   /*english   format   output*/ 
    printf(asctime(localtime(&lt)));/*tranfer   to   tm*/ 
    printf(asctime(gmtime(&lt)));   /*tranfer   to   Greenwich   time*/ 
}
