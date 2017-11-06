/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Valerio Bertacchi                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackingPerformanceEvaluation/hitXPModule.h>
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

  /**  This class implement some methods useful for the application of cuts
  *  evaluated in NoKickCutsEval module. Use and auxiliary class (NoKickCuts)
  *  that cointains the cuts used in selection.
  */

  class NoKickRTSel: public TObject {

  public:
    std::vector<hitXP> m_hitXP; /**< vector of hit, to convert the track */
    std::set<hitXP, hitXP::timeCompare> m_setHitXP; /**< set of hit to order the hit in time */
    std::vector<hitXP> m_8hitTrack; /**< vector of selected hit */
    NoKickCuts m_trackCuts; /**< auxiliary member to apply the cuts */
    double m_pmax = 2.; /**< range analyzed with cuts */
    int m_numberOfCuts; /**< number of catastrophic interaction for each track */
    bool m_outputFlag; /**< true=produce validation output */

    TFile* m_noKickOutputTFile; /**< validartion output TFile */
    TH1F* m_momSel; /**< histogram of selected tracks */
    TH1F* m_momCut; /**< histrogram of cutted tracks */
    TH1F* m_momEff; /**< histogram for efficiency */
    TH1F* m_PDGIDCut; /**< histogram for PDGID of cutted track */
    TH1F* m_PDGIDSel; /**< histogram for PDGID of selected track */
    TH1F* m_PDGIDEff; /**< histogram for efficiency for each PDGID */
    TH1F* m_nCutHit; /**< histogram for number of cutted hist per track */

    /** Constructor with input file for use specific cuts file and allows validation */
    NoKickRTSel(std::string fileName, bool outputHisto) :
      m_trackCuts(fileName)
    {
      m_outputFlag = false;
      initNoKickRTSel();
      initHistoNoKick(outputHisto);
    }

    /** Empty Constructor that uses the defaults cuts file. */
    NoKickRTSel() :
      m_trackCuts()
    {
      initNoKickRTSel();
    }

    /** Inizialize the class cleaning the member vectors */
    void initNoKickRTSel()
    {
      m_hitXP.clear();
      m_setHitXP.clear();
      m_8hitTrack.clear();
      m_numberOfCuts = 0;
    }

    /**  this method build a vector of hitXP from a track. The ouput is the
    *  member of the class.
    * input (one reconstructed track)
    */
    void hitXPBuilder(const RecoTrack& track);

    /**  this metod build a vector of hitXP from a track selecting the first
    *  hit on each layer of VXD (8 hit for SVD only, counting overlaps). The ouput
    *  is the member of the class.
    * input (one reconstructed track)
    */
    void hit8TrackBuilder(const RecoTrack& track);


    /**   This method return true if every segment (see segmentSelector) of the
    **   input track respects the cuts contraints.
    ** input (one reconstructed track)
    */
    bool trackSelector(const RecoTrack& track);

    /**  This method return true if a couple of hits resects the cuts constraints.
    * input (first hit, second hit, selected cut to apply, track parameter, it is first hit the IP?)
    */
    bool segmentSelector(hitXP hit1, hitXP hit2, std::vector<double> selCut, NoKickCuts::EParameters par, bool is0 = false);

    /**  This method make some global cuts on the tracks (layer 3 and 6 required, d0 and z0 inside beam pipe).
    * Return false if this filter fails.
    * input (the selected hit of the track)
    */
    bool globalCut(const std::vector<hitXP>& track8);

    /** This metod initialize some validation histograms of the Training Sample Selection.
    * The input boolean allows the initialization, otherwise the method is empty (no validation)
    */
    void initHistoNoKick(bool outHisto)
    {
      if (outHisto) {
        m_noKickOutputTFile = new TFile("TrackSelection_NoKick.root", "RECREATE");
        m_momSel = new TH1F("m_momSel", "m_momSel", 100, 0, 4);
        m_momCut = new TH1F("m_momCut", "m_momCut", 100, 0, 4);
        m_momEff = new TH1F("m_momEff", "m_momEff", 100, 0, 4);

        m_PDGIDSel = new TH1F("m_PDGIDSel", "m_PDGIDSel", 6000, -3000, 3000);
        m_PDGIDCut = new TH1F("m_PDGIDCut", "m_PDGIDCut", 6000, -3000, 3000);
        m_PDGIDEff = new TH1F("m_PDGIDEff", "m_PDGIDEff", 6000, -3000, 3000);

        m_nCutHit = new TH1F("m_nCutHit", "m_nCutHit", 30, 0, 30);

        m_outputFlag = true;
      }

    }

    /** This method produce the validation histograms (to be used the endrun
    combined with the filling in trackSelector method)
    */
    void produceHistoNoKick();


    ClassDef(NoKickRTSel, 1);
  };

} /** end namespace Belle2 */
