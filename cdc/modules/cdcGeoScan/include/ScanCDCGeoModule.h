/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef SCANCDCGEOMODULE_H
#define SCANCDCGEOMODULE_H

#include <framework/core/Module.h>

#include <TFile.h>
#include <TTree.h>
#include <TMath.h>

namespace Belle2 {

  /** Test module to save CDC geometry information to ntuple file */
  class ScanCDCGeoModule: public Module {

  public:

    /** constructor */
    ScanCDCGeoModule();

    /** destructor */
    virtual ~ScanCDCGeoModule() {};

    /** init  */
    virtual void initialize() override;;

    /** terminate and save information */
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