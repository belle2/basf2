/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SCANCDCGEOMODULE_H
#define SCANCDCGEOMODULE_H

#include <framework/core/Module.h>

#include <TFile.h>
#include <TTree.h>
#include <TMath.h>

namespace Belle2 {

  class ScanCDCGeoModule: public Module {

  public:

    ScanCDCGeoModule();

    /** constructor */
    virtual ~ScanCDCGeoModule() {};

    /** init class */
    virtual void initialize() override;;

    /** terminate class and save information */
    virtual void terminate() override;;

    /** Create the output TFiles and TTrees. */
    void bookOutput();

  protected:

    TFile* m_file{0x0}; /**< output ROOT files */
    TTree* m_tree{0x0}; /**< output ROOT trees */
    int fnWires{ -1}; /**< nwire per layer dist */
    double fstereoAng{ -1.0}; /**< stereo angle per layer dist */
    double fswire_posF_phi{ -1.0}; /**< sense wire position in forward dir (phi) per layer dist */
    double fswire_posF_theta{ -1.0}; /**< sense wire position in forward dir (theta) per layer dist */
    double fswire_posB_phi{ -1.0}; /**< sense wire position in backward dir (phi) per layer dist */
    double fswire_posB_theta{ -1.0}; /**< sense wire position in backward dir (theta) per layer dist */
    double fwire_iradius{ -1.0}; /**< field wire inner radius per layer dist */
    double fwire_oradius{ -1.0}; /**< field wire outer radius per layer dist */
    double fcwidth{ -1.0}; /**< cell width per layer dist */
    double fcheight{ -1.0}; /**< cell height per layer dist */
    double fclength{ -1.0}; /**< wire length per layer dist */
    double PI{TMath::Pi()}; /**< Pi value */
  };
} // Belle2 namespace
#endif