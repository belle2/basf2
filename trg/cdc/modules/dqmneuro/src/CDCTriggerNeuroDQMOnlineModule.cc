/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/dataobjects/EventMetaData.h>

#include "TDirectory.h"
#include <tracking/dataobjects/RecoTrack.h>
#include <trg/cdc/modules/dqmneuro/CDCTriggerNeuroDQMOnlineModule.h>


using namespace Belle2;
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CDCTriggerNeuroDQMOnline);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCTriggerNeuroDQMOnlineModule::CDCTriggerNeuroDQMOnlineModule() : HistoModule()
{
  //Set module properties
  setDescription("CDC Trigger DQM Online module for HLT/Expressreco");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("unpacked2DTracksName", m_unpacked2DTracksName,
           "The name of the StoreArray of the unpacked 2D finder tracks",
           std::string("CDCTrigger2DFinderTracks"));
  addParam("simNeuroTracksName", m_simNeuroTracksName,
           "The name of the StoreArray of the Neurotrigger tracks from HWSIM",
           std::string("TSimNeuroTracks"));
  addParam("unpackedNeuroTracksName", m_unpackedNeuroTracksName,
           "The name of the StoreArray of the unpacked neurotrigger tracks",
           std::string("CDCTriggerNeuroTracks"));
  addParam("unpackedNeuroInput2dTracksName", m_unpackedNeuroInput2DTracksName,
           "The name of the StoreArray of the neurotrigger input 2d tracks",
           std::string("CDCTriggerNNInput2DFinderTracks"));
  addParam("unpackedNeuroInputSegmentHits", m_unpackedNeuroInputSegmentHitsName,
           "The name of the StoreArray of the neurotrigger input segment hits",
           std::string("CDCTriggerNNInputSegmentHits"));
  addParam("unpackedNeuroInputAllStereoSegmentHitsName", m_unpackedNeuroInputAllSegmentHitsName,
           "The name of the StoreArray of all segmenthits arriving at the NN boards",
           std::string("CDCTriggerNNInputAllStereoSegmentHits"));
  addParam("histogramDirectoryName", m_histogramDirectoryName,
           "Name of the directory where histograms will be placed",
           std::string("TRGCDCTNN2"));
  addParam("RecoTracksName", m_recoTracksName,
           "Name of the RecoTrack StoreArray. Leave empty for skipping them.",
           std::string("RecoTracks"));
  addParam("useSimTracks", m_useSimTracks,
           "switch on to use simulated tracks for plots, error logs and in debug output"
           "Useful for debugging SW/HW Errors:",
           false);
  addParam("useRecoTracks", m_useRecoTracks,
           "switch on to use recotracks",
           true);

}


