//--------------------------------------------------------------------------
//
// Environment:
//      This software is part of the EvtGen package developed jointly
//      for the BaBar and CLEO collaborations.  If you use all or part
//      of it, please give an appropriate acknowledgement.
//
// Copyright Information: See EvtGen/COPYRIGHT
//      Copyright (C) 1998      Caltech, UCSB
//
// Module: EvtGen/EvtCLEOhacked.hh
//
// Description: This model is based on matrix element method used by
//              CLEO in Phys.Rev.D76:072001,2007 to model the dipion mass
//              and helicity angle distribution in the decays Y(mS) -> pi pi Y(nS),
//              where m,n are integers and m>n and m<4.
//              This model has two parameters, Re(B/A) and Im(B/A), which
//              are coefficients of the matrix element's terms determined by
//              the CLEO fits.
//
// Example:
//
// Decay  Upsilon(3S)
//  1.0000    Upsilon      pi+  pi-     YMSTOYNSPIPICLEO -2.523 1.189;
// Enddecay
// Decay  Upsilon(3S)
//  1.0000    Upsilon(2S)  pi+  pi-     YMSTOYNSPIPICLEO -0.395 0.001;
// Enddecay
// Decay  Upsilon(2S)
//  1.0000    Upsilon      pi+  pi-     YMSTOYNSPIPICLEO -0.753 0.000;
// Enddecay
//
//   --> the order of parameters is: Re(B/A) Im(B/A)
//
// Modification history:
//
//    SEKULA  Jan. 28, 2008         Module created
//    FULSOM, Bryan  May 12 2015    Change boost, L178
//
//------------------------------------------------------------------------

#ifndef EVTYMSTOYNSPIPICLEOBOOST_HH
#define EVTYMSTOYNSPIPICLEOBOOST_HH

// #include "EvtGenBase/EvtDecayProb.hh"
#include "EvtGenBase/EvtDecayAmp.hh"

class EvtParticle;

//! Register Decay model EvtYmSToYnSpipiCLEOboost
class EvtYmSToYnSpipiCLEOboost: public  EvtDecayAmp  {
  //EvtDecayProb  {

public:
  /* Constructor. */
  EvtYmSToYnSpipiCLEOboost() {}

  /* Destructor. */
  virtual ~EvtYmSToYnSpipiCLEOboost();

  std::string getName(); /**< Get function Name  */
  EvtDecayBase* clone();  /**< Clone the decay  */

  void decay(EvtParticle* p); /**< Member of particle in EvtGen */
  void init();  /**< Initialize standard stream objects  */
  void initProbMax();  /**< Initialize standard stream objects for probability function  */

};

#endif

