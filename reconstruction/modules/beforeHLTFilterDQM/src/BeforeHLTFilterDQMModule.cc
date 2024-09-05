/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "reconstruction/modules/beforeHLTFilterDQM/BeforeHLTFilterDQMModule.h"

#include <framework/dataobjects/EventMetaData.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <cdc/dataobjects/CDCHit.h>
#include <mdst/dataobjects/EventLevelTrackingInfo.h>
#include <mdst/dataobjects/TRGSummary.h>

#include <TDirectory.h>
#include <TLine.h>
#include <TStyle.h>

#include <algorithm>


using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(BeforeHLTFilterDQM);


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

BeforeHLTFilterDQMModule::BeforeHLTFilterDQMModule() : HistoModule()
  , m_eklmElementNumbers{&(EKLMElementNumbers::Instance())}
{
  setDescription("DQM Module to monitor basic detector quantities before the HLT filter");

  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed.",
           std::string("BeforeHLTFilter"));

  setPropertyFlags(c_ParallelProcessingCertified);
}


BeforeHLTFilterDQMModule::~BeforeHLTFilterDQMModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void BeforeHLTFilterDQMModule::defineHisto()
{

  // Create a separate histogram directories and cd into it.
  TDirectory* oldDir = gDirectory;
  if (m_histogramDirectoryName != "") {
    oldDir->mkdir(m_histogramDirectoryName.c_str());
    oldDir->cd(m_histogramDirectoryName.c_str());
  }

  //histogram index:
  // 0 if the event is triggered OUTSIDE the active_veto window
  std::string tag[2] = {"OUT", "IN"};
  std::string title[2] = {"[Outside Active Veto Window]", "[Inside Active Veto Window]"};


  //BKLM plane occupancy (phi)
  //outside active_veto window:
  std::string histoName = "plane_bklm_phi";
  std::string histoTitle = "BKLM plane occupancy (#phi readout)";
  m_PlaneBKLMPhi[0] = new TH1F((histoName + "_" + tag[0]).c_str(),
                               (histoTitle + " " + title[0]).c_str(),
                               240, 0.5, 240.5);
  m_PlaneBKLMPhi[0]->GetXaxis()->SetTitle("Layer number");

  //inside active_veto window:
  m_PlaneBKLMPhi[1] = new TH1F(*m_PlaneBKLMPhi[0]);
  m_PlaneBKLMPhi[1]->SetName((histoName + "_" + tag[1]).c_str());

  m_PlaneBKLMPhi[1]->SetTitle((histoTitle + " " + title[1]).c_str());


  //BKLM plane occupancy (z)
  //outside active_veto window:
  histoName = "plane_bklm_z";
  histoTitle = "BKLM plane occupancy (z readout)";
  m_PlaneBKLMZ[0] = new TH1F((histoName + "_" + tag[0]).c_str(),
                             (histoTitle + " " + title[0]).c_str(),
                             240, 0.5, 240.5);
  m_PlaneBKLMZ[0]->GetXaxis()->SetTitle("Layer number");

  //inside active_veto window:
  m_PlaneBKLMZ[1] = new TH1F(*m_PlaneBKLMZ[0]);
  m_PlaneBKLMZ[1]->SetName((histoName + "_" + tag[1]).c_str());
  m_PlaneBKLMZ[1]->SetTitle((histoTitle + " " + title[1]).c_str());


  //EKLM plane occupancy
  //outside active_veto window:
  histoName = "plane_eklm";
  histoTitle = "EKLM plane occupancy (both readouts)";
  m_PlaneEKLM[0] = new TH1F((histoName + "_" + tag[0]).c_str(),
                            (histoTitle + " " + title[0]).c_str(),
                            208, 0.5, 208.5);
  m_PlaneEKLM[0]->GetXaxis()->SetTitle("Plane number");

  //inside active_veto window:
  m_PlaneEKLM[1] = new TH1F(*m_PlaneEKLM[0]);
  m_PlaneEKLM[1]->SetName((histoName + "_" + tag[1]).c_str());
  m_PlaneEKLM[1]->SetTitle((histoTitle + " " + title[1]).c_str());

  //RPC Time
  histoName = "time_rpc";
  histoTitle = "RPC Hit Time";
  m_TimeRPC[0] = new TH1F((histoName + "_" + tag[0]).c_str(),
                          (histoTitle + " " + title[0]).c_str(),
                          128, double(-1223.5), double(-199.5));
  m_TimeRPC[0]->GetXaxis()->SetTitle("Time [ns]");

  //inside active_veto window:
  m_TimeRPC[1] = new TH1F(*m_TimeRPC[0]);
  m_TimeRPC[1]->SetName((histoName + "_" + tag[1]).c_str());
  m_TimeRPC[1]->SetTitle((histoTitle + " " + title[1]).c_str());

  //BKLM SCintillator Time
  histoName = "time_scintillator_bklm";
  histoTitle = "Scintillator Hit Time (BKLM)";
  m_TimeScintillatorBKLM[0] = new TH1F((histoName + "_" + tag[0]).c_str(),
                                       (histoTitle + " " + title[0]).c_str(),
                                       100, double(-5300), double(-4300));
  m_TimeScintillatorBKLM[0]->GetXaxis()->SetTitle("Time [ns]");

  //inside active_veto window:
  m_TimeScintillatorBKLM[1] = new TH1F(*m_TimeScintillatorBKLM[0]);
  m_TimeScintillatorBKLM[1]->SetName((histoName + "_" + tag[1]).c_str());
  m_TimeScintillatorBKLM[1]->SetTitle((histoTitle + " " + title[1]).c_str());

  //EKLM SCintillator Time
  histoName = "time_scintillator_eklm";
  histoTitle = "Scintillator Hit Time (EKLM)";
  m_TimeScintillatorEKLM[0] = new TH1F((histoName + "_" + tag[0]).c_str(),
                                       (histoTitle + " " + title[0]).c_str(),
                                       100, double(-5300), double(-4300));
  m_TimeScintillatorEKLM[0]->GetXaxis()->SetTitle("Time [ns]");

  //inside active_veto window:
  m_TimeScintillatorEKLM[1] = new TH1F(*m_TimeScintillatorEKLM[0]);
  m_TimeScintillatorEKLM[1]->SetName((histoName + "_" + tag[1]).c_str());
  m_TimeScintillatorEKLM[1]->SetTitle((histoTitle + " " + title[1]).c_str());

  //ARICH plane occupancy
  //outside active_veto window:
  histoName = "arich_occ";
  histoTitle = "ARICH Occupancy";
  m_ARICHOccupancy[0] = new TH1F((histoName + "_" + tag[0]).c_str(),
                                 (histoTitle + " " + title[0]).c_str(),
                                 201, -0.5, 200.5);
  m_ARICHOccupancy[0]->GetXaxis()->SetTitle("Number of hits");

  //inside active_veto window:
  m_ARICHOccupancy[1] = new TH1F(*m_ARICHOccupancy[0]);
  m_ARICHOccupancy[1]->SetName((histoName + "_" + tag[1]).c_str());
  m_ARICHOccupancy[1]->SetTitle((histoTitle + " " + title[1]).c_str());

  //TOP occupancy
  histoName = "top_occ";
  histoTitle = "TOP occupancy for good hits";
  for (int i = 0; i < 2; i++) {
    m_topOccupancy[i] = new TH1F((histoName + "_" + tag[i]).c_str(),
                                 (histoTitle + " " +  title[i]).c_str(),
                                 1000, 0, 10000);
    m_topOccupancy[i]->SetXTitle("hits per event");
    m_topOccupancy[i]->SetYTitle("entries per bin");
  }




  oldDir->cd();
}