CDCTriggerNeuroDQMOnlineModule::~CDCTriggerNeuroDQMOnlineModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void CDCTriggerNeuroDQMOnlineModule::defineHisto()
{
  // Create a separate histogram directory and cd into it.
  TDirectory* oldDir = gDirectory;
  if (m_histogramDirectoryName != "") {
    oldDir->mkdir(m_histogramDirectoryName.c_str());
    oldDir->cd(m_histogramDirectoryName.c_str());
  }
  //----------------------------------------------------------------

  // define neurotrigger histograms

  // histograms with recotracks (and hwsimtracks):

  m_neuroHWOutdzall = new TH1F("NeuroHWOutZ-RecoZ",
                               "z Resolution of unpacked and valid neuro tracks; z [cm]",
                               200, -100, 100);
  m_neuroHWOutdz0 = new TH1F("Q0NeuroHWOutZ-RecoZ",
                             "Quadrant0 z Resolution of unpacked and valid neuro tracks; z [cm]",
                             200, -100, 100);
  m_neuroHWOutdz1 = new TH1F("Q1NeuroHWOutZ-RecoZ",
                             "Quadrant1 z Resolution of unpacked and valid neuro tracks; z [cm]",
                             200, -100, 100);
  m_neuroHWOutdz2 = new TH1F("Q2NeuroHWOutZ-RecoZ",
                             "Quadrant2 z Resolution of unpacked and valid neuro tracks; z [cm]",
                             200, -100, 100);
  m_neuroHWOutdz3 = new TH1F("Q3NeuroHWOutZ-RecoZ",
                             "Quadrant3 z Resolution of unpacked and valid neuro tracks; z [cm]",
                             200, -100, 100);
  m_neuroHWSimRecodZ = new TH1F("NeuroHWSimZ-RecoZ",
                                "z Resolution of Simulated HW Tracks; z [cm]",
                                200, -100, 100);

  // histograms wih just hwneuro and hw2dtracks:

  m_neuroHWOutZ = new TH1F("NeuroHWOutZ",
                           "z distribution of unpacked and valid neuro tracks; z [cm]",
                           200, -100, 100);
  m_recoZ = new TH1F("RecoTrackZ",
                     "z distribution of all RecoTracks; z [cm]",
                     400, -200, 200);
  m_recoZ_related = new TH1F("RecoTrackZ_related",
                             "z distribution of all related Recotracks; z [cm]",
                             400, -200, 200);
  m_neuroHWOutSTTZ = new TH1F("NeuroHWOutSTTZ",
                              "z distribution of unpacked and valid first not updated per event Neurotracks  and p<0.7GeV; z [cm]",
                              200, -100, 100);
  m_neuroHWOutCosTheta = new TH1F("NeuroHWOutCosTheta",
                                  "cos theta distribution of unpacked and valid neuro tracks; cos(#theta) ",
                                  100, -1, 1);
  m_neuroHWOutPhi0 = new TH1F("NeuroHWOutPhi0",
                              "phi distribution of unpacked and valid neuro tracks; #phi [#circ]",
                              80, 0, 360); // shift to reduce the binning error
  m_neuroHWOutP = new TH1F("NeuroHWOutP",
                           "P distribution of unpacked neuro tracks; p_{T} [GeV]",
                           100, 0, 3);
  m_neuroHWOutPt = new TH1F("NeuroHWOutPt",
                            "Pt distribution of unpacked neuro tracks; p_{T} [GeV]",
                            10, 0, 3);
  m_neuroHWOutm_time = new TH1F("NeuroHWOutM_time", "m_time distribution of unpacked neuro tracks; clock cycle",
                                48, 0, 48);
  m_neuroHWValTracksNumber = new TH1F("NeuroHWValTracksNumber", "number of valid neurotracks per event; track count",
                                      20, 0, 20);
  m_neuroHWSector = new TH1F("NeuroHWExpert",
                             "Expert of unpacked neuro tracks; Expert",
                             5, 0, 5);
  m_neuroHWInTSID = new TH1F("NeuroHWInTSID", "ID of incoming track segments",
                             2336, 0, 2336);
  m_neuroHWInTSIDSel = new TH1F("NeuroHWInSelTSID", "ID of selected NNT track segments",
                                2336, 0, 2336);
  m_neuroHWInCDCFE = new TH1F("NeuroHWInCDCFE", "Number of incoming CDCFE board",
                              300, 0, 300);
  m_neuroHWInm_time = new TH1F("NeuroHWInM_time", "m_time distribution from incoming 2dtracks; clock cycle",
                               48, 0, 48);
  m_neuroHWIn2DTracksNumber = new TH1F("NeuroHWIn2DTracksNumber", "number of incoming 2dtracks per event; track count",
                                       20, 0, 20);

  // now the histograms with hwsim neurotracks:

  m_neuroHWOutHwSimdZ = new TH1F("NeuroHWOutZ-NeuroHWSimZ",
                                 "dz Distribution of Valid Neuro Tracks and Simulated HW Tracks; z [cm]",
                                 200, -100, 100);
  m_neuroHWSimZ = new TH1F("NeuroHWSimZ",
                           "z Distribution of Simulated HW Tracks; z [cm]",
                           200, -100, 100);
  m_neuroHWSimCosTheta = new TH1F("NeuroHWSimCosTheta",
                                  "cos theta Distribution of Simulated HW Tracks; cos(#theta) ",
                                  100, -1, 1);
  m_neuroErrors = new TH1F("Neurotrigger-Errors", "Errors in the Neuro Hardware", m_errcount, 0, m_errcount);
  //m_neuroErrors->SetDirectory(0);
  //m_neuroErrors->SetOption("bar");
  //m_neuroErrors->SetFillStyle(0);
  //m_neuroErrors->SetMinimum(0);
  //m_neuroErrors->SetStats(false);
  //m_neuroErrors->Draw("hist");
  //m_neuroErrors->SetDirectory(0);
  //m_neuroErrors->SetDirectory(0);

  //m_neuroErrorsRaw = new Th1F("Neurotrigger Errors");
  // cd back to root directory
  oldDir->cd();
}
void CDCTriggerNeuroDQMOnlineModule::initialize()
{
  // Register histograms (calls back defineHisto)

  REG_HISTOGRAM

  // require event time and hwneuro related arrays:

  m_eventTime.isOptional("CDCTriggerNeuroETFT0");
  m_unpacked2DTracks.isRequired(m_unpacked2DTracksName);

  m_unpackedNeuroTracks.isRequired(m_unpackedNeuroTracksName);
  m_unpackedNeuroInput2DTracks.isRequired(m_unpackedNeuroInput2DTracksName);
  m_unpackedNeuroInputSegmentHits.isRequired(m_unpackedNeuroInputSegmentHitsName);
  m_unpackedNeuroInputAllSegmentHits.isRequired(m_unpackedNeuroInputAllSegmentHitsName);
  m_unpackedNeuroInputVectorName = m_unpackedNeuroTracksName + "Input";
  m_unpackedNeuroInputVector.isRequired(m_unpackedNeuroInputVectorName);

  // require recotracks, if activated:

  if (m_useRecoTracks) {m_RecoTracks.isRequired("RecoTracks");}

  //require hwsimtracks, if activated:

  if (m_useSimTracks) {
    m_simNeuroTracks.isRequired(m_simNeuroTracksName);
    m_simNeuroInputVectorName = m_simNeuroTracksName + "Input";
    m_simNeuroInputVector.isRequired(m_simNeuroInputVectorName);
  }

  // require certain relations

  m_unpackedNeuroInput2DTracks.requireRelationTo(m_unpackedNeuroTracks);
  m_unpackedNeuroTracks.requireRelationTo(m_unpackedNeuroInputSegmentHits);
  m_unpackedNeuroTracks.requireRelationTo(m_unpackedNeuroInputVector);
  if (m_useSimTracks) {
    m_unpackedNeuroTracks.requireRelationTo(m_simNeuroTracks);
  }
  if (m_useRecoTracks) {
    m_RecoTracks.requireRelationTo(m_unpackedNeuroTracks);
  }
  if (m_useSimTracks && m_useRecoTracks) {
    m_RecoTracks.requireRelationTo(m_simNeuroTracks);
  }


}

