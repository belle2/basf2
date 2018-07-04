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
  //char Name[500];

  TString odir[] = { "ph1sustr" , "microtpc" , "he3tube" };
  TString ifn[] = { "tmp_PH1SUSTR.xml" , "tmp_MICROTPC.xml" , "tmp_HE3TUBE.xml" };
  TString ofn[] = { "PH1SUSTR.xml" , "MICROTPC-phase1_bak.xml" , "HE3TUBE-phase1_bak.xml" };
  
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
  /*
  double PlatePos[4][3] = 
    {
      {60.,0,40.},
      {-60.,0,40.},
      {0,45.,40.},
      {0,-45.,40.}
    };
  */
  /*MC campaign
  double PlatePos[4][3] = 
    {
      {60.,0,-40.},
      {-60.,0,-40.},
      {0,45.,-40.},
      {0,-45.,-40.}
    };
  */
  /*
  double PlatePos[4][3] = 
    {
      {45.,0,30.},
      {-45.,0,30.},
      {0,45.,30.},
      {0,-45.,30.}
    };
  */
  /*GOOD
    double PlatePos[4][3] = 
    {
      //{51.8,0.8,0.4},
      {51.8,0.8,40.0},
      {0.1,52.1,37.9},
      {2.1,-50.5,42.3}
      {-53.2,-0.3,37.7},
    };
  */
  /*
     Right (+x):

     Distance: 591.08mm  [23.271in ]
     Delta X: 468.91mm  [18.461in ]
     Delta Y: 7.52mm  [0.296in ]
     Delta Z: 359.77mm  [14.164in ]
     
     
     Top (+y):
     
     Distance: 644.85mm  [25.388in ]
     Delta X: 29.57mm  [1.164in ]
     Delta Y: 520.89mm  [20.507in ]
     Delta Z: 379.00mm  [14.921in ]
     
     
     Left (-x):
     
     Distance: 655.47mm  [25.806in ]
     Delta X: 537.05mm  [21.144in ]
     Delta Y: 3.40mm  [0.134in ]
     Delta Z: 375.77mm  [14.794in ]
     
     Bottom (-y):
     
     Distance: 658.73mm  [25.934in ]
     Delta X: 9.53mm  [0.375in ]
     Delta Y: 504.89mm  [19.877in ]
     Delta Z: 423.00mm  [16.653in ] 
  */
  /**** Last 
  double PlatePos[4][3] = 
    {
      {46.891,0.752,35.977},
      {2.957,52.089,37.9},
      {-53.705,0.34,37.577},
      {0.953,-50.489,42.3}
    };
  ****/
  double PlatePos[4][3] = 
    {
      {49.2,-0.7,36.0},//
      {-3.0,52.1,37.9},//
      {-53.2,-0.3,37.7},//
      {-1.0,-50.5,42.3}//
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
  double dx_tpc[] = {12.7, 15.0, 15.0, 12.7};
  double dy_tpc[] = {10.16, 10.4, 10.4, 10.16};
  double tub_rad = 2.54;
  //double plate_l = 50.0;
  double plate_w = 32.0;//27.47788;
  double plate_hw = 0.5;//2.54 * 0.35;
  double TPCpos[4][3];
  double Tubpos[4][3];
  double tpc_offset = 1.5;
  double he3_offset = 5;
  for(int i=0;i<4;i++)
    {
      cout << "plate # " << i;
      if (i == 0) {	
	TPCpos[i][0] = PlatePos[i][0] - plate_hw / 2. - dy_tpc[i] / 2.;
	Tubpos[i][0] = PlatePos[i][0] - plate_hw / 2. - tub_rad;
	TPCpos[i][1] = PlatePos[i][1] - plate_w / 2. + dx_tpc[i] / 2. + tpc_offset;
	Tubpos[i][1] = PlatePos[i][1] + plate_w / 2. - tub_rad - he3_offset;
      }
      if (i == 1) {	
	TPCpos[i][0] = PlatePos[i][0] + plate_w / 2. - dx_tpc[i] / 2. - tpc_offset;
	Tubpos[i][0] = PlatePos[i][0] - plate_w / 2. + tub_rad + he3_offset;
	TPCpos[i][1] = PlatePos[i][1] - plate_hw / 2. - dy_tpc[i] / 2.;
	Tubpos[i][1] = PlatePos[i][1] - plate_hw / 2. - tub_rad;
      }
      if ( i == 2) {
	TPCpos[i][0] = PlatePos[i][0] + plate_hw / 2. + dy_tpc[i] / 2.;
	Tubpos[i][0] = PlatePos[i][0] + plate_hw / 2. + tub_rad;
	TPCpos[i][1] = PlatePos[i][1] + plate_w / 2. - dx_tpc[i] / 2. - tpc_offset;
	Tubpos[i][1] = PlatePos[i][1] - plate_w / 2. + tub_rad + he3_offset;
      }
      if (i == 3) {	
	TPCpos[i][0] = PlatePos[i][0] - plate_w / 2. + dx_tpc[i] / 2. + tpc_offset;
	Tubpos[i][0] = PlatePos[i][0] + plate_w / 2. - tub_rad - he3_offset;
	TPCpos[i][1] = PlatePos[i][1] + plate_hw / 2. + dy_tpc[i] / 2.;
	Tubpos[i][1] = PlatePos[i][1] + plate_hw / 2. + tub_rad;
      }
      TPCpos[i][2]=PlatePos[i][2];
      Tubpos[i][2]=PlatePos[i][2];
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
      Action = "cp ../"+odir[i]+"/data/"+ofn[i]+" ../"+odir[i]+"/data/"+ofn[i]+"_backup";
      cout << Action << endl;
      system(Action);
      Action = "cp "+ofn[i]+" ../"+odir[i]+"/data/.";
      cout << Action << endl;
      system(Action);
    }

}
