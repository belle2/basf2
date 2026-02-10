/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "reconstruction/modules/detectorOccupanciesDQM/DetectorOccupanciesDQMModule.h"

#include <framework/dataobjects/EventMetaData.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLElementNumbers.h>
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/KLMDigit.h>
#include <arich/dataobjects/ARICHHit.h>
#include <top/dataobjects/TOPDigit.h>

#include <TDirectory.h>



using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DetectorOccupanciesDQM);


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DetectorOccupanciesDQMModule::DetectorOccupanciesDQMModule() : HistoModule()
  , m_eklmElementNumbers{&(EKLMElementNumbers::Instance())}, m_klmTime{&(KLMTime::Instance())}
{
  setDescription("DQM Module to monitor basic detector quantities");

  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed.",
           std::string("DetectorOccupancies"));
  addParam("eclEnergyThr", m_eclEnergyThr, "Energy threshold (in MeV) for ECL occupancy histogram", 5.0);
  addParam("BKLMTimeMin", m_BKLMTimeMin,
           "Min time for BKLM time histogram.", double(-6500));
  addParam("BKLMTimeMax", m_BKLMTimeMax,
           "Max time for BKLM time histogram.", double(5000));
  addParam("EKLMTimeMin", m_EKLMTimeMin,
           "Min time for EKLM time histogram.", double(-5100));
  addParam("EKLMTimeMax", m_EKLMTimeMax,
           "Max time for EKLM time histogram.", double(-4500));
  addParam("BKLMScintOffset", m_BKLMScintOffset,
           "Offset to center BKLM Scint. times at 0", double(-4705.));
  addParam("BKLMRPCOffset", m_BKLMRPCOffset,
           "Offset to center BKLM RPC times at 0", double(-4300.5));
  setPropertyFlags(c_ParallelProcessingCertified);
}


