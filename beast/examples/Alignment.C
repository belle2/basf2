/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <iostream>
#include <fstream>
using namespace std;
#include <TMath.h>
#include <TROOT.h>
#include <TSystemDirectory.h>
#include "Riostream.h"
#include "TSystem.h"

void Alignment()
{
  char Name[500];

  TString odir[] = { "ph1sustr" , "microtpc" , "he3tube" };
  TString ifn[] = { "tmp_PH1SUSTR.xml" , "tmp_MICROTPC.xml" , "tmp_HE3TUBE.xml" };
  TString ofn[] = { "PH1SUSTR.xml" , "MICROTPC.xml" , "HE3TUBE.xml" };
  
  //Set the Al plate position in centimeter
  /*
  double PlatePos[4][3] = 
    {
      {60.,0,12.},
      {-60.,0,12.},
      {0,45.,12.},
      {0,-45.,12.}
    };
  */
  
  double PlatePos[4][3] = 
    {
      {60.,0,40.},
      {-60.,0,40.},
      {0,45.,40.},
      {0,-45.,40.}
    };
  
  /*
  double PlatePos[4][3] = 
    {
      {60.,0,0.},
      {-60.,0,0.},
      {0,45.,0.},
      {0,-45.,0.}
    };
  */
  //Calculate TPC and Tube positions
  double dx_tpc = 12.7;
  double dy_tpc = 10.16;
  double tub_rad = 2.38;
  double TPCpos[4][3];
  double Tubpos[4][3];
  for(int i=0;i<4;i++)
    {
      cout << "plate # " << i;
      for(int j=0;j<3;j++)
	{
	  if(j==2)
	    {
	      TPCpos[i][j]=PlatePos[i][j];
	      Tubpos[i][j]=PlatePos[i][j];
	    }
	  if(i<2){
	    if(j==0 && PlatePos[i][j]>0)
	      {
		TPCpos[i][j]=PlatePos[i][j]-dy_tpc/2.;
		Tubpos[i][j]=PlatePos[i][j]-tub_rad;
	      }
	    else if(j==0 && PlatePos[i][j]<0)
	      {
		TPCpos[i][j]=PlatePos[i][j]+dy_tpc/2.;
		Tubpos[i][j]=PlatePos[i][j]+tub_rad;
	      }
	    else if(j==1){
	      TPCpos[i][j]=PlatePos[i][j]+dx_tpc/2.+2.;
	      Tubpos[i][j]=PlatePos[i][j]-tub_rad-2.;
	    }
	  }else{
	    if(j==1 && PlatePos[i][j]>0)
	      {
		TPCpos[i][j]=PlatePos[i][j]-dy_tpc/2.;
		Tubpos[i][j]=PlatePos[i][j]-tub_rad;
              }
            else if(j==1 && PlatePos[i][j]<0)
	      {
                TPCpos[i][j]=PlatePos[i][j]+dy_tpc/2.;
		Tubpos[i][j]=PlatePos[i][j]+tub_rad;
              }
            else if(j==0){
              TPCpos[i][j]=PlatePos[i][j]+dx_tpc/2.+2.;
	      Tubpos[i][j]=PlatePos[i][j]-tub_rad-2.;
            }
	  }
	  cout << " TPC " << TPCpos[i][j]; 
	  cout << " Tub " << Tubpos[i][j]; 
	}
      cout << endl;
    }
  
  //Write and move new xml files in ph1sustr, microtpc, and he3tube directories
  for(int i=0;i<3;i++)
    {
      
      ifstream in;
      ofstream out;
      TString Line;
      
      in.open(ifn[i]);
      out.open(ofn[i]);
      while( in.good () )
	{
	  Line = "";
	  Line.ReadLine(in);
	  for(int j=0;j<4;j++)
	    {
	      for(int k=0;k<3;k++)
		{
		  TString POS = "";
		  if(k==0)POS = TString::Format("POS_X%d",j+1);
		  if(k==1)POS = TString::Format("POS_Y%d",j+1);
		  if(k==2)POS = TString::Format("POS_Z%d",j+1);
		  double pos = 0;
		  if(i==0)pos = PlatePos[j][k];
		  if(i==1)pos = TPCpos[j][k];
		  if(i==2)pos = Tubpos[j][k];
		  if(Line.Contains(POS))Line.ReplaceAll(POS, TString::Format("%.5f",pos) );
		  //if(Line.Contains("POS_X1"))Line.ReplaceAll("POS_X1", TString::Format("%.5f",PlatePos[0][0]) );
		  //if(Line.Contains("POS_Y1"))Line.ReplaceAll("POS_Y1", TString::Format("%.5f",PlatePos[0][1]) );
		  //if(Line.Contains("POS_Z1"))Line.ReplaceAll("POS_Z1", TString::Format("%.5f",PlatePos[0][2]) );
		}
	    }
	  out<<Line;
	  out<<endl;
	}
      out.close();
      in.close();
      //sprintf(Name,"");
      TString Action = "fixstyle "+ofn[i];
      cout << Action << endl;
      system(Action);
      Action = "cp "+ofn[i]+" ../"+odir[i]+"/data/.";
      cout << Action << endl;
      system(Action);
    }
  
}
