/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "reconstruction/modules/detectorOccupanciesDQM/DetectorOccupanciesDQMModule.h"

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
  , m_eklmElementNumbers{&(EKLMElementNumbers::Instance())}
{
  setDescription("DQM Module to monitor basic detector quantities");

  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed.",
           std::string("DetectorOccupancies"));

  addParam("eclEnergyThr", m_eclEnergyThr, "Energy threshold (in MeV) for ECL occupancy histogram", 5.0);

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


  //BKLM plane occupancy (phi)
  //outside active_veto window:
  std::string histoName = "bklm_plane_phi_occupancy";
  std::string histoTitle = "BKLM plane occupancy (#phi readout)";
  m_BKLM_PlanePhi_Occupancy[0] = new TH1F((histoName + "_" + tag[0]).c_str(),
                                          (histoTitle + " " + title[0]).c_str(),
                                          240, 0.5, 240.5);
  m_BKLM_PlanePhi_Occupancy[0]->GetXaxis()->SetTitle("Layer number");

  //inside active_veto window:
  m_BKLM_PlanePhi_Occupancy[1] = new TH1F(*m_BKLM_PlanePhi_Occupancy[0]);
  m_BKLM_PlanePhi_Occupancy[1]->SetName((histoName + "_" + tag[1]).c_str());

  m_BKLM_PlanePhi_Occupancy[1]->SetTitle((histoTitle + " " + title[1]).c_str());


  //BKLM plane occupancy (z)
  //outside active_veto window:
  histoName = "bklm_plane_z_occupancy";
  histoTitle = "BKLM plane occupancy (z readout)";
  m_BKLM_PlaneZ_Occupancy[0] = new TH1F((histoName + "_" + tag[0]).c_str(),
                                        (histoTitle + " " + title[0]).c_str(),
                                        240, 0.5, 240.5);
  m_BKLM_PlaneZ_Occupancy[0]->GetXaxis()->SetTitle("Layer number");

  //inside active_veto window:
  m_BKLM_PlaneZ_Occupancy[1] = new TH1F(*m_BKLM_PlaneZ_Occupancy[0]);
  m_BKLM_PlaneZ_Occupancy[1]->SetName((histoName + "_" + tag[1]).c_str());
  m_BKLM_PlaneZ_Occupancy[1]->SetTitle((histoTitle + " " + title[1]).c_str());


  //EKLM plane occupancy
  //outside active_veto window:
  histoName = "eklm_plane_occupancy";
  histoTitle = "EKLM plane occupancy (both readouts)";
  m_EKLM_Plane_Occupancy[0] = new TH1F((histoName + "_" + tag[0]).c_str(),
                                       (histoTitle + " " + title[0]).c_str(),
                                       208, 0.5, 208.5);
  m_EKLM_Plane_Occupancy[0]->GetXaxis()->SetTitle("Plane number");

  //inside active_veto window:
  m_EKLM_Plane_Occupancy[1] = new TH1F(*m_EKLM_Plane_Occupancy[0]);
  m_EKLM_Plane_Occupancy[1]->SetName((histoName + "_" + tag[1]).c_str());
  m_EKLM_Plane_Occupancy[1]->SetTitle((histoTitle + " " + title[1]).c_str());

  //RPC Time
  histoName = "bklm_rpc_time";
  histoTitle = "BKLM RPC Hit Time";
  m_BKLM_TimeRPC[0] = new TH1F((histoName + "_" + tag[0]).c_str(),
                               (histoTitle + " " + title[0]).c_str(),
                               128, double(-1223.5), double(-199.5));
  m_BKLM_TimeRPC[0]->GetXaxis()->SetTitle("Time [ns]");

  //inside active_veto window:
  m_BKLM_TimeRPC[1] = new TH1F(*m_BKLM_TimeRPC[0]);
  m_BKLM_TimeRPC[1]->SetName((histoName + "_" + tag[1]).c_str());
  m_BKLM_TimeRPC[1]->SetTitle((histoTitle + " " + title[1]).c_str());

  //BKLM Scintillator Time
  histoName = "bklm_scintillator_time";
  histoTitle = "BKLM Scintillator Hit Time";
  m_BKLM_TimeScintillator[0] = new TH1F((histoName + "_" + tag[0]).c_str(),
                                        (histoTitle + " " + title[0]).c_str(),
                                        100, double(-5300), double(-4300));
  m_BKLM_TimeScintillator[0]->GetXaxis()->SetTitle("Time [ns]");

  //inside active_veto window:
  m_BKLM_TimeScintillator[1] = new TH1F(*m_BKLM_TimeScintillator[0]);
  m_BKLM_TimeScintillator[1]->SetName((histoName + "_" + tag[1]).c_str());
  m_BKLM_TimeScintillator[1]->SetTitle((histoTitle + " " + title[1]).c_str());

  //EKLM SCintillator Time
  histoName = "eklm_scintillator_time";
  histoTitle = "EKLM Scintillator Hit Time";
  m_EKLM_TimeScintillator[0] = new TH1F((histoName + "_" + tag[0]).c_str(),
                                        (histoTitle + " " + title[0]).c_str(),
                                        100, double(-5300), double(-4300));
  m_EKLM_TimeScintillator[0]->GetXaxis()->SetTitle("Time [ns]");

  //inside active_veto window:
  m_EKLM_TimeScintillator[1] = new TH1F(*m_EKLM_TimeScintillator[0]);
  m_EKLM_TimeScintillator[1]->SetName((histoName + "_" + tag[1]).c_str());
  m_EKLM_TimeScintillator[1]->SetTitle((histoTitle + " " + title[1]).c_str());

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
  m_BklmHit1ds.isOptional();
  m_ARICHHits.isOptional();
  m_topDigits.isOptional();
  m_eclCalDigits.isOptional();

  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM
}