DetectorOccupanciesDQMModule::~DetectorOccupanciesDQMModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void DetectorOccupanciesDQMModule::defineHisto()
{

  // Create a separate histogram directories and cd into it.
  TDirectory* oldDir = gDirectory;
  if (m_histogramDirectoryName != "") {
    oldDir->mkdir(m_histogramDirectoryName.c_str());
    oldDir->cd(m_histogramDirectoryName.c_str());
  }

  //histogram index:
  // 0 if the event is triggered OUTSIDE the active_veto window
  const std::string tag[2] = {"OUT", "IN"};
  const std::string title[2] = {"[Outside Active Veto Window]", "[Inside Active Veto Window]"};


  //BKLM plane occupancy
  //outside active_veto window:
  std::string histoName = "bklm_plane_occupancy";
  std::string histoTitle = "BKLM plane occupancy";
  m_BKLM_Plane_Occupancy[0] = new TH2F((histoName + "_" + tag[0]).c_str(),
                                       (histoTitle + " " + title[0]).c_str(),
                                       240, 0.5, 240.5,
                                       200, m_BKLMTimeMin, m_BKLMTimeMax);
  m_BKLM_Plane_Occupancy[0]->GetXaxis()->SetTitle("Layer number");
  m_BKLM_Plane_Occupancy[0]->GetYaxis()->SetTitle("Shifted Time (ns)");

  //inside active_veto window:
  m_BKLM_Plane_Occupancy[1] = new TH2F(*m_BKLM_Plane_Occupancy[0]);
  m_BKLM_Plane_Occupancy[1]->SetName((histoName + "_" + tag[1]).c_str());

  m_BKLM_Plane_Occupancy[1]->SetTitle((histoTitle + " " + title[1]).c_str());


  //BKLM plane occupancy (w/ random triggers)
  //outside active_veto window:
  histoName = "bklm_plane_trg_occupancy";
  histoTitle = "BKLM plane occupancy (w/ trgs)";
  m_BKLM_PlaneTrg_Occupancy[0] = new TH2F((histoName + "_" + tag[0]).c_str(),
                                          (histoTitle + " " + title[0]).c_str(),
                                          240, 0.5, 240.5,
                                          200, m_BKLMTimeMin, m_BKLMTimeMax);
  m_BKLM_PlaneTrg_Occupancy[0]->GetXaxis()->SetTitle("Layer number");
  m_BKLM_PlaneTrg_Occupancy[0]->GetYaxis()->SetTitle("Shifted Time (ns)");

  //inside active_veto window:
  m_BKLM_PlaneTrg_Occupancy[1] = new TH2F(*m_BKLM_PlaneTrg_Occupancy[0]);
  m_BKLM_PlaneTrg_Occupancy[1]->SetName((histoName + "_" + tag[1]).c_str());
  m_BKLM_PlaneTrg_Occupancy[1]->SetTitle((histoTitle + " " + title[1]).c_str());


  //EKLM plane occupancy
  //outside active_veto window:
  histoName = "eklm_plane_occupancy";
  histoTitle = "EKLM plane occupancy";
  m_EKLM_Plane_Occupancy[0] = new TH2F((histoName + "_" + tag[0]).c_str(),
                                       (histoTitle + " " + title[0]).c_str(),
                                       208, 0.5, 208.5,
                                       100, m_EKLMTimeMin, m_EKLMTimeMax);
  m_EKLM_Plane_Occupancy[0]->GetXaxis()->SetTitle("Plane number");
  m_EKLM_Plane_Occupancy[0]->GetYaxis()->SetTitle("Time from L1Trigger (ns)");

  //inside active_veto window:
  m_EKLM_Plane_Occupancy[1] = new TH2F(*m_EKLM_Plane_Occupancy[0]);
  m_EKLM_Plane_Occupancy[1]->SetName((histoName + "_" + tag[1]).c_str());
  m_EKLM_Plane_Occupancy[1]->SetTitle((histoTitle + " " + title[1]).c_str());

  //EKLM plane occupancy w/ random triggers
  //outside active_veto window:
  histoName = "eklm_plane_trg_occupancy";
  histoTitle = "EKLM plane occupancy (w/ trgs)";
  m_EKLM_PlaneTrg_Occupancy[0] = new TH2F((histoName + "_" + tag[0]).c_str(),
                                          (histoTitle + " " + title[0]).c_str(),
                                          208, 0.5, 208.5,
                                          100, m_EKLMTimeMin, m_EKLMTimeMax);
  m_EKLM_PlaneTrg_Occupancy[0]->GetXaxis()->SetTitle("Plane number");
  m_EKLM_PlaneTrg_Occupancy[0]->GetXaxis()->SetTitle("Time from L1Trigger (ns)");

  //inside active_veto window:
  m_EKLM_PlaneTrg_Occupancy[1] = new TH2F(*m_EKLM_PlaneTrg_Occupancy[0]);
  m_EKLM_PlaneTrg_Occupancy[1]->SetName((histoName + "_" + tag[1]).c_str());
  m_EKLM_PlaneTrg_Occupancy[1]->SetTitle((histoTitle + " " + title[1]).c_str());



  //ARICH plane occupancy
  //outside active_veto window:
  histoName = "arich_occupancy";
  histoTitle = "ARICH Occupancy";
  m_ARICH_Occupancy[0] = new TH1F((histoName + "_" + tag[0]).c_str(),
                                  (histoTitle + " " + title[0]).c_str(),
                                  201, -0.5, 200.5);
  m_ARICH_Occupancy[0]->GetXaxis()->SetTitle("hits per event");

  //inside active_veto window:
  m_ARICH_Occupancy[1] = new TH1F(*m_ARICH_Occupancy[0]);
  m_ARICH_Occupancy[1]->SetName((histoName + "_" + tag[1]).c_str());
  m_ARICH_Occupancy[1]->SetTitle((histoTitle + " " + title[1]).c_str());

  //TOP occupancy
  histoName = "top_occupancy";
  histoTitle = "TOP Occupancy for good hits";
  for (int i = 0; i < 2; i++) {
    m_TOP_Occupancy[i] = new TH1F((histoName + "_" + tag[i]).c_str(),
                                  (histoTitle + " " +  title[i]).c_str(),
                                  1000, 0, 10000);
    m_TOP_Occupancy[i]->SetXTitle("hits per event");
    m_TOP_Occupancy[i]->SetYTitle("entries per bin");
  }

  //ECL occupancy
  histoName = "ecl_occupancy";
  histoTitle = "ECL occupancy (for hits with E > " + std::to_string((int)m_eclEnergyThr) + " MeV)";
  for (int i = 0; i < 2; i++) {
    m_ECL_Occupancy[i] = new TProfile((histoName + "_" + tag[i]).c_str(),
                                      (histoTitle + " " +  title[i]).c_str(),
                                      ECLElementNumbers::c_NCrystals,
                                      1, ECLElementNumbers::c_NCrystals + 1);
    m_ECL_Occupancy[i]->SetXTitle("cell id");
    m_ECL_Occupancy[i]->SetYTitle("Occupancy (hits / evt_count)");
  }


  oldDir->cd();
}

