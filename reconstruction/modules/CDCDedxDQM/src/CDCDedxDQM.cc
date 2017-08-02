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

  // prepare the wire gain constants
  for (unsigned int i = 0; i < 14336; ++i) {
    if (m_DBWireGains->getWireGain(i) == 0)
      B2ERROR("Wire gain " << i << " is zero!");
  }

  // prepare the electron saturation constants (key is low edge of bin)
  std::vector<double> binedges = m_DBCosine->getCosThetaBins();
  for (unsigned int i = 0; i < binedges.size(); ++i) {
    B2INFO(binedges[i] << " : " << m_DBCosine->getMean(binedges[i]));
    double gain = m_DBCosine->getMean(binedges[i]) / m_DBRunGain->getRunGain();
    if (gain == 0) {
      B2WARNING("Cosine gain is zero...");
      gain = 1;
    }
    m_cosineGains[binedges[i]] = gain;
  }

  const int ncgains = m_cosineGains.size();
  m_h_rungains = new TH1F("h_rungains", "h_rungains", 100, 0, 100);
  m_h_wiregains = new TH1F("h_wiregains", "h_wiregains", 14336, 0, 14336);
  m_h_cosinegains = new TH1F("h_cosinegains", "h_cosinegains", ncgains, -1, 1);
}

void CDCDedxDQMModule::initialize()
{
  REG_HISTOGRAM   // required to register histograms to HistoManager
}

void CDCDedxDQMModule::beginRun()
{

  m_h_rungains->Fill(m_DBRunGain->getRunGain());

  int bin = 0;
  for (unsigned int i = 0; i < 14336; ++i)
    m_h_wiregains->SetBinContent(i, m_DBWireGains->getWireGain(i));

  bin = 0;
  for (auto& cosine : m_cosineGains)
    m_h_cosinegains->SetBinContent(bin++, cosine.second);
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