void CDCTriggerNeuroDQMOnlineModule::beginRun()
{
  // Just to make sure, reset all the histograms.

  // histograms with recotracks (and hwsimtracks):

  m_neuroHWOutdzall->Reset();
  m_neuroHWOutdz0->Reset();
  m_neuroHWOutdz1->Reset();
  m_neuroHWOutdz2->Reset();
  m_neuroHWOutdz3->Reset();
  m_neuroHWSimRecodZ->Reset();

  // histograms with only hwneurotracks

  m_neuroHWOutZ->Reset();
  m_recoZ->Reset();
  m_recoZ_related->Reset();
  m_neuroHWOutSTTZ->Reset();
  m_neuroHWOutCosTheta->Reset();
  m_neuroHWOutPhi0->Reset();
  m_neuroHWOutPt->Reset();
  m_neuroHWOutP->Reset();
  m_neuroHWOutm_time->Reset();
  m_neuroHWValTracksNumber->Reset();
  m_neuroHWSector->Reset();
  m_neuroHWInTSID->Reset();
  m_neuroHWInTSIDSel->Reset();
  m_neuroHWInCDCFE->Reset();
  m_neuroHWInm_time->Reset();
  m_neuroHWIn2DTracksNumber->Reset();

  // histograms with hwsim tracks:

  m_neuroHWOutHwSimdZ->Reset();
  m_neuroHWSimZ->Reset();
  m_neuroHWSimCosTheta->Reset();
  m_neuroErrors->Reset();
  for (unsigned i = 0; i < m_errcount; ++i) {
    m_neuroErrors->GetXaxis()->SetBinLabel(i + 1, m_errdict[i].c_str());
  }
  //m_neuroErrorsRaw->Reset();
}
void CDCTriggerNeuroDQMOnlineModule::event()
{
  if (m_unpackedNeuroInputSegmentHits.getEntries() == 0) {
    B2DEBUG(150, "No unpacked TS found, skipping event.");
    return;
  }

  fillHWPlots();

  if (m_useRecoTracks) {fillRecoPlots();}

  if (m_useSimTracks) {
    fillSimPlots();
    makeDebugOutput();
  }



}

