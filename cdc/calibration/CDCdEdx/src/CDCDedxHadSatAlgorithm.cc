/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <cdc/calibration/CDCdEdx/CDCDedxHadSatAlgorithm.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
CDCDedxHadSatAlgorithm::CDCDedxHadSatAlgorithm() :
  CalibrationAlgorithm("CDCDedxHadronCollector"),
  m_ismakePlots(true),
  m_suffix("")
{
  // Set module properties
  setDescription("A calibration algorithm for CDC dE/dx hadron saturation");
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

  // particle list
  std::vector<std::string> particles = {"pion", "kaon", "muon", "proton"};

  // check we have enough data
  for (int i = 0; i < int(particles.size()); i++) {
    std::string p = particles[i];
    auto tree = getObjectPtr<TTree>(Form("%s", p.data()));
    if (!tree) return c_NotEnoughData;
  }

  gSystem->Exec("mkdir -p plots/HadronSat");

  // save plots with no had saturation correction
  std::string filename = "widget_uncorrected_NoHSpar_2D.root";
  prepareSample(particles, filename, "noSat", false);

  // save plots with default had saturation correction
  filename = "widget_corrected_defHSpar_2D.root";
  prepareSample(particles, filename, "defSat", true);

  double par[5];
  for (int i = 0; i < 5; ++i)  par[i] = m_DBHadronCor->getHadronPar(i);

  HadronSaturation hadsat(par[0], par[1], par[2], par[3], par[4], m_cosBins);

  hadsat.fillSample("widget_uncorrected_NoHSpar_2D.root");

  // perform the hadron saturation correction
  hadsat.fitSaturation();

  // save plots with new had saturation correction
  filename = "widget_corrected_newHSpar_2D.root";
  prepareSample(particles, filename, "newSat", true);

  B2INFO("Saving calibration for: " << m_suffix << "");
  createPayload();

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

//--------------------------
void CDCDedxHadSatAlgorithm::createPayload()
{

  std::ifstream fin("sat-pars.fit.txt");
  if (!fin.good()) B2FATAL("\tWARNING: CANNOT FIND sat-pars.fit.txt!");

  double hadsatpars;
  std::vector<double> v_hadsatpars;

  B2INFO("\t --> Hadron saturation parameters");
  for (int i = 0; i < 5; ++i) {
    fin >> hadsatpars;
    v_hadsatpars.push_back(hadsatpars);
    B2INFO("\t\t (" << i << ")" << v_hadsatpars[i]);
  }
  fin.close();

  B2INFO("dE/dx Calibration done for " << v_hadsatpars.size() << " CDC Hadron saturation");
  CDCDedxHadronCor* gains = new CDCDedxHadronCor(0, v_hadsatpars);
  saveCalibration(gains, "CDCDedxHadronCor");

}

void CDCDedxHadSatAlgorithm::prepareSample(std::vector< std::string >& particles, const std::string& filename,
                                           const std::string& sfx,
                                           bool correct)
{
  TFile* outfile = new TFile(Form("%s", filename.data()), "RECREATE");

  for (int i = 0; i < int(particles.size()); i++) {

    // strip the name of the particle lists to make this work
    std::string p = particles[i];
    auto tree = getObjectPtr<TTree>(Form("%s", p.data()));

    HadronPrep prep(m_bgpar[p][0], m_bgpar[p][1], m_bgpar[p][2], m_cosBins, m_cosMin, m_cosMax, m_cut);
    prep.prepareSample(tree, outfile, Form("%s_%s", m_suffix.data(), sfx.data()), p.data(), m_ismakePlots, correct);

  }
  outfile->Close();
}