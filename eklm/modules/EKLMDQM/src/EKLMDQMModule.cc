/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/modules/EKLMDQM/EKLMDQMModule.h>

using namespace Belle2;

REG_MODULE(EKLMDQM)

EKLMDQMModule::EKLMDQMModule() : HistoModule()
{
  setDescription("EKLM data quality monitor.");
  setPropertyFlags(c_ParallelProcessingCertified);
  m_GeoDat = NULL;
}

EKLMDQMModule::~EKLMDQMModule()
{
}

void EKLMDQMModule::defineHisto()
{
  m_Sector = new TH1F("eklm_sector", "Sector number", 112, 0.5, 112.5);
}

void EKLMDQMModule::initialize()
{
  REG_HISTOGRAM
  m_Digits.isRequired();
  m_GeoDat = &(EKLM::GeometryData::Instance());
}

void EKLMDQMModule::beginRun()
{
}

void EKLMDQMModule::event()
{
  int i, n, sector;
  EKLMDigit* eklmDigit;
  n = m_Digits.getEntries();
  for (i = 0; i < n; i++) {
    eklmDigit = m_Digits[i];
    sector = m_GeoDat->sectorNumber(
               eklmDigit->getEndcap(), eklmDigit->getLayer(),
               eklmDigit->getSector());
    m_Sector->Fill(sector);
  }
}

void EKLMDQMModule::endRun()
{
}

void EKLMDQMModule::terminate()
{
  delete m_Sector;
}
