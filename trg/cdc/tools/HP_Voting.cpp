/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdio>
#include <vector>
#include <math.h>
#include <iomanip>
using namespace std;

int main()
{   

  ofstream outputp("HP_Voting_0to39.txt",fstream::app);

  for(int i=1;i<17;i++){
    for (int t=0;t<40;t++){   

      int j=t%160;

      //   outputp<<"V_"<<k<<": and_gate5"<<endl;
      //    outputp<<"port map(";
      // k=k+1;

      outputp<<"row"<<i<<"("<<j<<")<=";

      for(int u=0;u<5;u++){
	outputp<<"SL"<<2*u<<"_row"<<i<<"("<<j<<") and ";
	//    outputp<<setw(10)<<"I"<<u<<"=>HP_SL"<<2*u<<"_row"<<i<<"("<<j<<"),"<<endl;
      }

      outputp<<";"<<endl;

      //       outputp<<setw(25)<<"O=>HP_Minus_row"<<i<<"("<<j<<"));"<<endl;
      //       outputp<<""<<endl;
    }
  }
  return 0;
}