void BeforeHLTFilterDQMModule::initialize()
{
  m_eventMetaData.isOptional();
  m_trgSummary.isOptional();
  m_KLMDigits.isOptional();
  m_BklmHit1ds.isOptional();
  m_ARICHHits.isOptional();
  m_topDigits.isOptional();

  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM
}


void BeforeHLTFilterDQMModule::beginRun()
{

  for (int i = 0; i < 2; i++) {
    if (m_PlaneBKLMPhi[i] != nullptr)  m_PlaneBKLMPhi[i]->Reset();
    if (m_PlaneBKLMZ[i] != nullptr)  m_PlaneBKLMZ[i]->Reset();
    if (m_PlaneEKLM[i] != nullptr)  m_PlaneEKLM[i]->Reset();
    if (m_ARICHOccupancy[i] != nullptr)  m_ARICHOccupancy[i]->Reset();
    if (m_topOccupancy[i] != nullptr)  m_topOccupancy[i]->Reset();

  }
}


void BeforeHLTFilterDQMModule::event()
{

  //skip events in which we do not have EventMetaData or TRGSummary
  if (!m_eventMetaData.isValid()) return;
  if (!m_trgSummary.isValid()) return;

  //skip the empty events
  if (m_eventMetaData->getErrorFlag() & EventMetaData::EventErrorFlag::c_B2LinkPacketCRCError)
    return;

  if (m_eventMetaData->getErrorFlag() & EventMetaData::EventErrorFlag::c_B2LinkEventCRCError)
    return;

  if (m_eventMetaData->getErrorFlag() & EventMetaData::EventErrorFlag::c_HLTCrash)
    return;

  if (m_eventMetaData->getErrorFlag() & EventMetaData::EventErrorFlag::c_ReconstructionAbort)
    return;

  //find out if we are in the passive veto (i=0) or in the active veto window (i=1)
  int index = 0; //events accepted in the passive veto window but not in the active
  if (m_trgSummary->testInput("passive_veto") == 1 &&  m_trgSummary->testInput("cdcecl_veto") == 0) index = 1;


  //fill the BKLM plane occupancy pliots
  for (const BKLMHit1d& hit1d : m_BklmHit1ds) {
    int section = hit1d.getSection();
    int sector = hit1d.getSector();
    int layer = hit1d.getLayer();
    int layerGlobal = BKLMElementNumbers::layerGlobalNumber(
                        section, sector, layer);
    if (hit1d.isPhiReadout())
      m_PlaneBKLMPhi[index]->Fill(layerGlobal);
    else
      m_PlaneBKLMZ[index]->Fill(layerGlobal);

  }

  for (const KLMDigit& digit : m_KLMDigits) {
    /*
     * Reject digits that are below the threshold (such digits may appear
     * for simulated events).
     */
    if (!digit.isGood())
      continue;

    if (digit.getSubdetector() == KLMElementNumbers::c_EKLM) {
      int section = digit.getSection();
      int layer = digit.getLayer();
      int sector = digit.getSector();
      int plane = digit.getPlane();
      int planeGlobal = m_eklmElementNumbers->planeNumber(section, layer, sector, plane);
      m_PlaneEKLM[index]->Fill(planeGlobal);
      m_TimeScintillatorEKLM[index]->Fill(digit.getTime());
    } else if (digit.getSubdetector() == KLMElementNumbers::c_BKLM) {

      if (digit.inRPC()) m_TimeRPC[index]->Fill(digit.getTime());
      else m_TimeScintillatorBKLM[index]->Fill(digit.getTime());
    }
  }

  int arichNentr = m_ARICHHits.isValid() ?  m_ARICHHits.getEntries() : 0;
  m_ARICHOccupancy[index] -> Fill(arichNentr > 200 ? 200 : arichNentr);


  int topGoodHits = 0;
  for (const auto& digit : m_topDigits) {
    if (digit.getHitQuality() != TOPDigit::c_Junk) topGoodHits++;
  }
  m_topOccupancy[index]->Fill(topGoodHits);

}
