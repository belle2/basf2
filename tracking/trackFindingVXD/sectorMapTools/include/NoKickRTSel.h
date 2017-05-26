/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Valerio Bertacchi                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/hitXP/hitXPModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <TFile.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/hitXPDerivate.h>
#include <tracking/dataobjects/hitXP.h>
#include <TObject.h>
#include <tracking/trackFindingVXD/sectorMapTools/NoKickCuts.h>
#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
#include "TFile.h"
#include <TCanvas.h>
#include <iostream>
#include <fstream>
#include <string>
#include "TH1.h"
#include "TF1.h"
#include "TH2.h"
#include "TF2.h"
#include "TMath.h"
#include "TLatex.h"
#include <algorithm>
#include <functional>

#pragma once


namespace Belle2 {

  class NoKickRTSel: public TObject {

    //  This class implement some methods useful for the application of cuts
    //  evaluated in NoKickCutsEval module. Use and auxiliary class (NoKickCuts)
    //  that cointains the cuts used in selection.

  public:
    std::vector<hitXP> m_hitXP; //vector of hit, to convert the track
    std::set<hitXP, hitXP::timeCompare> m_setHitXP; //set of hit to order the hit in time
    std::vector<hitXP> m_8hitTrack; //vector of selected hit
    NoKickCuts m_trackCuts; //auxiliary member to apply the cuts
    double m_pmax = 2.; //range analyzed with cuts

    enum Eparameters { //name of the track parameters
      omega,
      d0,
      phi0,
      z0,
      tanlambda
    };

    //Constructor with input file for use specific cuts file
    NoKickRTSel(std::string fileName) :
      m_trackCuts(fileName)
    {
      initNoKickRTSel();
    }

    //Empty Constructor that uses the defaults cuts file.
    NoKickRTSel() :
      m_trackCuts()
    {
      initNoKickRTSel();
    }

    //Inizialize the class cleaning the member vectors
    void initNoKickRTSel()
    {
      m_hitXP.clear();
      m_setHitXP.clear();
      m_8hitTrack.clear();
    }

    // this method build a vector of hitXP from a track. The ouput is the
    // member of the class.
    //input (one reconstructed track)
    void hitXPBuilder(const RecoTrack& track);

    // this metod build a vector of hitXP from a track selecting the first
    // hit on each layer of VXD (8 hit for SVD only, counting overlaps). The ouput
    // is the member of the class.
    //input (one reconstructed track)
    void hit8TrackBuilder(const RecoTrack& track);


    //  This method return true if every segment (see segmentSelector) of the
    //  input track respects the cuts contraints.
    //input (one reconstructed track)
    bool trackSelector(const RecoTrack& track);

    // This method return true if a couple of hits resects the cuts constraints.
    //input (first hit, second hit, selected cut to apply, track parameter, it is first hit the IP?)
    bool segmentSelector(hitXP hit1, hitXP hit2, std::vector<double> selCut, Eparameters par, bool is0 = false);

    // This method make some global cuts on the tracks (layer 3 and 6 required, d0 and z0 inside beam pipe).
    //Return false if this filter fails.
    //input (the selected hit of the track)
    bool globalCut(const std::vector<hitXP>& track8);

    ClassDef(NoKickRTSel, 1);
  };

} //end namespace Belle2
