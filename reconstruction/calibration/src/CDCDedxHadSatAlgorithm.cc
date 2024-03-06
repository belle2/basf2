/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/calibration/CDCDedxHadSatAlgorithm.h>
#include <reconstruction/calibration/HadronPrep.h>
using namespace Belle2;

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
CDCDedxHadSatAlgorithm::CDCDedxHadSatAlgorithm() :
  CalibrationAlgorithm("CDCDedxHadronCollector"),
  m_dedxBins(250),
  m_dedxMin(0.0),
  m_dedxMax(2.5),
  m_bgBins(4),
  m_bgMin(600),
  m_bgMax(9600),
  m_cosBins(8),
  m_cosMin(0),
  m_cosMax(0.95),
  m_chiBins(250),
  m_chiMin(-10.0),
  m_chiMax(10.0),
  m_ismakePlots(true),
  m_isMerge(true),
  m_suffix("")
{
  // Set module properties
  setDescription("A calibration algorithm for CDC dE/dx injection time gain and reso");
}

//-----------------------------------------------------------------
//                 Run the calibration
//-----------------------------------------------------------------
CalibrationAlgorithm::EResult CDCDedxHadSatAlgorithm::calibrate()
{

  gROOT->SetBatch();
  getExpRunInfo();

  //existing hadron saturation payload for merging
  if (!m_DBHadronCor.isValid())
    B2FATAL("There is no valid payload for Hadron saturation");

  // Get data objects
  std::map<std::string, std::string> pdgMap = {
    // {"e+", "electron"},
    {"pi+", "pion"},
    {"K+", "kaon"},
    {"mu+", "muon"},
    {"p+", "proton"}
  };

  TFile* outfile = new TFile("widget_uncorrected_NoHSpar_2D.root", "RECREATE");
  for (auto& x : pdgMap) {

    // strip the name of the particle lists to make this work
    std::string pdg = x.second;

    auto tree = getObjectPtr<TTree>(Form("%s", pdg.data()));
    B2INFO(pdg.data() << " entries:  " << tree->GetEntries());
    std::cout << pdg.data() << " entries:  " << tree->GetEntries();
    if (!tree) return c_NotEnoughData;

    HadronPrep prep;
    prep.prepareSample(tree, outfile, Form("%s_noSat", m_suffix.data()), pdg.data(), m_ismakePlots, false);

  }
  outfile->Close();

  // TFile* outfile_def = new TFile("widget_corrected_defHSpar_2D.root", "RECREATE");

  // for (auto& x : pdgMap) {

  //   // strip the name of the particle lists to make this work
  //   std::string pdg = x.second;

  //   auto tree = getObjectPtr<TTree>(Form("%s", pdg.data()));

  //   HadronPrep prep;
  //   prep.prepareSample(tree, outfile_def, Form("%s_defSat", m_suffix.data()), pdg.data(), m_ismakePlots, true);
  // }

  // outfile_def->Close();

  HadronSaturation hadsat;
  hadsat.setParameters();
  hadsat.fillSample("widget_uncorrected_NoHSpar_2D.root");

  // // perform the hadron saturation correction
  hadsat.fitSaturation();

  // TFile* outfile_new = new TFile("widget_corrected_newHSpar_2D.root", "RECREATE");

  // for (auto& x : pdgMap) {

  //   // strip the name of the particle lists to make this work
  //   std::string pdg = x.second;

  //   auto tree = getObjectPtr<TTree>(Form("%s", pdg.data()));

  //   HadronPrep prep;
  //   prep.prepareSample(tree, outfile_new, Form("%s_newSat", m_suffix.data()), pdg.data(), m_ismakePlots, true);
  // }
  // outfile_new->Close();

  B2INFO("Saving calibration for: " << m_suffix << "");
  return c_OK;
}


//------------------------------------
void CDCDedxHadSatAlgorithm::getExpRunInfo()
{
  int cruns = 0;
  for (auto expRun : getRunList()) {
    if (cruns == 0)B2INFO("CDCDedxInjectTimeAlgorithm: start exp " << expRun.first << " and run " << expRun.second << "");
    cruns++;
  }

  const auto erStart = getRunList()[0];
  int estart = erStart.first;
  int rstart = erStart.second;

  updateDBObjPtrs(1, erStart.second, erStart.first);

  m_suffix = Form("e%dr%d", estart, rstart);
  B2INFO("tool exp = " << estart << ", run = " << rstart << ", m_suffix = " << m_suffix << "");
}
