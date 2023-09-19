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

    char timeLoc[100], timeGMT[100];
    std::strftime(timeLoc, sizeof(timeLoc), "%Y-%m-%d %H:%M:%S\n", localtime(&lt));
    std::strftime(timeGMT, sizeof(timeGMT), "%Y-%m-%d %H:%M:%S\n", gmtime(&lt));

    printf(timeLoc);/*tranfer   to   tm*/ 
    printf(timeGMT);   /*tranfer   to   Greenwich   time*/ 
}
