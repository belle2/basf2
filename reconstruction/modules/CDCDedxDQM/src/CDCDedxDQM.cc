/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/modules/CDCDedxDQM/CDCDedxDQM.h>

using namespace Belle2;

REG_MODULE(CDCDedxDQM)

CDCDedxDQMModule::CDCDedxDQMModule() : HistoModule()
{

  setDescription("Make data quality monitoring plots for dE/dx calibration.");
}

CDCDedxDQMModule::~CDCDedxDQMModule() { }

void CDCDedxDQMModule::defineHisto()
{
  B2INFO("Creating a ROOT file for CDC dE/dx DQM...");

  // make sure the calibration constants are reasonable
  // run gains
  if (m_DBRunGain->getRunGain() == 0)
    B2ERROR("Run gain is zero!");

  // wire gains
  for (unsigned int i = 0; i < 14336; ++i) {
    if (m_DBWireGains->getWireGain(i) == 0)
      B2ERROR("Wire gain " << i << " is zero!");
  }

  // cosine correction (store the bin edges for extrapolation)
  const unsigned int ncosbins = m_DBCosineCor->getNBins();
  for (unsigned int i = 0; i < ncosbins; ++i) {
    double gain = m_DBCosineCor->getMean(2 / ncosbins - 1);
    if (gain == 0)
      B2ERROR("Cosine gain is zero...");
  }

  m_h_rungains = new TH1F("h_rungains", "h_rungains", 100, 0, 100);
  m_h_wiregains = new TH1F("h_wiregains", "h_wiregains", 100, 0, 2);
  m_h_cosinegains = new TH1F("h_cosinegains", "h_cosinegains", ncosbins, -1, 1);
}

void CDCDedxDQMModule::initialize()
{
  REG_HISTOGRAM   // required to register histograms to HistoManager
}

void CDCDedxDQMModule::beginRun()
{

  m_h_rungains->Fill(m_DBRunGain->getRunGain());

  for (unsigned int i = 0; i < 14336; ++i)
    m_h_wiregains->Fill(m_DBWireGains->getWireGain(i));

  int ncosbins = m_DBCosineCor->getNBins();
  for (unsigned int i = 0; i < ncosbins; ++i)
    m_h_cosinegains->SetBinContent(i, m_DBCosineCor->getMean(2 / ncosbins - 1));
}

void CDCDedxDQMModule::event()
{
}

void CDCDedxDQMModule::endRun()
{
}

void CDCDedxDQMModule::terminate()
{
}
