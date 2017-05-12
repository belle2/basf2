
#include <tracking/modules/hitToTrueXP/hitToTrueXPModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <TFile.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/hitToTrueXPDerivate.h>
#include <tracking/dataobjects/hitToTrueXP.h>
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
// #include <TRandom3.h>
// #include <TMinuit.h>
#include "TH1.h"
#include "TF1.h"
#include "TH2.h"
#include "TF2.h"
// #include "TGraphErrors.h"
// #include "TStyle.h"
#include "TMath.h"
// #include "TMatrixDSym.h"
// #include "TFitResult.h"
// #include "TLegend.h"
// #include "TColor.h"
// #include "TPaveText.h"
// #include "TPaveStats.h"
// #include "TGraphAsymmErrors.h"
// #include "TMacro.h"
// #include "THStack.h"
// #include "TLegendEntry.h"
// #include "TDatime.h"
// #include "TString.h"
// #include "TStyle.h"
#include "TLatex.h"
// #include "TRandom3.h"
// #include "TGraphPainter.h"
#include <algorithm>
#include <functional>

#pragma once


namespace Belle2 {

  class NoKickRTSel: public TObject {

    //  This class implement some methods useful for the application of cuts
    //  evaluated in NoKickCutsEval module. Use and auxiliary class (NoKickCuts)
    //  that cointains the cuts used in selection.

  public:
    std::vector<hitToTrueXP> m_hitToTrueXP;
    std::set<hitToTrueXP, hitToTrueXP::timeCompare> m_setHitToTrueXP;
    std::vector<hitToTrueXP> m_8hitTrack;
    NoKickCuts m_trackCuts;
    double m_pmax = 1.; //range analyzed with cuts

    enum parameters {
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
      m_hitToTrueXP.clear();
      m_setHitToTrueXP.clear();
      m_8hitTrack.clear();
    }

    // this method build a vector of hitToTrueXP from a track. The ouput is the
    // member of the class.
    void hitToTrueXPBuilder(const RecoTrack& track);

    // this metod build a vector of hitToTrueXP from a track selecting the first
    // hit on each layer of VXD (8 hit for SVD only, counting overlaps). The ouput
    // is the member of the class.
    void hit8TrackBuilder(const RecoTrack& track);


    //  This method return true if every segment (see segmentSelector) of the
    //  input track respects the cuts contraints.
    bool trackSelector(const RecoTrack& track);

    // This method return true if a couple of hits resects the cuts constraints.
    bool segmentSelector(hitToTrueXP hit1, hitToTrueXP hit2, std::vector<double> selCut, parameters par, bool is0 = false);

    // This method make some global cuts on the tracks (layer 3 and 6 required, d0 and z0 inside beam pipe).
    //Return false if this filter fails.
    bool globalCut(const std::vector<hitToTrueXP>& track8);

    ClassDef(NoKickRTSel, 1);
  };

} //end namespace Belle2
