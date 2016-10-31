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

#include <framework/gearbox/Unit.h>
#include <eklm/geometry/GeometryData.h>
#include <eklm/modules/EKLMADC/EKLMADCModule.h>

using namespace Belle2;

REG_MODULE(EKLMADC)

static const char MemErr[] = "Memory allocation error.";

EKLMADCModule::EKLMADCModule() : Module()
{
  setDescription("Standalone generation and studies of ADC output.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("OutputFile", m_out, "Output file.", std::string("EKLMADC.root"));
  addParam("Mode", m_mode, "Mode (\"Shape\" or \"Strips\").",
           std::string("Strips"));
}

EKLMADCModule::~EKLMADCModule()
{
}

void EKLMADCModule::generateHistogram(const char* name, double l, double d,
                                      int npe)
{
  int j;
  int gnpe;
  double t, s;
  EKLM::FiberAndElectronics fe(&m_digPar, NULL, false);
  TH1F* h = NULL;
  t = m_digPar.getNDigitizations() * m_digPar.getADCSamplingTime();
  try {
    h = new TH1F(name, "", m_digPar.getNDigitizations(), 0, t);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  for (j = 0; j < m_digPar.getNDigitizations(); j++) {
    m_hDir[j] = 0;
    m_hRef[j] = 0;
  }
  fe.fillSiPMOutput(l, d, npe, 0, false, m_hDir, &gnpe);
  fe.fillSiPMOutput(l, d, npe, 0, true, m_hRef, &gnpe);
  s = 0;
  for (j = 0; j < m_digPar.getNDigitizations(); j++)
    s = s + m_hDir[j] + m_hRef[j];
  for (j = 1; j <= m_digPar.getNDigitizations(); j++)
    h->SetBinContent(j, (m_hDir[j - 1] + m_hRef[j - 1]) / s);
  h->Write();
  delete h;
}

void EKLMADCModule::initialize()
{
  char str[32];
  int i;
  double l;
  EKLM::setDefDigitizationParams(&m_digPar);
  const EKLM::GeometryData* geoDat = &EKLM::GeometryData::Instance();
  try {
    m_fout = new TFile(m_out.c_str(), "recreate");
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  m_hDir = (float*)malloc(m_digPar.getNDigitizations() * sizeof(float));
  if (m_hDir == NULL)
    B2FATAL(MemErr);
  m_hRef = (float*)malloc(m_digPar.getNDigitizations() * sizeof(float));
  if (m_hRef == NULL)
    B2FATAL(MemErr);
  if (m_mode.compare("Strips") == 0) {
    for (i = 1; i <= geoDat->getNStrips(); i++) {
      l = geoDat->getStripLength(i) / CLHEP::mm * Unit::mm;
      snprintf(str, 32, "h%d_near", i);
      generateHistogram(str, l, 0, 10000);
      snprintf(str, 32, "h%d_far", i);
      generateHistogram(str, l, l, 10000);
    }
  } else if (m_mode.compare("Shape") == 0) {
    m_digPar.setMirrorReflectiveIndex(0);
    generateHistogram("FitShape", 0, 0, 1000000);
  } else
    B2FATAL("Unknown operation mode.");
  free(m_hDir);
  free(m_hRef);
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