void DetectorOccupanciesDQMModule::beginRun()
{

  for (int i = 0; i < 2; i++) {
    if (m_BKLM_PlanePhi_Occupancy[i] != nullptr)  m_BKLM_PlanePhi_Occupancy[i]->Reset();
    if (m_BKLM_PlaneZ_Occupancy[i] != nullptr)  m_BKLM_PlaneZ_Occupancy[i]->Reset();
    if (m_EKLM_Plane_Occupancy[i] != nullptr)  m_EKLM_Plane_Occupancy[i]->Reset();
    if (m_BKLM_TimeRPC[i] != nullptr) m_BKLM_TimeRPC[i]->Reset();
    if (m_BKLM_TimeScintillator[i] != nullptr) m_BKLM_TimeScintillator[i]->Reset();
    if (m_EKLM_TimeScintillator[i] != nullptr) m_EKLM_TimeScintillator[i]->Reset();
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

  //fill the BKLM plane occupancy plots
  for (const BKLMHit1d& hit1d : m_BklmHit1ds) {
    int section = hit1d.getSection();
    int sector = hit1d.getSector();
    int layer = hit1d.getLayer();
    int layerGlobal = BKLMElementNumbers::layerGlobalNumber(
                        section, sector, layer);
    if (hit1d.isPhiReadout())
      m_BKLM_PlanePhi_Occupancy[index]->Fill(layerGlobal);
    else
      m_BKLM_PlaneZ_Occupancy[index]->Fill(layerGlobal);

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
      m_EKLM_Plane_Occupancy[index]->Fill(planeGlobal);
      m_EKLM_TimeScintillator[index]->Fill(digit.getTime());
    } else if (digit.getSubdetector() == KLMElementNumbers::c_BKLM) {

      if (digit.inRPC()) m_BKLM_TimeRPC[index]->Fill(digit.getTime());
      else m_BKLM_TimeScintillator[index]->Fill(digit.getTime());
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
