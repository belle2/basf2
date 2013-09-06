/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* External headers. */
#include <TH1F.h>

/* Belle2 headers. */
#include <framework/core/ModuleManager.h>
#include <eklm/modules/EKLMADC/EKLMADCModule.h>

using namespace Belle2;

REG_MODULE(EKLMADC)

static const char MemErr[] = "Memory allocation error.";

EKLMADCModule::EKLMADCModule() : Module()
{
  setDescription("Standalone generation and studies of ADC output.");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);
  addParam("OutputFile", m_out, "Output file.", std::string("EKLMADC.root"));
}

EKLMADCModule::~EKLMADCModule()
{
}

void EKLMADCModule::generateHistogram(char* name, double l, double d)
{
  int j;
  double t;
  TH1F* h;
  t = m_digPar.nDigitizations * m_digPar.ADCSamplingTime;
  try {
    h = new TH1F(name, "", m_digPar.nDigitizations, 0, t);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  EKLM::fillSiPMOutput(l, d, 10000, 0, false, &m_digPar, m_hDir);
  EKLM::fillSiPMOutput(l, d, 10000, 0, true, &m_digPar, m_hRef);
  for (j = 1; j <= m_digPar.nDigitizations; j++)
    h->SetBinContent(j, m_hDir[j - 1] + m_hRef[j - 1]);
  h->Write();
  delete h;
}

void EKLMADCModule::initialize()
{
  char str[32];
  int i;
  double l;
  m_geoDat.read();
  EKLM::setDefDigitizationParams(&m_digPar);
  m_digPar.debug = false;
  try {
    m_fout = new TFile(m_out.c_str(), "recreate");
    m_hDir = new float[m_digPar.nDigitizations];
    m_hRef = new float[m_digPar.nDigitizations];
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  for (i = 1; i <= 75; i++) {
    l = m_geoDat.getStripLength(i);
    snprintf(str, 32, "h%d_near", i);
    generateHistogram(str, l, 0);
    snprintf(str, 32, "h%d_far", i);
    generateHistogram(str, l, l);
  }
  delete m_hDir;
  delete m_hRef;
  m_fout->Close();
  delete m_fout;
}

void EKLMADCModule::beginRun()
{
}

void EKLMADCModule::event()
{
}

void EKLMADCModule::endRun()
{
}

void EKLMADCModule::terminate()
{
}

