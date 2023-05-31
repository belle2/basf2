/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/modules/KLMScintillatorSimulator/KLMScintillatorSimulatorModule.h>

/* KLM headers. */
#include <klm/eklm/geometry/GeometryData.h>
#include <klm/simulation/ScintillatorSimulator.h>

/* Basf2 headers. */
#include <framework/gearbox/Unit.h>

/* ROOT headers. */
#include <TH1F.h>

using namespace Belle2;

REG_MODULE(KLMScintillatorSimulator)

static const char MemErr[] = "Memory allocation error.";

KLMScintillatorSimulatorModule::KLMScintillatorSimulatorModule() :
  Module(),
  m_fout(nullptr),
  m_SciSimPar(nullptr),
  m_hDir(nullptr),
  m_hRef(nullptr)
{
  setDescription("Standalone generation and studies of ADC output.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("OutputFile", m_out, "Output file.",
           std::string("KLMScintillatorSimulator.root"));
  addParam("Mode", m_mode, "Mode (\"Shape\" or \"Strips\").",
           std::string("Strips"));
}

KLMScintillatorSimulatorModule::~KLMScintillatorSimulatorModule()
{
}

void KLMScintillatorSimulatorModule::generateHistogram(
  const char* name, double l, double d, int npe)
{
  int j;
  double t, s;
  KLM::ScintillatorSimulator fe(m_SciSimPar, nullptr, 0, false);
  KLMTime* klmTime = &(KLMTime::Instance());
  klmTime->updateConstants();
  TH1F* h = nullptr;
  t = m_SciSimPar->getNDigitizations() *
      m_SciSimPar->getADCSamplingTDCPeriods() * klmTime->getTDCPeriod();
  try {
    h = new TH1F(name, "", m_SciSimPar->getNDigitizations(), 0, t);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  for (j = 0; j < m_SciSimPar->getNDigitizations(); j++) {
    m_hDir[j] = 0;
    m_hRef[j] = 0;
  }
  fe.generatePhotoelectrons(l, d, npe, 0, false);
  fe.generatePhotoelectrons(l, d, npe, 0, true);
  fe.fillSiPMOutput(m_hDir, true, false);
  fe.fillSiPMOutput(m_hRef, false, true);
  s = 0;
  for (j = 0; j < m_SciSimPar->getNDigitizations(); j++)
    s = s + m_hDir[j] + m_hRef[j];
  for (j = 1; j <= m_SciSimPar->getNDigitizations(); j++)
    h->SetBinContent(j, (m_hDir[j - 1] + m_hRef[j - 1]) / s);
  h->Write();
  delete h;
}

void KLMScintillatorSimulatorModule::initialize()
{
  /* cppcheck-suppress variableScope */
  char str[32];
  /* cppcheck-suppress variableScope */
  int i;
  /* cppcheck-suppress variableScope */
  double l;
  if (!m_SciSimParDatabase.isValid())
    B2FATAL("EKLM digitization parameters are not available.");
  m_SciSimPar = new KLMScintillatorDigitizationParameters(*m_SciSimParDatabase);
  const EKLM::GeometryData* geoDat = &EKLM::GeometryData::Instance();
  try {
    m_fout = new TFile(m_out.c_str(), "recreate");
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  m_hDir = (float*)malloc(m_SciSimPar->getNDigitizations() * sizeof(float));
  if (m_hDir == nullptr)
    B2FATAL(MemErr);
  m_hRef = (float*)malloc(m_SciSimPar->getNDigitizations() * sizeof(float));
  if (m_hRef == nullptr)
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
    m_SciSimPar->setMirrorReflectiveIndex(0);
    generateHistogram("FitShape", 0, 0, 1000000);
  } else
    B2FATAL("Unknown operation mode.");
  free(m_hDir);
  free(m_hRef);
  m_fout->Close();
  delete m_fout;
}

void KLMScintillatorSimulatorModule::beginRun()
{
}

void KLMScintillatorSimulatorModule::event()
{
}

void KLMScintillatorSimulatorModule::endRun()
{
}

void KLMScintillatorSimulatorModule::terminate()
{
  delete m_SciSimPar;
}