void CDCTriggerNeuroDQMOnlineModule::fillHWPlots()
{

  unsigned valtrackcount = 0;

  // first, fill the tsid plots and the cdcfe plot:

  for (const CDCTriggerSegmentHit& hit : m_unpackedNeuroInputAllSegmentHits) {
    m_neuroHWInTSID->Fill(hit.getSegmentID());
    m_neuroHWInCDCFE->Fill((int)(std::roundf(hit.getSegmentID()) * 2 + 8));
    m_neuroHWInCDCFE->Fill((int)(std::roundf(hit.getSegmentID()) * 2 + 9));
  }
  for (const CDCTriggerSegmentHit& hit : m_unpackedNeuroInputSegmentHits) {
    m_neuroHWInTSIDSel->Fill(hit.getSegmentID());
  }

  // now, we loop over the hardware neurotracks and fill the
  // corresponding histograms

  bool firsttrack = true;
  for (CDCTriggerTrack& neuroHWTrack : m_unpackedNeuroTracks) {
    bool valtrack = false;
    try {
      valtrack = neuroHWTrack.getValidStereoBit();
    } catch (...) {
      B2WARNING("HWTrack doesn't have 'valid bit', get it from relations now ...");
      valtrack = isValidPattern(getPattern(&neuroHWTrack, m_unpackedNeuroInputSegmentHitsName));
    }
    if (valtrack) {
      valtrackcount ++;
      m_neuroHWOutZ->Fill(neuroHWTrack.getZ0());
      m_neuroHWOutCosTheta->Fill(copysign(1.0,
                                          neuroHWTrack.getCotTheta()) / sqrt(1. / (neuroHWTrack.getCotTheta()*neuroHWTrack.getCotTheta())));
      double phinorm = neuroHWTrack.getPhi0() * 180. / M_PI;
      if (phinorm < 0.) {phinorm += 360.;}
      m_neuroHWOutPhi0->Fill(phinorm);
      m_neuroHWOutPt->Fill(neuroHWTrack.getPt());
      float momentum = neuroHWTrack.getPt() / sin(acos(neuroHWTrack.getCotTheta() / sqrt(1 + neuroHWTrack.getCotTheta() *
                                                       neuroHWTrack.getCotTheta())));
      m_neuroHWOutP->Fill(momentum);
      if (momentum < 0.7 && firsttrack && neuroHWTrack.getFoundOldTrack()[0] == false) {
        m_neuroHWOutSTTZ->Fill(neuroHWTrack.getZ0());
        firsttrack = false;
      }

      m_neuroHWOutm_time->Fill(neuroHWTrack.getTime());
      m_neuroHWSector->Fill(neuroHWTrack.getRelatedTo<CDCTriggerMLPInput>(m_unpackedNeuroInputVectorName)->getSector());


      CDCTriggerTrack* twodHWTrack = neuroHWTrack.getRelatedFrom<CDCTriggerTrack>(m_unpackedNeuroInput2DTracksName);
      m_neuroHWInm_time->Fill(twodHWTrack->getTime());

      // fill the error plots here, but only if simtracks are available
      // first, get the related simtrack
      if (m_useSimTracks) {
        CDCTriggerTrack* neuroSimTrack = neuroHWTrack.getRelatedTo<CDCTriggerTrack>(m_simNeuroTracksName);
        if (!neuroSimTrack) {
          B2WARNING("No Simtrack related to HWTrack! This should not happen!");
        }
        std::vector<float> unpackedInput =
          neuroHWTrack.getRelatedTo<CDCTriggerMLPInput>(m_unpackedNeuroInputVectorName)->getInput();
        std::vector<float> simInput =
          neuroSimTrack->getRelatedTo<CDCTriggerMLPInput>(m_simNeuroInputVectorName)->getInput();
        //unsigned unpackedSector =
        //  neuroHWTrack.getRelatedTo<CDCTriggerMLPInput>(m_unpackedNeuroInputVectorName)->getSector();
        //unsigned simSector =
        //  neuroSimTrack->getRelatedTo<CDCTriggerMLPInput>(m_simNeuroInputVectorName)->getSector();
        // m_neuroDeltaSector->Fill(unpackedSector - simSector);
        bool sameInputId = true;
        bool sameInputAlpha = true;
        bool scaleErr = false;
        bool missingTS = false;
        bool timeErr = false;
        for (unsigned ii = 0; ii < unpackedInput.size(); ii += 3) {
          bool hwZero = false;
          bool hwSimZero = false;
          if (unpackedInput[ii] != simInput[ii]) {sameInputId = false;}
          if (unpackedInput[ii + 2] != simInput[ii + 2]) {sameInputAlpha = false;}
          if (unpackedInput[ii + 1] != simInput[ii + 1]) {timeErr = true;}
          if (unpackedInput[ii + 1] == 0 && fabs(simInput[ii + 1] > 0.99)) {scaleErr = true;}
          if (simInput[ii + 1] == 0 && fabs(unpackedInput[ii + 1] > 0.99)) {scaleErr = true;}
          if (unpackedInput[ii] == 0 && unpackedInput[ii + 1] == 0 && unpackedInput[ii + 2] == 0) {hwZero = true;}
          if (simInput[ii] == 0 && simInput[ii + 1] == 0 && simInput[ii + 2] == 0) {hwSimZero = true;}
          if (hwZero != hwSimZero) {missingTS = true;}
        }
        double diff = neuroHWTrack.getZ0() - neuroSimTrack->getZ0();
        if (abs(diff) > 1.) {neuroHWTrack.setQualityVector(2);}
        if (!sameInputId) {neuroHWTrack.setQualityVector(4);}
        if (!sameInputAlpha) {neuroHWTrack.setQualityVector(8);}
        if (scaleErr) {neuroHWTrack.setQualityVector(16);}
        if (missingTS) {neuroHWTrack.setQualityVector(32);}
        if (timeErr) {neuroHWTrack.setQualityVector(64);}

        // now fill the error histogram:
        unsigned qvec = neuroHWTrack.getQualityVector();
        //m_neuroErrorsRaw->Fill(qvec);
        m_neuroErrors->Fill(8);
        for (unsigned k = 0; k < m_errcount; k++) {
          //if (qvec & (1 << k)) {m_neuroErrors->Fill(m_errordict[k], 1);}
          if (qvec & (1 << k)) {m_neuroErrors->Fill(k);}
        }
      }
    }

  }
  m_neuroHWIn2DTracksNumber->Fill(m_unpackedNeuroInput2DTracks.getEntries());
  m_neuroHWValTracksNumber->Fill(valtrackcount);
}



void CDCTriggerNeuroDQMOnlineModule::fillSimPlots()
{

  for (CDCTriggerTrack& neuroSimTrack : m_simNeuroTracks) {
    CDCTriggerTrack* neuroHWTrack = nullptr;
    bool valtrack = false;
    try {
      valtrack = neuroSimTrack.getValidStereoBit();
    } catch (...) {
      B2WARNING("SimTrack doesn't have 'valid bit', get it from relations now ...");
      valtrack = isValidPattern(getPattern(&neuroSimTrack, m_unpackedNeuroInputSegmentHitsName));
    }
    if (valtrack) {

      m_neuroHWSimZ->Fill(neuroSimTrack.getZ0());
      m_neuroHWSimCosTheta->Fill(copysign(1.0,
                                          neuroSimTrack.getCotTheta()) / sqrt(1. / (neuroSimTrack.getCotTheta()*neuroSimTrack.getCotTheta())));

      neuroHWTrack = neuroSimTrack.getRelatedFrom<CDCTriggerTrack>(m_unpackedNeuroTracksName);
      if (neuroHWTrack) {
        bool valhwtrack = false;
        try {
          valhwtrack = neuroHWTrack->getValidStereoBit();
        } catch (...) {
          B2WARNING("HWTrack doesn't have 'valid bit', get it from relations now ...");
          valhwtrack = isValidPattern(getPattern(neuroHWTrack, m_unpackedNeuroInputSegmentHitsName));
        }
        if (valhwtrack) {
          double diff = neuroHWTrack->getZ0() - neuroSimTrack.getZ0();
          m_neuroHWOutHwSimdZ->Fill(diff);
        }
      }

      // now check for recotracks and fill the delta plot

      if (m_useRecoTracks) {

        // try to find related neurotrack

        RecoTrack* recoTrack = neuroSimTrack.getRelatedFrom<RecoTrack>(m_recoTracksName);
        if (recoTrack) {
          double zTarget = 0;
          bool foundValidRep = false;

          for (genfit::AbsTrackRep* rep : recoTrack->getRepresentations()) {
            if (!recoTrack->wasFitSuccessful(rep))
              continue;
            // get state (position, momentum etc.) from hit closest to IP and
            // extrapolate to z-axis (may throw an exception -> continue to next representation)
            try {
              genfit::MeasuredStateOnPlane state =
                recoTrack->getMeasuredStateOnPlaneClosestTo(ROOT::Math::XYZVector(0, 0, 0), rep);
              rep->extrapolateToLine(state, TVector3(0, 0, -1000), TVector3(0, 0, 2000));

              zTarget = state.getPos().Z();
            } catch (...) {
              continue;
            }
            // break loop
            foundValidRep = true;
            break;
          }
          if (!foundValidRep) {
            B2DEBUG(150, "No valid representation found for RecoTrack, skipping.");
          } else {
            //no need to check valid bit again
            m_neuroHWSimRecodZ->Fill(neuroSimTrack.getZ0() - zTarget);
          }
        }
      }
    }
  }
}

