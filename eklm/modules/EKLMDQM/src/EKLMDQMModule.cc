/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* External headers. */
#include <TDirectory.h>

/* Belle2 headers. */
#include <eklm/modules/EKLMDQM/EKLMDQMModule.h>

using namespace Belle2;

REG_MODULE(EKLMDQM)

EKLMDQMModule::EKLMDQMModule() : HistoModule()
{
  setDescription("EKLM data quality monitor.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("histogramDirectoryName", m_HistogramDirectoryName,
           "Directory for EKLM DQM histograms in ROOT file.",
           std::string("EKLM"));
  m_Elements = NULL;
  m_Sector = NULL;
}

EKLMDQMModule::~EKLMDQMModule()
{
}

void EKLMDQMModule::defineHisto()
{
  TDirectory* oldDirectory, *newDirectory;
  oldDirectory = gDirectory;
  newDirectory = oldDirectory->mkdir(m_HistogramDirectoryName.c_str());
  newDirectory->cd();
  m_Sector = new TH1F("sector", "Sector number", 104, 0.5, 104.5);
  m_Time = new TH1F("time", "Hit time", 50, 0, 50);
  oldDirectory->cd();
}

void EKLMDQMModule::initialize()
{
  REG_HISTOGRAM
  m_Digits.isRequired();
  m_Elements = &(EKLM::ElementNumbersSingleton::Instance());
}

void EKLMDQMModule::beginRun()
{
  m_Sector->Reset();
  m_Time->Reset();
}

void EKLMDQMModule::event()
{
  int i, n, sector;
  EKLMDigit* eklmDigit;
  n = m_Digits.getEntries();
  for (i = 0; i < n; i++) {
    eklmDigit = m_Digits[i];
    /*
     * Reject digits that are below the threshold (such digits may appear
     * for simulated events).
     */
    if (!eklmDigit->isGood())
      continue;
    sector = m_Elements->sectorNumber(
               eklmDigit->getEndcap(), eklmDigit->getLayer(),
               eklmDigit->getSector());
    m_Sector->Fill(sector);
    m_Time->Fill(eklmDigit->getTime());
  }
}

void EKLMDQMModule::endRun()
{
}

void EKLMDQMModule::terminate()
{
}