void DetectorOccupanciesDQMModule::initialize()
{
  m_eventMetaData.isOptional();
  m_trgSummary.isOptional();
  m_KLMDigits.isOptional();
  m_ARICHHits.isOptional();
  m_topDigits.isOptional();
  m_eclCalDigits.isOptional();

  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM
}


void DetectorOccupanciesDQMModule::beginRun()
{
  m_klmTime->updateConstants(); //to get correct CTime
  for (int i = 0; i < 2; i++) {
    if (m_BKLM_Plane_Occupancy[i] != nullptr)  m_BKLM_Plane_Occupancy[i]->Reset();
    if (m_BKLM_PlaneTrg_Occupancy[i] != nullptr)  m_BKLM_PlaneTrg_Occupancy[i]->Reset();
    if (m_EKLM_Plane_Occupancy[i] != nullptr)  m_EKLM_Plane_Occupancy[i]->Reset();
    if (m_EKLM_PlaneTrg_Occupancy[i] != nullptr)  m_EKLM_PlaneTrg_Occupancy[i]->Reset();
    if (m_ARICH_Occupancy[i] != nullptr)  m_ARICH_Occupancy[i]->Reset();
    if (m_TOP_Occupancy[i] != nullptr)  m_TOP_Occupancy[i]->Reset();
    if (m_ECL_Occupancy[i] != nullptr)  m_ECL_Occupancy[i]->Reset();

  }
}


void DetectorOccupanciesDQMModule::event()
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
  try {
    if (m_trgSummary->testInput("passive_veto") == 1 &&  m_trgSummary->testInput("cdcecl_veto") == 0) index = 1;
  } catch (const std::exception&) {
  }

  // Check if any L1Triggers in the backTriggers list are hot
  bool backBooleanFlag = std::any_of(
                           std::begin(m_klmBackTriggers),
                           std::end(m_klmBackTriggers),
  [trg = m_trgSummary](TRGSummary::ETimingType trgBit) {
    return trg->testInput(trgBit);
  }
                         );


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
      m_EKLM_Plane_Occupancy[index]->Fill(planeGlobal, digit.getTime());
      if (backBooleanFlag) m_EKLM_PlaneTrg_Occupancy[index]->Fill(planeGlobal, digit.getTime());
    } else if (digit.getSubdetector() == KLMElementNumbers::c_BKLM) {
      int section = digit.getSection();
      int layer = digit.getLayer();
      int sector = digit.getSector();
      int layerGlobal = BKLMElementNumbers::layerGlobalNumber(
                          section, sector, layer);
      float offset = (digit.inRPC()) ? m_BKLMRPCOffset : m_BKLMScintOffset;
      float rawTime = (digit.inRPC()) ? digit.getRevo9DCArrivalTime() * m_klmTime->getCTimePeriod() : digit.getTime();
      float time = rawTime - offset; //shift is to align scintillator and RPC hits on one graph

      bool goodHit = (rawTime > -11000 && rawTime < 0);  //~11us from L1 trigger
      if (!goodHit)
        continue;
      m_BKLM_Plane_Occupancy[index]->Fill(layerGlobal, time);
      if (backBooleanFlag)
        m_BKLM_PlaneTrg_Occupancy[index]->Fill(layerGlobal, time);
    }
  }

  float xMax =   m_ARICH_Occupancy[0]->GetXaxis()->GetBinCenter(m_ARICH_Occupancy[0]->GetNbinsX());
  int arichNentr = m_ARICHHits.isValid() ?  m_ARICHHits.getEntries() : 0;
  m_ARICH_Occupancy[index] -> Fill(arichNentr > xMax ? xMax : arichNentr);


  int topGoodHits = 0;
  for (const auto& digit : m_topDigits) {
    if (digit.getHitQuality() != TOPDigit::c_Junk) topGoodHits++;
  }
  m_TOP_Occupancy[index]->Fill(topGoodHits);

  std::array<bool, ECLElementNumbers::c_NCrystals> crystal_hit;

  for (const auto& digit : m_eclCalDigits) {
    const double thresholdGeV = m_eclEnergyThr * 1e-3;
    if (digit.getEnergy() > thresholdGeV)
      crystal_hit.at(digit.getCellId() - 1) = true;
  }
  for (int cid0 = 0; cid0 < ECLElementNumbers::c_NCrystals; cid0++) {
    m_ECL_Occupancy[index]->Fill(cid0 + 1, crystal_hit[cid0]);
  }

}