void CDCTriggerNeuroDQMOnlineModule::makeDebugOutput()
{
  StoreObjPtr<EventMetaData> eventMetaData;
  static constexpr std::array<int, 9> nWiresInSuperLayer = {
    160, 160, 192, 224, 256, 288, 320, 352, 384
  };
  unsigned axhwts = 0;
  unsigned sthwts = 0;

  std::vector<TSLine> hwtsoutput;
  for (const CDCTriggerSegmentHit& xhit : m_unpackedNeuroInputSegmentHits) {
    if (xhit.getISuperLayer() % 2 == 0) {
      axhwts ++;
    } else {
      sthwts ++;
    }
    int iSL = xhit.getISuperLayer();
    int iTS = xhit.getIWire();
    int nwires = nWiresInSuperLayer[ iSL ];
    if (iSL == 8) {
      iTS += 16;
      if (iTS > nwires) {
        iTS -= nwires;
      }
    }
    int tsIDInTracker = iTS - nwires * xhit.getQuadrant() / 4;
    if (tsIDInTracker < 0) {
      tsIDInTracker += nwires;
    }
    TSLine l(xhit);
    l.strline = "("
                + padto(std::to_string(xhit.getISuperLayer()), 2) + ", "
                + padto(std::to_string(xhit.getQuadrant()), 1) + ", "
                + padto(std::to_string(xhit.getSegmentID()), 4) + ", "
                + padto(std::to_string(xhit.getIWire()), 5) + ", "
                + padto(std::to_string(xhit.getPriorityPosition()), 2) + ", "
                + padto(std::to_string(xhit.getLeftRight()), 2) + ", "
                + padto(std::to_string(xhit.priorityTime()), 4) + ", "
                + padto(std::to_string(xhit.foundTime()), 3) + ", "
                + padto(std::to_string(tsIDInTracker), 4) + ") | ";

    unsigned count2d = 0;
    for (CDCTriggerTrack& track : m_unpackedNeuroInput2DTracks) {
      count2d++;
      if (have_relation(track, xhit, m_unpackedNeuroInputSegmentHitsName)) {
        l.strline += std::to_string(count2d);
      } else {
        l.strline += ".";
      }
    }
    l.strline += " | ";
    unsigned counthwn = 0;
    for (CDCTriggerTrack& track : m_unpackedNeuroTracks) {
      counthwn++;
      if (have_relation(track, xhit, m_unpackedNeuroInputSegmentHitsName)) {
        if (track.getValidStereoBit()) {
          l.strline += std::to_string(counthwn);
        } else {
          l.strline += "x";
        }
      } else {
        l.strline += ".";
      }
    }
    l.strline += " | ";

    unsigned countswn = 0;
    for (CDCTriggerTrack& track : m_simNeuroTracks) {
      countswn++;
      if (have_relation(track, xhit, m_unpackedNeuroInputSegmentHitsName)) {
        l.strline += std::to_string(countswn);
      } else {
        l.strline += ".";
      }
    }
    l.strline += " | ";

    if (m_useRecoTracks) {
      unsigned countreco = 0;
      for (const auto& track : m_RecoTracks) {
        countreco++;
        bool related = false;
        for (const CDCTriggerSegmentHit& ts : track.getRelationsTo<CDCTriggerSegmentHit>(m_unpackedNeuroInputSegmentHitsName)) {
          if (&ts == &xhit) {related = true;}
        }
        if (related) {
          l.strline += std::to_string(countreco);
        } else {
          l.strline += ".";
        }
      }
      l.strline += " | ";
    }
    sorted_insert(hwtsoutput, l, m_unpackedNeuroInputSegmentHitsName, m_unpackedNeuroInput2DTracksName, m_unpackedNeuroTracksName);
  }

  std::string axhw = std::to_string(axhwts) + " / " + std::to_string(sthwts);
  std::string hwtsstring = "(SL, Q, SID , WID  , PP, LR, pT , fT , TSID) | HWNNIn2D | HWNeuro | SWNeuro | ";
  if (m_useRecoTracks) {hwtsstring += "Reco |  ";}
  B2DEBUG(10, padright(" ", 100));
  B2DEBUG(10, "----------------------------------------------------------------------------------------------------");
  B2DEBUG(10, padright(" ", 100));
  std::string experimentstring = "Experiment " + std::to_string(eventMetaData->getExperiment()) + "  Run " +
                                 std::to_string(eventMetaData->getRun()) + "  Event " + std::to_string(eventMetaData->getEvent());
  B2DEBUG(10, padright(experimentstring, 100));
  B2DEBUG(10, padright(" ", 100));
  B2DEBUG(10, padright(" ", 100));
  B2DEBUG(10,  padright("Number of NN HW TS (Axial/Stereo): ", 40) << padright(axhw, 60));
  B2DEBUG(10,  padright("Number of HW 2DFinderTracks: ", 40)            << padright(std::to_string(m_unpacked2DTracks.getEntries()),
          60));
  unsigned f2dtrn = 0;
  for (CDCTriggerTrack& ltrack : m_unpacked2DTracks) {
    f2dtrn++;
    std::stringstream strpt;
    std::stringstream stromega;
    std::stringstream strphi;
    strpt       << std::fixed << std::setprecision(2)   << ltrack.getPt();
    stromega    << std::fixed << std::setprecision(2)   << ltrack.getOmega();
    strphi      << std::fixed << std::setprecision(2)   << (ltrack.getPhi0() * 180. / M_PI);
    std::string trs  = "    HW2DFinderTrack Nr. " + std::to_string(f2dtrn) + " (pt, omega, phi) = ";
    trs += padto(strpt.str(), 6) + ", " + padto(stromega.str(), 6) + ", " + padto(strphi.str(), 6) + ")";
    B2DEBUG(15, padright(trs, 100));
  }
  B2DEBUG(10,  padright("Number of HW NNInput2DFinderTracks: ",
                        40)     << padright(std::to_string(m_unpackedNeuroInput2DTracks.getEntries()), 60));
  unsigned n2dtrn = 0;
  for (CDCTriggerTrack& ltrack : m_unpackedNeuroInput2DTracks) {
    n2dtrn++;
    std::stringstream strpt;
    std::stringstream stromega;
    std::stringstream strphi;
    strpt       << std::fixed << std::setprecision(2)   << ltrack.getPt();
    stromega    << std::fixed << std::setprecision(2)   << ltrack.getOmega();
    strphi      << std::fixed << std::setprecision(2)   << (ltrack.getPhi0() * 180. / M_PI);
    std::string trs  = "    HWNeuroInput2DTrack Nr. " + std::to_string(n2dtrn) + " (pt, omega, phi) = ";
    trs += padto(strpt.str(), 6) + ", " + padto(stromega.str(), 6) + ", " + padto(strphi.str(), 6) + ", " + ")";
    B2DEBUG(15, padright(trs, 100));
  }
  B2DEBUG(10,  padright("Number of HW NeuroTracks: ",
                        40)               << padright(std::to_string(m_unpackedNeuroTracks.getEntries()), 60));
  unsigned hwntrn = 0;
  for (CDCTriggerTrack& ltrack : m_unpackedNeuroTracks) {
    hwntrn++;
    std::stringstream strpt;
    std::stringstream stromega;
    std::stringstream strphi;
    std::stringstream strtheta;
    std::stringstream strz;
    std::stringstream hwomega;
    std::stringstream hwphi;
    std::stringstream hwtheta;
    std::stringstream hwz;
    strpt       << std::fixed << std::setprecision(2)   << ltrack.getPt();
    stromega    << std::fixed << std::setprecision(2)   << ltrack.getOmega();
    strphi      << std::fixed << std::setprecision(2)   << (ltrack.getPhi0() * 180. / M_PI);
    strtheta    << std::fixed << std::setprecision(2)   << (ltrack.getDirection().Theta() * 180. / M_PI);
    strz        << std::fixed << std::setprecision(2)   << ltrack.getZ0();
    hwomega     << std::fixed << std::setprecision(0)   << ltrack.getRawOmega();
    hwphi       << std::fixed << std::setprecision(0)   << ltrack.getRawPhi0();
    hwtheta     << std::fixed << std::setprecision(0)   << ltrack.getRawTheta();
    hwz         << std::fixed << std::setprecision(0)   << ltrack.getRawZ();
    std::string trs  = "    HWNeuroTrack Nr. " + std::to_string(hwntrn) + " (pt,om,phi,theta,z)=(";
    trs += padto(strpt.str(), 4) + "," + padto(stromega.str(), 6) + "," + padto(strphi.str(), 6) + "," + padto(strtheta.str(),
           6) + "," + padto(strz.str(), 6) + "),(x," + padto(hwomega.str(), 3) + "," + padto(hwphi.str(), 3) + "," + padto(hwtheta.str(),
               3) + "," + padto(hwz.str(), 3) + ")";
    B2DEBUG(15, padright(trs, 100));
    std::string infostr = ", Found old track: ( ";
    for (bool x : ltrack.getFoundOldTrack()) {
      infostr += std::to_string(x);
    }
    infostr += "), ";
    infostr = padright(infostr, 50);
    infostr += "Drift threshold: ( ";
    for (bool x : ltrack.getDriftThreshold()) {
      infostr += std::to_string(x);
    }
    infostr += ")";
    infostr += (ltrack.getValidStereoBit()) ? " valid" : " NOT valid";
    B2DEBUG(15, padright(infostr, 100));
    std::string infostr2 = "     std. ETF vld:";
    infostr2 += std::to_string(m_eventTime.isValid());
    infostr2 += ", ETFT0: ";
    if (m_eventTime.isValid()) {
      infostr2 += std::to_string(m_eventTime->getBinnedEventT0(Const::CDC));
    } else {
      infostr2 += "  ";
    }
    infostr2 += ", ETF in CC: ";
    infostr2 += std::to_string(ltrack.getETF_unpacked());
    infostr2 += ", ETF recalculated: ";
    infostr2 += std::to_string(ltrack.getETF_recalced());
    B2DEBUG(15, padright(infostr2, 100));
    std::string info2str = "      Expert Network Number: " + std::to_string(ltrack.getExpert());
    info2str += ", TSVector: (";
    for (unsigned x : ltrack.getTSVector()) {
      info2str += std::to_string(x) + " ";
    }
    info2str += ")";
    info2str += ", Quality=";
    info2str += std::to_string(ltrack.getQualityVector());
    B2DEBUG(15, padright(info2str, 100));
    CDCTriggerTrack* ftrack = ltrack.getRelatedFrom<CDCTriggerTrack>(m_unpackedNeuroInput2DTracksName);
    CDCTriggerTrack* strack = ftrack->getRelatedTo<CDCTriggerTrack>(m_simNeuroTracksName);
    if (strack) {
      std::vector<float> unpackedInput =
        ltrack.getRelatedTo<CDCTriggerMLPInput>(m_unpackedNeuroInputVectorName)->getInput();
      std::vector<float> simInput =
        strack->getRelatedTo<CDCTriggerMLPInput>(m_simNeuroInputVectorName)->getInput();
      B2DEBUG(20, padright("      Input Vector unpacked (id, t, alpha), sim (id, t, alpha), delta (id, t, alpha):", 100));
      for (unsigned ii = 0; ii < unpackedInput.size(); ii += 3) {
        std::string lla = "      " + std::to_string(ii / 3) + ")";
        std::string llb = "      " + std::to_string(ii / 3) + ")";
        lla += "(" + padright(std::to_string(unpackedInput[ii]), 8) + " " + padright(std::to_string(unpackedInput[ii + 1]),
               8) + " " + padright(std::to_string(unpackedInput[ii + 2]), 8) + "),(" + padright(std::to_string(simInput[ii]),
                   8) + " " + padright(std::to_string(simInput[ii + 1]), 8) + " " + padright(std::to_string(simInput[ii + 2]),
                       8) + "),(" + padright(std::to_string(unpackedInput[ii] - simInput[ii]),
                                             8) + " " + padright(std::to_string(unpackedInput[ii + 1] - simInput[ii + 1]),
                                                                 8) + " " + padright(std::to_string(unpackedInput[ii + 2] - simInput[ii + 2]), 8) + ")";
        llb += "  (" + padright(std::to_string(int(unpackedInput[ii] * 4096)),
                                8) + " " + padright(std::to_string(int(unpackedInput[ii + 1] * 4096)),
                                                    8) + " " + padright(std::to_string(int(unpackedInput[ii + 2] * 4096)),
                                                        8) + "),(" + padright(std::to_string(int(simInput[ii] * 4096)), 8) + " " + padright(std::to_string(int(simInput[ii + 1] * 4096)),
                                                            8) + " " + padright(std::to_string(int(simInput[ii + 2] * 4096)),
                                                                8) + "),(" + padright(std::to_string(int(unpackedInput[ii] * 4096 - simInput[ii] * 4096)),
                                                                    8) + " " + padright(std::to_string(int(unpackedInput[ii + 1] * 4096 - simInput[ii + 1] * 4096)),
                                                                        8) + " " + padright(std::to_string(int(unpackedInput[ii + 2] * 4096 - simInput[ii + 2] * 4096)), 8) + ")";

        B2DEBUG(30, padright(lla, 100));
        B2DEBUG(20, padright(llb, 100));
      }
    }
  }
  if (m_useRecoTracks) {
    B2DEBUG(10,  padright("Number of  RecoTracks: ", 40)                  << padright(std::to_string(m_RecoTracks.getEntries()), 60));
    unsigned recotrn = 0;
    for (RecoTrack& ltrack : m_RecoTracks) {
      double phi0Target = 0;
      double invptTarget = 0;
      double thetaTarget = 0;
      double zTarget = 0;
      double d0Target = 0;
      bool foundValidRep = false;
      for (genfit::AbsTrackRep* rep : ltrack.getRepresentations()) {
        if (!ltrack.wasFitSuccessful(rep))
          continue;
        // get state (position, momentum etc.) from hit closest to IP and
        // extrapolate to z-axis (may throw an exception -> continue to next representation)
        try {
          genfit::MeasuredStateOnPlane state =
            ltrack.getMeasuredStateOnPlaneClosestTo(ROOT::Math::XYZVector(0, 0, 0), rep);
          rep->extrapolateToLine(state, TVector3(0, 0, -1000), TVector3(0, 0, 2000));
          // TODO check after matching
          //  // flip tracks if necessary, such that trigger tracks and reco tracks
          //  // point in the same direction
          //  if (state.getMom().Dot(m_tracks[itrack]->getDirection()) < 0) {
          //    state.setPosMom(state.getPos(), -state.getMom());
          //    state.setChargeSign(-state.getCharge());
          //  }
          // get track parameters
          phi0Target = state.getMom().Phi() * 180. / M_PI ;
          if (phi0Target < 0.) {phi0Target = phi0Target + 360. ;}
          invptTarget = state.getCharge() * state.getMom().Pt();
          thetaTarget = acos(state.getMom().CosTheta()) * 180 / M_PI;
          zTarget = state.getPos().Z();
          d0Target = state.getPos().Perp();
        } catch (...) {
          continue;
        }
        // break loop
        foundValidRep = true;
        break;
      }
      if (!foundValidRep) {
        B2DEBUG(150, "No valid representation found for RecoTrack, skipping.");
        continue;
      }
      recotrn++;
      std::stringstream strpt;
      std::stringstream stromega;
      std::stringstream strphi;
      std::stringstream strtheta;
      std::stringstream strz;
      strpt       << std::fixed << std::setprecision(2)   << invptTarget;
      stromega    << std::fixed << std::setprecision(2)   << d0Target;
      strphi      << std::fixed << std::setprecision(2)   << phi0Target;
      strtheta    << std::fixed << std::setprecision(2)   << thetaTarget;
      strz        << std::fixed << std::setprecision(2)   << zTarget;
      std::string trs  = "    RecoTrack Nr. " + std::to_string(recotrn) + " (invpt, d0, phi, theta, z) = ";
      trs += padto(strpt.str(), 6) + ", " + padto(stromega.str(), 6) + ", " + padto(strphi.str(), 6) + ", " + padto(strtheta.str(),
             6) + ", " + padto(strz.str(), 6) + ")";
      B2DEBUG(15, padright(trs, 100));
    }
  }
  B2DEBUG(10, padright(" ", 100));
  B2DEBUG(10, padright(" ", 100));
  B2DEBUG(15, padright("Detailed information about HW TS ", 100));
  B2DEBUG(15, padright(" ", 100));
  B2DEBUG(15, padright(hwtsstring, 100));
  for (auto x : hwtsoutput) {
    B2DEBUG(15, padright(x.strline, 100));
  }
  B2DEBUG(15, padright(" ", 100));


}


