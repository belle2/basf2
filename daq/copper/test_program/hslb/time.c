/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
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
