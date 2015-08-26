#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdio>
#include <vector>
#include <math.h>
#include <iomanip>
using namespace std;
int main()


{   int i,j,t,k ,p,u;
  ofstream outputp("Cluster_part2.txt",fstream::app);

   i=1;
   j=1;
   p=j+16;


  for(j;j<p;j+=2)
  {
     t=41;
     u=t+40;
        for(t;t<u;t+=2)
           {
  i=i++;

if( j==1 or j==15  )
{
    if( t==41  )
        {
            outputp<<"Minus_row"<<j<<"("<<t<<" downto "<<t-1<<")<=patternI_"<<t-1<<"_"<<j<<"_A(1 downto 0);"<<endl;
            outputp<<"Minus_row"<<j+1<<"("<<t<<" downto "<<t-1<<")<=patternI_"<<t-1<<"_"<<j<<"_A(3 downto 2);"<<endl;
            outputp<<" "<<endl;
        }

    if(j==15 and t!=41  )
      {  outputp<<"Minus_row"<<j<<"("<<t-1<<")<=patternI_"<<t-1<<"_"<<j<<"_A(0) or patternI_"<<t-3<<"_"<<j<<"_B(0) or patternI_";
         outputp<<t-1<<"_"<<j-2<<"_D(0) or patternI_"<<t-3<<"_"<<j-2<<"_E(0) or patternI_"<<t-1<<"_"<<j-4<<"_G(0) or patternI_"<<t-3<<"_"<<j-4<<"_H(0);"<<endl;
         i=i++;
         outputp<<"Minus_row"<<j<<"("<<t<<")<=patternI_"<<t-1<<"_"<<j<<"_A(1) or patternI_"<<t-3<<"_"<<j<<"_B(1) or patternI_";
         outputp<<t-1<<"_"<<j-2<<"_D(1) or patternI_"<<t-3<<"_"<<j-2<<"_E(1) or patternI_"<<t-1<<"_"<<j-4<<"_G(1) or patternI_"<<t-3<<"_"<<j-4<<"_H(1);"<<endl;
         i=i++;
         outputp<<"Minus_row"<<j+1<<"("<<t-1<<")<=patternI_"<<t-1<<"_"<<j<<"_A(2) or patternI_"<<t-3<<"_"<<j<<"_B(2) or patternI_";
         outputp<<t-1<<"_"<<j-2<<"_D(2) or patternI_"<<t-3<<"_"<<j-2<<"_E(2) or patternI_"<<t-1<<"_"<<j-4<<"_G(2) or patternI_"<<t-3<<"_"<<j-4<<"_H(2);"<<endl;
         i=i++;
         outputp<<"Minus_row"<<j+1<<"("<<t<<")<=patternI_"<<t-1<<"_"<<j<<"_A(3) or patternI_"<<t-3<<"_"<<j<<"_B(3) or patternI_";
         outputp<<t-1<<"_"<<j-2<<"_D(3) or patternI_"<<t-3<<"_"<<j-2<<"_E(3) or patternI_"<<t-1<<"_"<<j-4<<"_G(3) or patternI_"<<t-3<<"_"<<j-4<<"_H(3);"<<endl;
         outputp<<" "<<endl;
      }

     if(j==1 and t!=41  )
     {
         outputp<<"Minus_row"<<j<<"("<<t-1<<")<=patternI_"<<t-1<<"_"<<j<<"_A(0) or patternI_"<<t-3<<"_"<<j<<"_B(0);"<<endl;
         i=i++;
         outputp<<"Minus_row"<<j<<"("<<t<<")<=patternI_"<<t-1<<"_"<<j<<"_A(1) or patternI_"<<t-3<<"_"<<j<<"_B(1);"<<endl;
         i=i++;
         outputp<<"Minus_row"<<j+1<<"("<<t-1<<")<=patternI_"<<t-1<<"_"<<j<<"_A(2) or patternI_"<<t-3<<"_"<<j<<"_B(2);"<<endl;
         i=i++;
         outputp<<"Minus_row"<<j+1<<"("<<t<<")<=patternI_"<<t-1<<"_"<<j<<"_A(3) or patternI_"<<t-3<<"_"<<j<<"_B(3);"<<endl;
         outputp<<" "<<endl;

     }
}

if (j>=3 and j<=13)
{
     if( t==41 and j==3)
     {
         outputp<<"Minus_row"<<j<<"("<<t-1<<")<=patternI_"<<t-1<<"_"<<j<<"_A(0) or patternI_"<<t-1<<"_"<<j-2<<"_D(0);"<<endl;
         i=i++;
         outputp<<"Minus_row"<<j<<"("<<t<<")<=patternI_"<<t-1<<"_"<<j<<"_A(1) or patternI_"<<t-1<<"_"<<j-2<<"_D(1);"<<endl;
         i=i++;
         outputp<<"Minus_row"<<j+1<<"("<<t-1<<")<=patternI_"<<t-1<<"_"<<j<<"_A(2) or patternI_"<<t-1<<"_"<<j-2<<"_D(2);"<<endl;
         i=i++;
         outputp<<"Minus_row"<<j+1<<"("<<t<<")<=patternI_"<<t-1<<"_"<<j<<"_A(3) or patternI_"<<t-1<<"_"<<j-2<<"_D(3);"<<endl;
     }

     if( t==41 and j!=3 )
        {
         outputp<<"Minus_row"<<j<<"("<<t-1<<")<=patternI_"<<t-1<<"_"<<j<<"_A(0) or patternI_"<<t-1<<"_"<<j-2<<"_D(0) or patternI_"<<t-1<<"_"<<j-4<<"_G(0);"<<endl;
         i=i++;
         outputp<<"Minus_row"<<j<<"("<<t<<")<=patternI_"<<t-1<<"_"<<j<<"_A(1) or patternI_"<<t-1<<"_"<<j-2<<"_D(1) or patternI_"<<t-1<<"_"<<j-4<<"_G(1);"<<endl;
         i=i++;
         outputp<<"Minus_row"<<j+1<<"("<<t-1<<")<=patternI_"<<t-1<<"_"<<j<<"_A(2) or patternI_"<<t-1<<"_"<<j-2<<"_D(2) or patternI_"<<t-1<<"_"<<j-4<<"_G(2);"<<endl;
         i=i++;
         outputp<<"Minus_row"<<j+1<<"("<<t<<")<=patternI_"<<t-1<<"_"<<j<<"_A(3) or patternI_"<<t-1<<"_"<<j-2<<"_D(3) or patternI_"<<t-1<<"_"<<j-4<<"_G(3);"<<endl;
         outputp<<" "<<endl;
        }

       if( t!=41 and j==3  )
        {
         outputp<<"Minus_row"<<j<<"("<<t-1<<")<=patternI_"<<t-1<<"_"<<j<<"_A(0) or patternI_"<<t-3<<"_"<<j<<"_B(0) or patternI_"<<t-1<<"_"<<j-2<<"_D(0) or patternI_"<<t-3<<"_"<<j-2<<"_E(0);"<<endl;
         i=i++;
         outputp<<"Minus_row"<<j<<"("<<t<<")<=patternI_"<<t-1<<"_"<<j<<"_A(1) or patternI_"<<t-3<<"_"<<j<<"_B(1) or patternI_"<<t-1<<"_"<<j-2<<"_D(1) or patternI_"<<t-3<<"_"<<j-2<<"_E(1);"<<endl;
         i=i++;
         outputp<<"Minus_row"<<j+1<<"("<<t-1<<")<=patternI_"<<t-1<<"_"<<j<<"_A(2) or patternI_"<<t-3<<"_"<<j<<"_B(2) or patternI_"<<t-1<<"_"<<j-2<<"_D(2) or patternI_"<<t-3<<"_"<<j-2<<"_E(2);"<<endl;
         i=i++;
         outputp<<"Minus_row"<<j+1<<"("<<t<<")<=patternI_"<<t-1<<"_"<<j<<"_A(3) or patternI_"<<t-3<<"_"<<j<<"_B(3) or patternI_"<<t-1<<"_"<<j-2<<"_D(3) or patternI_"<<t-3<<"_"<<j-2<<"_E(3);"<<endl;
         outputp<<" "<<endl;
        }

        if( t!=41 and j!=3  )
        {  outputp<<"Minus_row"<<j<<"("<<t-1<<")<=patternI_"<<t-1<<"_"<<j<<"_A(0) or patternI_"<<t-3<<"_"<<j<<"_B(0) or patternI_"<<t-1<<"_"<<j-2<<"_D(0) or patternI_"<<t-3<<"_"<<j-2<<"_E(0) or patternI_"<<t-1<<"_"<<j-4<<"_G(0) or patternI_"<<t-3<<"_"<<j-4<<"_H(0);"<<endl;
         i=i++;
         outputp<<"Minus_row"<<j<<"("<<t<<")<=patternI_"<<t-1<<"_"<<j<<"_A(1) or patternI_"<<t-3<<"_"<<j<<"_B(1) or patternI_"<<t-1<<"_"<<j-2<<"_D(1) or patternI_"<<t-3<<"_"<<j-2<<"_E(1) or patternI_"<<t-1<<"_"<<j-4<<"_G(1) or patternI_"<<t-3<<"_"<<j-4<<"_H(1);"<<endl;
         i=i++;
         outputp<<"Minus_row"<<j+1<<"("<<t-1<<")<=patternI_"<<t-1<<"_"<<j<<"_A(2) or patternI_"<<t-3<<"_"<<j<<"_B(2) or patternI_"<<t-1<<"_"<<j-2<<"_D(2) or patternI_"<<t-3<<"_"<<j-2<<"_E(2) or patternI_"<<t-1<<"_"<<j-4<<"_G(2) or patternI_"<<t-3<<"_"<<j-4<<"_H(2);"<<endl;
         i=i++;
         outputp<<"Minus_row"<<j+1<<"("<<t<<")<=patternI_"<<t-1<<"_"<<j<<"_A(3) or patternI_"<<t-3<<"_"<<j<<"_B(3) or patternI_"<<t-1<<"_"<<j-2<<"_D(3) or patternI_"<<t-3<<"_"<<j-2<<"_E(3) or patternI_"<<t-1<<"_"<<j-4<<"_G(3) or patternI_"<<t-3<<"_"<<j-4<<"_H(3);"<<endl;
         outputp<<" "<<endl;
        }

}
           }}


  return 0;
}
