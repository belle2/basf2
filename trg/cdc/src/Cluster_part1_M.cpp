#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdio>
#include <vector>
#include <math.h>
#include <iomanip>
using namespace std;
int main()


{   int i,j,t,k ,p,u,z;
  ofstream outputp("Cluster_part1_M.txt",fstream::app);


  //計算數
  i=1;

//層
   j=1;
   p=j+16;

  for(j;j<p;j+=2)
  {
     //第幾塊HP
     z=41;
     t=z%160;
     u=t+40;


//cout<<"j="<<j<<endl;

        for(t;t<u;t+=2)
           {


  i=i++;
//cout<<"j="<<j<<","<<"t="<<t<<endl;

if (u-t==40)
{
    if(j-1<=0 )
{

  outputp<<"Center_"<<i<<" :cluster_center"<<endl;
  outputp<<"port map ("<<endl;

  outputp<<setw(21)<<"X =>\"0000\","<<endl;
  outputp<<setw(21)<<"Y =>\"0000\","<<endl;
  outputp<<setw(21)<<"Z =>\"0000\","<<endl;
  outputp<<setw(26)<<"A(1 downto 0)=>row"<<j<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"A(3 downto 2)=>row"<<j+1<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"B(1 downto 0)=>row"<<j<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;
  outputp<<setw(26)<<"B(3 downto 2)=>row"<<j+1<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;
  outputp<<setw(26)<<"D(1 downto 0)=>row"<<j+2<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"D(3 downto 2)=>row"<<j+3<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"E(1 downto 0)=>row"<<j+2<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;
  outputp<<setw(26)<<"E(3 downto 2)=>row"<<j+3<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;
  outputp<<setw(26)<<"G(1 downto 0)=>row"<<j+4<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"G(3 downto 2)=>row"<<j+5<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"H(1 downto 0)=>row"<<j+4<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;
  outputp<<setw(26)<<"H(3 downto 2)=>row"<<j+5<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;

  outputp<<setw(43)<<"new_cluster_A(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_A(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_B(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_B(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_D(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_D(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_E(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_E(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_G(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_G(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_H(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_H(3 downto 0),"<<endl;
  outputp<<setw(40)<<"top_clkData_s =>top_clkData_s "<<endl;
  outputp<<setw(10)<<");"<<endl;
  outputp<<" "<<endl;
}


if(j-1>0 and j<= 11) {
  outputp<<"Center_"<<i<<" :cluster_center"<<endl;
  outputp<<"port map ("<<endl;

  outputp<<setw(21)<<"X =>\"0000\","<<endl;
  outputp<<setw(21)<<"Y =>\"0000\","<<endl;
  outputp<<setw(26)<<"Z(1 downto 0)=>row"<<j-2<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"Z(3 downto 2)=>row"<<j-1<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"A(1 downto 0)=>row"<<j<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"A(3 downto 2)=>row"<<j+1<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"B(1 downto 0)=>row"<<j<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;
  outputp<<setw(26)<<"B(3 downto 2)=>row"<<j+1<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;
  outputp<<setw(26)<<"D(1 downto 0)=>row"<<j+2<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"D(3 downto 2)=>row"<<j+3<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"E(1 downto 0)=>row"<<j+2<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;
  outputp<<setw(26)<<"E(3 downto 2)=>row"<<j+3<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;
  outputp<<setw(26)<<"G(1 downto 0)=>row"<<j+4<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"G(3 downto 2)=>row"<<j+5<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"H(1 downto 0)=>row"<<j+4<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;
  outputp<<setw(26)<<"H(3 downto 2)=>row"<<j+5<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;

  outputp<<setw(43)<<"new_cluster_A(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_A(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_B(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_B(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_D(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_D(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_E(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_E(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_G(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_G(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_H(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_H(3 downto 0),"<<endl;
  outputp<<setw(40)<<"top_clkData_s =>top_clkData_s "<<endl;
  outputp<<setw(10)<<");"<<endl;
  outputp<<" "<<endl;
      }

 if( j<=14 and j>=13) {
  outputp<<"Center_"<<i<<" :cluster_center"<<endl;
  outputp<<"port map ("<<endl;

  outputp<<setw(21)<<"X =>\"0000\","<<endl;
  outputp<<setw(21)<<"Y =>\"0000\","<<endl;
  outputp<<setw(26)<<"Z(1 downto 0)=>row"<<j-2<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"Z(3 downto 2)=>row"<<j-1<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"A(1 downto 0)=>row"<<j<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"A(3 downto 2)=>row"<<j+1<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"B(1 downto 0)=>row"<<j<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;
  outputp<<setw(26)<<"B(3 downto 2)=>row"<<j+1<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;
  outputp<<setw(26)<<"D(1 downto 0)=>row"<<j+2<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"D(3 downto 2)=>row"<<j+3<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"E(1 downto 0)=>row"<<j+2<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;
  outputp<<setw(26)<<"E(3 downto 2)=>row"<<j+3<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;
  outputp<<setw(21)<<"G =>\"0000\","<<endl;
  outputp<<setw(21)<<"H =>\"0000\","<<endl;

  outputp<<setw(43)<<"new_cluster_A(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_A(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_B(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_B(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_D(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_D(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_E(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_E(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_G(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_G(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_H(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_H(3 downto 0),"<<endl;
  outputp<<setw(40)<<"top_clkData_s =>top_clkData_s "<<endl;
  outputp<<setw(10)<<");"<<endl;
  outputp<<" "<<endl;
      }

if( j<=16 and j>=15) {
  outputp<<"Center_"<<i<<" :cluster_center"<<endl;
  outputp<<"port map ("<<endl;

  outputp<<setw(21)<<"X =>\"0000\","<<endl;
  outputp<<setw(21)<<"Y =>\"0000\","<<endl;
  outputp<<setw(26)<<"Z(1 downto 0)=>row"<<j-2<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"Z(3 downto 2)=>row"<<j-1<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"A(1 downto 0)=>row"<<j<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"A(3 downto 2)=>row"<<j+1<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"B(1 downto 0)=>row"<<j<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;
  outputp<<setw(26)<<"B(3 downto 2)=>row"<<j+1<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;
  outputp<<setw(21)<<"D =>\"0000\","<<endl;
  outputp<<setw(21)<<"E =>\"0000\","<<endl;
  outputp<<setw(21)<<"G =>\"0000\","<<endl;
  outputp<<setw(21)<<"H =>\"0000\","<<endl;

  outputp<<setw(43)<<"new_cluster_A(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_A(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_B(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_B(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_D(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_D(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_E(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_E(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_G(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_G(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_H(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_H(3 downto 0),"<<endl;
  outputp<<setw(40)<<"top_clkData_s =>top_clkData_s "<<endl;
  outputp<<setw(10)<<");"<<endl;
  outputp<<" "<<endl;
      }
}

if(u-t==2)
{

if(j-1<=0 )
{

  outputp<<"Center_"<<i<<" :cluster_center"<<endl;
  outputp<<"port map ("<<endl;

  outputp<<setw(26)<<"X(1 downto 0)=>row"<<j<<"("<<t-2<<" downto "<<t-3<<"),"<<endl;
  outputp<<setw(26)<<"X(3 downto 2)=>row"<<j+1<<"("<<t-2<<" downto "<<t-3<<"),"<<endl;
  outputp<<setw(21)<<"Y =>\"0000\","<<endl;
  outputp<<setw(21)<<"Z =>\"0000\","<<endl;
  outputp<<setw(26)<<"A(1 downto 0)=>row"<<j<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"A(3 downto 2)=>row"<<j+1<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(21)<<"B =>\"0000\","<<endl;
  outputp<<setw(26)<<"D(1 downto 0)=>row"<<j+2<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"D(3 downto 2)=>row"<<j+3<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(21)<<"E =>\"0000\","<<endl;
  outputp<<setw(26)<<"G(1 downto 0)=>row"<<j+4<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"G(3 downto 2)=>row"<<j+5<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(21)<<"H =>\"0000\","<<endl;

  outputp<<setw(43)<<"new_cluster_A(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_A(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_B(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_B(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_D(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_D(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_E(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_E(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_G(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_G(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_H(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_H(3 downto 0),"<<endl;
  outputp<<setw(40)<<"top_clkData_s =>top_clkData_s "<<endl;
  outputp<<setw(10)<<");"<<endl;
  outputp<<" "<<endl;
}


if(j-1>0 and j<= 11) {
  outputp<<"Center_"<<i<<" :cluster_center"<<endl;
  outputp<<"port map ("<<endl;

  outputp<<setw(26)<<"X(1 downto 0)=>row"<<j<<"("<<t-2<<" downto "<<t-3<<"),"<<endl;
  outputp<<setw(26)<<"X(3 downto 2)=>row"<<j+1<<"("<<t-2<<" downto "<<t-3<<"),"<<endl;
  outputp<<setw(26)<<"Y(1 downto 0)=>row"<<j-2<<"("<<t-2<<" downto "<<t-3<<"),"<<endl;
  outputp<<setw(26)<<"Y(3 downto 2)=>row"<<j-1<<"("<<t-2<<" downto "<<t-3<<"),"<<endl;
  outputp<<setw(26)<<"Z(1 downto 0)=>row"<<j-2<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"Z(3 downto 2)=>row"<<j-1<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"A(1 downto 0)=>row"<<j<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"A(3 downto 2)=>row"<<j+1<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(21)<<"B =>\"0000\","<<endl;
  outputp<<setw(26)<<"D(1 downto 0)=>row"<<j+2<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"D(3 downto 2)=>row"<<j+3<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(21)<<"E =>\"0000\","<<endl;
  outputp<<setw(26)<<"G(1 downto 0)=>row"<<j+4<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"G(3 downto 2)=>row"<<j+5<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(21)<<"H =>\"0000\","<<endl;

  outputp<<setw(43)<<"new_cluster_A(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_A(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_B(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_B(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_D(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_D(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_E(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_E(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_G(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_G(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_H(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_H(3 downto 0),"<<endl;
  outputp<<setw(40)<<"top_clkData_s =>top_clkData_s "<<endl;
  outputp<<setw(10)<<");"<<endl;
  outputp<<" "<<endl;
      }

 if( j<=14 and j>=13) {
  outputp<<"Center_"<<i<<" :cluster_center"<<endl;
  outputp<<"port map ("<<endl;

  outputp<<setw(26)<<"X(1 downto 0)=>row"<<j<<"("<<t-2<<" downto "<<t-3<<"),"<<endl;
  outputp<<setw(26)<<"X(3 downto 2)=>row"<<j+1<<"("<<t-2<<" downto "<<t-3<<"),"<<endl;
  outputp<<setw(26)<<"Y(1 downto 0)=>row"<<j-2<<"("<<t-2<<" downto "<<t-3<<"),"<<endl;
  outputp<<setw(26)<<"Y(3 downto 2)=>row"<<j-1<<"("<<t-2<<" downto "<<t-3<<"),"<<endl;
  outputp<<setw(26)<<"Z(1 downto 0)=>row"<<j-2<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"Z(3 downto 2)=>row"<<j-1<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"A(1 downto 0)=>row"<<j<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"A(3 downto 2)=>row"<<j+1<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(21)<<"B =>\"0000\","<<endl;
  outputp<<setw(26)<<"D(1 downto 0)=>row"<<j+2<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"D(3 downto 2)=>row"<<j+3<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(21)<<"E =>\"0000\","<<endl;
  outputp<<setw(21)<<"G =>\"0000\","<<endl;
  outputp<<setw(21)<<"H =>\"0000\","<<endl;

  outputp<<setw(43)<<"new_cluster_A(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_A(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_B(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_B(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_D(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_D(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_E(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_E(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_G(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_G(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_H(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_H(3 downto 0),"<<endl;
  outputp<<setw(40)<<"top_clkData_s =>top_clkData_s "<<endl;
  outputp<<setw(10)<<");"<<endl;
  outputp<<" "<<endl;
      }

if( j<=16 and j>=15)
{
  outputp<<"Center_"<<i<<" :cluster_center"<<endl;
  outputp<<"port map ("<<endl;

  outputp<<setw(26)<<"X(1 downto 0)=>row"<<j<<"("<<t-2<<" downto "<<t-3<<"),"<<endl;
  outputp<<setw(26)<<"X(3 downto 2)=>row"<<j+1<<"("<<t-2<<" downto "<<t-3<<"),"<<endl;
  outputp<<setw(26)<<"Y(1 downto 0)=>row"<<j-2<<"("<<t-2<<" downto "<<t-3<<"),"<<endl;
  outputp<<setw(26)<<"Y(3 downto 2)=>row"<<j-1<<"("<<t-2<<" downto "<<t-3<<"),"<<endl;
  outputp<<setw(26)<<"Z(1 downto 0)=>row"<<j-2<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"Z(3 downto 2)=>row"<<j-1<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"A(1 downto 0)=>row"<<j<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"A(3 downto 2)=>row"<<j+1<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(21)<<"B =>\"0000\","<<endl;
  outputp<<setw(21)<<"D =>\"0000\","<<endl;
  outputp<<setw(21)<<"E =>\"0000\","<<endl;
  outputp<<setw(21)<<"G =>\"0000\","<<endl;
  outputp<<setw(21)<<"H =>\"0000\","<<endl;

  outputp<<setw(43)<<"new_cluster_A(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_A(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_B(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_B(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_D(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_D(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_E(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_E(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_G(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_G(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_H(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_H(3 downto 0),"<<endl;
  outputp<<setw(40)<<"top_clkData_s =>top_clkData_s "<<endl;
  outputp<<setw(10)<<");"<<endl;
  outputp<<" "<<endl;
      }
}

if (u-t!=40 and u-t!=2)
{

if(j-1<=0 )
{

  outputp<<"Center_"<<i<<" :cluster_center"<<endl;
  outputp<<"port map ("<<endl;

  outputp<<setw(26)<<"X(1 downto 0)=>row"<<j<<"("<<t-2<<" downto "<<t-3<<"),"<<endl;
  outputp<<setw(26)<<"X(3 downto 2)=>row"<<j+1<<"("<<t-2<<" downto "<<t-3<<"),"<<endl;
  outputp<<setw(21)<<"Y =>\"0000\","<<endl;
  outputp<<setw(21)<<"Z =>\"0000\","<<endl;
  outputp<<setw(26)<<"A(1 downto 0)=>row"<<j<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"A(3 downto 2)=>row"<<j+1<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"B(1 downto 0)=>row"<<j<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;
  outputp<<setw(26)<<"B(3 downto 2)=>row"<<j+1<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;
  outputp<<setw(26)<<"D(1 downto 0)=>row"<<j+2<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"D(3 downto 2)=>row"<<j+3<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"E(1 downto 0)=>row"<<j+2<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;
  outputp<<setw(26)<<"E(3 downto 2)=>row"<<j+3<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;
  outputp<<setw(26)<<"G(1 downto 0)=>row"<<j+4<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"G(3 downto 2)=>row"<<j+5<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"H(1 downto 0)=>row"<<j+4<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;
  outputp<<setw(26)<<"H(3 downto 2)=>row"<<j+5<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;

  outputp<<setw(43)<<"new_cluster_A(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_A(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_B(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_B(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_D(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_D(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_E(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_E(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_G(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_G(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_H(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_H(3 downto 0),"<<endl;
  outputp<<setw(40)<<"top_clkData_s =>top_clkData_s "<<endl;
  outputp<<setw(10)<<");"<<endl;
  outputp<<" "<<endl;
}


if(j-1>0 and j<= 11) {
  outputp<<"Center_"<<i<<" :cluster_center"<<endl;
  outputp<<"port map ("<<endl;

  outputp<<setw(26)<<"X(1 downto 0)=>row"<<j<<"("<<t-2<<" downto "<<t-3<<"),"<<endl;
  outputp<<setw(26)<<"X(3 downto 2)=>row"<<j+1<<"("<<t-2<<" downto "<<t-3<<"),"<<endl;
  outputp<<setw(26)<<"Y(1 downto 0)=>row"<<j-2<<"("<<t-2<<" downto "<<t-3<<"),"<<endl;
  outputp<<setw(26)<<"Y(3 downto 2)=>row"<<j-1<<"("<<t-2<<" downto "<<t-3<<"),"<<endl;
  outputp<<setw(26)<<"Z(1 downto 0)=>row"<<j-2<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"Z(3 downto 2)=>row"<<j-1<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"A(1 downto 0)=>row"<<j<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"A(3 downto 2)=>row"<<j+1<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"B(1 downto 0)=>row"<<j<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;
  outputp<<setw(26)<<"B(3 downto 2)=>row"<<j+1<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;
  outputp<<setw(26)<<"D(1 downto 0)=>row"<<j+2<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"D(3 downto 2)=>row"<<j+3<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"E(1 downto 0)=>row"<<j+2<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;
  outputp<<setw(26)<<"E(3 downto 2)=>row"<<j+3<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;
  outputp<<setw(26)<<"G(1 downto 0)=>row"<<j+4<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"G(3 downto 2)=>row"<<j+5<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"H(1 downto 0)=>row"<<j+4<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;
  outputp<<setw(26)<<"H(3 downto 2)=>row"<<j+5<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;

  outputp<<setw(43)<<"new_cluster_A(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_A(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_B(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_B(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_D(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_D(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_E(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_E(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_G(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_G(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_H(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_H(3 downto 0),"<<endl;
  outputp<<setw(40)<<"top_clkData_s =>top_clkData_s "<<endl;
  outputp<<setw(10)<<");"<<endl;
  outputp<<" "<<endl;
      }

 if( j<=14 and j>=13) {
  outputp<<"Center_"<<i<<" :cluster_center"<<endl;
  outputp<<"port map ("<<endl;

  outputp<<setw(26)<<"X(1 downto 0)=>row"<<j<<"("<<t-2<<" downto "<<t-3<<"),"<<endl;
  outputp<<setw(26)<<"X(3 downto 2)=>row"<<j+1<<"("<<t-2<<" downto "<<t-3<<"),"<<endl;
  outputp<<setw(26)<<"Y(1 downto 0)=>row"<<j-2<<"("<<t-2<<" downto "<<t-3<<"),"<<endl;
  outputp<<setw(26)<<"Y(3 downto 2)=>row"<<j-1<<"("<<t-2<<" downto "<<t-3<<"),"<<endl;
  outputp<<setw(26)<<"Z(1 downto 0)=>row"<<j-2<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"Z(3 downto 2)=>row"<<j-1<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"A(1 downto 0)=>row"<<j<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"A(3 downto 2)=>row"<<j+1<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"B(1 downto 0)=>row"<<j<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;
  outputp<<setw(26)<<"B(3 downto 2)=>row"<<j+1<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;
  outputp<<setw(26)<<"D(1 downto 0)=>row"<<j+2<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"D(3 downto 2)=>row"<<j+3<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"E(1 downto 0)=>row"<<j+2<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;
  outputp<<setw(26)<<"E(3 downto 2)=>row"<<j+3<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;
  outputp<<setw(21)<<"G =>\"0000\","<<endl;
  outputp<<setw(21)<<"H =>\"0000\","<<endl;

  outputp<<setw(43)<<"new_cluster_A(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_A(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_B(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_B(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_D(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_D(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_E(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_E(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_G(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_G(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_H(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_H(3 downto 0),"<<endl;
  outputp<<setw(40)<<"top_clkData_s =>top_clkData_s "<<endl;
  outputp<<setw(10)<<");"<<endl;
  outputp<<" "<<endl;
      }

if( j<=16 and j>=15) {
  outputp<<"Center_"<<i<<" :cluster_center"<<endl;
  outputp<<"port map ("<<endl;

  outputp<<setw(26)<<"X(1 downto 0)=>row"<<j<<"("<<t-2<<" downto "<<t-3<<"),"<<endl;
  outputp<<setw(26)<<"X(3 downto 2)=>row"<<j+1<<"("<<t-2<<" downto "<<t-3<<"),"<<endl;
  outputp<<setw(26)<<"Y(1 downto 0)=>row"<<j-2<<"("<<t-2<<" downto "<<t-3<<"),"<<endl;
  outputp<<setw(26)<<"Y(3 downto 2)=>row"<<j-1<<"("<<t-2<<" downto "<<t-3<<"),"<<endl;
  outputp<<setw(26)<<"Z(1 downto 0)=>row"<<j-2<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"Z(3 downto 2)=>row"<<j-1<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"A(1 downto 0)=>row"<<j<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"A(3 downto 2)=>row"<<j+1<<"("<<t<<" downto "<<t-1<<"),"<<endl;
  outputp<<setw(26)<<"B(1 downto 0)=>row"<<j<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;
  outputp<<setw(26)<<"B(3 downto 2)=>row"<<j+1<<"("<<t+2<<" downto "<<t+1<<"),"<<endl;
  outputp<<setw(21)<<"D =>\"0000\","<<endl;
  outputp<<setw(21)<<"E =>\"0000\","<<endl;
  outputp<<setw(21)<<"G =>\"0000\","<<endl;
  outputp<<setw(21)<<"H =>\"0000\","<<endl;

  outputp<<setw(43)<<"new_cluster_A(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_A(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_B(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_B(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_D(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_D(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_E(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_E(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_G(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_G(3 downto 0),"<<endl;
  outputp<<setw(43)<<"new_cluster_H(3 downto 0) =>patternI_"<<t-1<<"_"<<j<<"_H(3 downto 0),"<<endl;
  outputp<<setw(40)<<"top_clkData_s =>top_clkData_s "<<endl;
  outputp<<setw(10)<<");"<<endl;
  outputp<<" "<<endl;
      }
}
                        }

                }
  return 0;
}
