/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Matthew Barrett                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vector>
#include "TObject.h"

#ifndef TOPEVENT_HH
#define TOPEVENT_HH

//Use exactly one of the following two lines based on the input ROOT format being used
#define USEFALL2015ROOTFORMAT
//#define USETOPCAFROOTFORMAT

#define MAXHITS 400

struct topEvent {

#ifdef USEFALL2015ROOTFORMAT
  Int_t   nHit;
  Int_t   runNumber;
  Int_t   eventNumber;
  Short_t pixel[MAXHITS];
  Short_t pmt[MAXHITS];
  Float_t time[MAXHITS];
  Float_t tdc[MAXHITS];
  Int_t   charge[MAXHITS];
  Int_t   adc[MAXHITS]; 
#endif


#ifdef USETOPCAFROOTFORMAT
  Int_t   nHits;
  Int_t   runNumber;
  Int_t   eventNumber;
  vector<unsigned int> pixel;
  vector<unsigned int> pmt;
  vector<double>       time;
  vector<unsigned int> tdc;
  vector<double>       charge;
  vector<unsigned int> adc; 
#endif

} ;


#endif