void CDCTriggerNeuroDQMOnlineModule::fillRecoPlots()
{
  for (RecoTrack& recoTrack : m_RecoTracks) {
    double zTarget = 0;
    bool foundValidRep = false;
    for (genfit::AbsTrackRep* rep : recoTrack.getRepresentations()) {
      if (!recoTrack.wasFitSuccessful(rep))
        continue;
      // get state (position, momentum etc.) from hit closest to IP and
      // extrapolate to z-axis (may throw an exception -> continue to next representation)
      try {
        genfit::MeasuredStateOnPlane state =
          recoTrack.getMeasuredStateOnPlaneClosestTo(ROOT::Math::XYZVector(0, 0, 0), rep);
        rep->extrapolateToLine(state, TVector3(0, 0, -1000), TVector3(0, 0, 2000));
        // get track parameters
        //phi0Target = state.getMom().Phi() * 180. / M_PI ;
        //if (phi0Target < 0.) {phi0Target = phi0Target + 360. ;}
        //ptTarget = state.getCharge() * state.getMom().Pt();
        //cosThetaTarget = state.getMom().CosTheta();
        zTarget = state.getPos().Z();
      } catch (...) {
        continue;
      }
      // break loop
      foundValidRep = true;
      break;
    }
    if (!foundValidRep) {
      B2DEBUG(150, "No valid representation found for RecoTrack, skipping.");
      continue;
    }
    m_recoZ->Fill(zTarget);
    // try to find related neurotrack

    CDCTriggerTrack* neuroHWTrack = recoTrack.getRelatedFrom<CDCTriggerTrack>(m_unpackedNeuroTracksName);
    if (!neuroHWTrack) {continue;}
    bool valtrack = false;

    //check valid bit, else continue (valid ^= enough stereo track segments)

    try {
      valtrack = neuroHWTrack->getValidStereoBit();
    } catch (...) {
      B2WARNING("HWTrack doesn't have 'valid bit', get it from relations now ...");
      valtrack = isValidPattern(getPattern(neuroHWTrack, m_unpackedNeuroInputSegmentHitsName));
    }
    if (!valtrack) {continue;}
    // TODO check after matching
    //  // flip tracks if necessary, such that trigger tracks and reco tracks
    //  // point in the same direction
    //  if (state.getMom().Dot(m_tracks[itrack]->getDirection()) < 0) {
    //    state.setPosMom(state.getPos(), -state.getMom());
    //    state.setChargeSign(-state.getCharge());
    //  }
    m_recoZ_related->Fill(zTarget);
    m_neuroHWOutdzall->Fill(neuroHWTrack->getZ0() - zTarget);
    switch (neuroHWTrack->getQuadrant()) {
      case -1:
        B2WARNING("Neurotrack quadrant was not set!");
        break;
      case 0:
        m_neuroHWOutdz0->Fill(neuroHWTrack->getZ0() - zTarget);
        break;
      case 1:
        m_neuroHWOutdz1->Fill(neuroHWTrack->getZ0() - zTarget);
        break;
      case 2:
        m_neuroHWOutdz2->Fill(neuroHWTrack->getZ0() - zTarget);
        break;
      case 3:
        m_neuroHWOutdz3->Fill(neuroHWTrack->getZ0() - zTarget);
        break;
    }
  }
}
void CDCTriggerNeuroDQMOnlineModule::endRun()
{
}


void CDCTriggerNeuroDQMOnlineModule::terminate()
{
}
