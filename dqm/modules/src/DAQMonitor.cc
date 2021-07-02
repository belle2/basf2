/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DAQMonitor.cc
// Description : Module to monitor raw data
//-

/* Own header. */
#include <dqm/modules/DAQMonitor.h>

/* ROOT headers. */
#include <TDirectory.h>

using namespace Belle2;

REG_MODULE(DAQMonitor)

DAQMonitorModule::DAQMonitorModule() : HistoModule()
{
  setDescription("This module produces general DAQ DQM histograms.");
  setPropertyFlags(c_ParallelProcessingCertified);
}

void DAQMonitorModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir("DAQ");
  oldDir->cd("DAQ");
  h_nEvt = new TH1F("Nevent", "Total Number of Events", 3, 0.0, 2.0);
  h_pxdSize = new TH1F("PXDDataSize", "PXD Data Size;Size [kB];", 100, 0.0, 100.0);
  h_svdSize = new TH1F("SVDDataSize", "SVD Data Size;Size [kB];", 100, 0.0, 100.0);
  h_cdcSize = new TH1F("CDCDataSize", "CDC Data Size;Size [kB];", 100, 0.0, 100.0);
  h_topSize = new TH1F("TOPDataSize", "TOP Data Size;Size [kB];", 100, 0.0, 100.0);
  h_arichSize = new TH1F("ARICHDataSize", "ARICH Data Size;Size [kB];", 100, 0.0, 40.0);
  h_eclSize = new TH1F("ECLDataSize", "ECL Data Size;Size [kB];", 100, 0.0, 100.0);
  h_klmSize = new TH1F("KLMDataSize", "KLM Data Size;Size [kB];", 100, 0.0, 40.0);
  h_trgSize = new TH1F("TRGDataSize", "TRG Data Size;Size [kB];", 100, 0.0, 40.0);
  h_hltSize = new TH1F("HLTDataSize", "HLT (Total - PXD) Data Size;Size [kB];", 100, 0.0, 300.0);
  h_totalSize = new TH1F("TotalDataSize", "Total (HLT + PXD) Data Size;Size [kB];", 100, 0.0, 300.0);
  oldDir->cd();
}

void DAQMonitorModule::initialize()
{
  REG_HISTOGRAM;
  m_pxdRaw.isOptional();
  m_svdRaw.isOptional();
  m_cdcRaw.isOptional();
  m_topRaw.isOptional();
  m_arichRaw.isOptional();
  m_eclRaw.isOptional();
  m_klmRaw.isOptional();
  m_trgRaw.isOptional();
}

void DAQMonitorModule::beginRun()
{
  h_nEvt->Reset();
  h_pxdSize->Reset();
  h_svdSize->Reset();
  h_cdcSize->Reset();
  h_topSize->Reset();
  h_arichSize->Reset();
  h_eclSize->Reset();
  h_klmSize->Reset();
  h_trgSize->Reset();
  h_hltSize->Reset();
  h_totalSize->Reset();
}

void DAQMonitorModule::event()
{
  // Total number of events: just fill the histogram with 1
  h_nEvt->Fill(1.0);

  // Since sizeof returns the size in bytes (B),
  // if we divide it by 1000 we obtain kilobytes (kB).

  // PXD
  int pxdSize{0};
  for (RawPXD& pxdRaw : m_pxdRaw)
    pxdSize += (pxdRaw.size()) * sizeof(unsigned int);
  h_pxdSize->Fill(static_cast<float>(pxdSize) / 1000.);

  // SVD
  int svdSize{0};
  for (RawSVD& svdRaw : m_svdRaw) // Loop over COPPERs
    svdSize += svdRaw.GetBlockNwords(0) * sizeof(unsigned int);
  h_svdSize->Fill(static_cast<float>(svdSize) / 1000.);

  // CDC
  int cdcSize{0};
  for (RawCDC& cdcRaw : m_cdcRaw) // Loop over COPPERs
    cdcSize += cdcRaw.GetBlockNwords(0) * sizeof(unsigned int);
  h_cdcSize->Fill(static_cast<float>(cdcSize) / 1000.);

  // TOP
  int topSize{0};
  for (RawTOP& topRaw : m_topRaw) // Loop over COPPERs
    topSize += topRaw.GetBlockNwords(0) * sizeof(unsigned int);
  h_topSize->Fill(static_cast<float>(topSize) / 1000.);

  // ARICH
  int arichSize{0};
  for (RawARICH& arichRaw : m_arichRaw) // Loop over COPPERs
    arichSize += arichRaw.GetBlockNwords(0) * sizeof(unsigned int);
  h_arichSize->Fill(static_cast<float>(arichSize) / 1000.);

  // ECL
  int eclSize{0};
  for (RawECL& eclRaw : m_eclRaw) // Loop over COPPERs
    eclSize += eclRaw.GetBlockNwords(0) * sizeof(unsigned int);
  h_eclSize->Fill(static_cast<float>(eclSize) / 1000.);

  // KLM
  int klmSize{0};
  for (RawKLM& klmRaw : m_klmRaw) // Loop over COPPERs
    klmSize += klmRaw.GetBlockNwords(0) * sizeof(unsigned int);
  h_klmSize->Fill(static_cast<float>(klmSize) / 1000.);

  // TRG
  int trgSize{0};
  for (RawTRG& trgRaw : m_trgRaw) // Loop over COPPERs
    trgSize += trgRaw.GetBlockNwords(0) * sizeof(unsigned int);
  h_trgSize->Fill(static_cast<float>(trgSize) / 1000.);

  // HLT size and total (HLT + PXD) size
  int hltSize = svdSize + cdcSize + topSize + arichSize + eclSize + klmSize + trgSize;
  h_hltSize->Fill(static_cast<float>(hltSize) / 1000.);
  int totalSize = pxdSize + hltSize;
  h_totalSize->Fill(static_cast<float>(totalSize) / 1000.);
}
