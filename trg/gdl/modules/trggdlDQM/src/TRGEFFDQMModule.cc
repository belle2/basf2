/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <trg/gdl/modules/trggdlDQM/TRGEFFDQMModule.h>

// Dataobject classes
#include <TF1.h>
#include <Math/Vector3D.h>
#include <TVector3.h>
#include <TDirectory.h>
#include <fstream>
#include <math.h>
#include <mdst/dataobjects/HitPatternCDC.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

REG_MODULE(TRGEFFDQM);

TRGEFFDQMModule::TRGEFFDQMModule() : HistoModule()
{
  // set module description (e.g. insert text)
  setDescription("Make kinematics dependent efficiency plot");
  setPropertyFlags(c_ParallelProcessingCertified);
}

TRGEFFDQMModule::~TRGEFFDQMModule()
{
}

void TRGEFFDQMModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir("TRGEFF");
  oldDir->cd("TRGEFF");

  m_hPhi                         = new TH1F("hPhi", "", 36, -180.0, 180.0);
  m_hPhi_psnecl                  = new TH1F("hPhi_psnecl", "", 36, -180.0, 180.0);
  m_hPhi_psnecl_ftdf             = new TH1F("hPhi_psnecl_ftdf", "", 36, -180.0, 180.0);

  m_hPt                          = new TH1F("hPt", "", 50, 0, 5);
  m_hPt_psnecl                   = new TH1F("hPt_psnecl", "", 50, 0, 5);
  m_hPt_psnecl_ftdf              = new TH1F("hPt_psnecl_ftdf", "", 50, 0, 5);

  m_nobha_f_phi                  = new TH1F("nobha_f_phi", "", 36, -180., 180.);
  m_nobha_f_phi_psnecl           = new TH1F("nobha_f_phi_psnecl", "", 36, -180., 180.);
  m_nobha_f_phi_psnecl_ftdf      = new TH1F("nobha_f_phi_psnecl_ftdf", "", 36, -180., 180.);

  m_nobha_hPt                    = new TH1F("nobha_hPt", "", 50, 0, 5);
  m_nobha_hPt_psnecl             = new TH1F("nobha_hPt_psnecl", "", 50, 0, 5);
  m_nobha_hPt_psnecl_ftdf        = new TH1F("nobha_hPt_psnecl_ftdf", "", 50, 0, 5);

  m_hP3_y                        = new TH1F("hP3_y", "", 50, 0, 5);
  m_hP3_y_psnecl                 = new TH1F("hP3_y_psnecl", "", 50, 0, 5);
  m_hP3_y_psnecl_ftdf            = new TH1F("hP3_y_psnecl_ftdf", "", 50, 0, 5);

  m_hP3_z                        = new TH1F("hP3_z", "", 50, 0, 5);
  m_hP3_z_psnecl                 = new TH1F("hP3_z_psnecl", "", 50, 0, 5);
  m_hP3_z_psnecl_ftdf            = new TH1F("hP3_z_psnecl_ftdf", "", 50, 0, 5);

  m_nobha_hP3_y                  = new TH1F("nobha_hP3_y", "", 50, 0, 5);
  m_nobha_hP3_y_psnecl           = new TH1F("nobha_hP3_y_psnecl", "", 50, 0, 5);
  m_nobha_hP3_y_psnecl_ftdf      = new TH1F("nobha_hP3_y_psnecl_ftdf", "", 50, 0, 5);

  m_nobha_hP3_z                  = new TH1F("nobha_hP3_z", "", 50, 0, 5);
  m_nobha_hP3_z_psnecl           = new TH1F("nobha_hP3_z_psnecl", "", 50, 0, 5);
  m_nobha_hP3_z_psnecl_ftdf      = new TH1F("nobha_hP3_z_psnecl_ftdf", "", 50, 0, 5);

  m_nobha_phi_y                  = new TH1F("nobha_phi_y", "", 36, -180., 180.);
  m_nobha_phi_y_psnecl           = new TH1F("nobha_phi_y_psnecl", "", 36, -180., 180.);
  m_nobha_phi_y_psnecl_ftdf      = new TH1F("nobha_phi_y_psnecl_ftdf", "", 36, -180., 180.);

  m_nobha_phi_z                  = new TH1F("nobha_phi_z", "", 36, -180., 180.);
  m_nobha_phi_z_psnecl           = new TH1F("nobha_phi_z_psnecl", "", 36, -180., 180.);
  m_nobha_phi_z_psnecl_ftdf      = new TH1F("nobha_phi_z_psnecl_ftdf", "", 36, -180., 180.);

  m_fyo_dphi                     = new TH1F("fyo_dphi", "", 18, 0., 180.);
  m_fyo_dphi_psnecl              = new TH1F("fyo_dphi_psnecl", "", 18, 0., 180.);
  m_fyo_dphi_psnecl_ftdf         = new TH1F("fyo_dphi_psnecl_ftdf", "", 18, 0., 180.);

  m_nobha_fyo_dphi               = new TH1F("nobha_fyo_dphi", "", 18, 0., 180.);
  m_nobha_fyo_dphi_psnecl        = new TH1F("nobha_fyo_dphi_psnecl", "", 18, 0., 180.);
  m_nobha_fyo_dphi_psnecl_ftdf   = new TH1F("nobha_fyo_dphi_psnecl_ftdf", "", 18, 0., 180.);

  m_stt_phi                      = new TH1F("stt_phi", "", 36, -180., 180.);
  m_stt_phi_psnecl               = new TH1F("stt_phi_psnecl", "", 36, -180., 180.);
  m_stt_phi_psnecl_ftdf          = new TH1F("stt_phi_psnecl_ftdf", "", 36, -180., 180.);

  m_stt_P3                       = new TH1F("stt_P3", "", 50, 0, 5);
  m_stt_P3_psnecl                = new TH1F("stt_P3_psnecl", "", 50, 0, 5);
  m_stt_P3_psnecl_ftdf           = new TH1F("stt_P3_psnecl_ftdf", "", 50, 0, 5);

  m_stt_theta                    = new TH1F("stt_theta", "", 18, 0, 180);
  m_stt_theta_psnecl             = new TH1F("stt_theta_psnecl", "", 18, 0, 180);
  m_stt_theta_psnecl_ftdf        = new TH1F("stt_theta_psnecl_ftdf", "", 18, 0, 180);

  m_nobha_stt_phi                = new TH1F("nobha_stt_phi", "", 36, -180., 180.);
  m_nobha_stt_phi_psnecl         = new TH1F("nobha_stt_phi_psnecl", "", 36, -180., 180.);
  m_nobha_stt_phi_psnecl_ftdf    = new TH1F("nobha_stt_phi_psnecl_ftdf", "", 36, -180., 180.);

  m_nobha_stt_P3                 = new TH1F("nobha_stt_P3", "", 50, 0, 5);
  m_nobha_stt_P3_psnecl          = new TH1F("nobha_stt_P3_psnecl", "", 50, 0, 5);
  m_nobha_stt_P3_psnecl_ftdf     = new TH1F("nobha_stt_P3_psnecl_ftdf", "", 50, 0, 5);

  m_nobha_stt_theta              = new TH1F("nobha_stt_theta", "", 18, 0, 180);
  m_nobha_stt_theta_psnecl       = new TH1F("nobha_stt_theta_psnecl", "", 18, 0, 180);
  m_nobha_stt_theta_psnecl_ftdf  = new TH1F("nobha_stt_theta_psnecl_ftdf", "", 18, 0, 180);

  m_hie_E                        = new TH1F("hie_E", "", 60, 0, 12);
  m_hie_E_psnecl                 = new TH1F("hie_E_psnecl", "", 60, 0, 12);
  m_hie_E_psnecl_ftdf            = new TH1F("hie_E_psnecl_ftdf", "", 60, 0, 12);

  m_nobha_hie_E                  = new TH1F("nobha_hie_E", "", 60, 0, 12);
  m_nobha_hie_E_psnecl           = new TH1F("nobha_hie_E_psnecl", "", 60, 0, 12);
  m_nobha_hie_E_psnecl_ftdf      = new TH1F("nobha_hie_E_psnecl_ftdf", "", 60, 0, 12);

  m_ecltiming_E                  = new TH1F("ecltiming_E", "", 60, 0, 12);
  m_ecltiming_E_psnecl           = new TH1F("ecltiming_E_psnecl", "", 60, 0, 12);
  m_ecltiming_E_psnecl_ftdf      = new TH1F("ecltiming_E_psnecl_ftdf", "", 60, 0, 12);

  m_ecltiming_theta              = new TH1F("ecltiming_theta", "", 18, 0, 180);
  m_ecltiming_theta_psnecl       = new TH1F("ecltiming_theta_psnecl", "", 18, 0, 180);
  m_ecltiming_theta_psnecl_ftdf  = new TH1F("ecltiming_theta_psnecl_ftdf", "", 18, 0, 180);

  m_ecltiming_phi                = new TH1F("ecltiming_phi", "", 36, -180., 180.);
  m_ecltiming_phi_psnecl         = new TH1F("ecltiming_phi_psnecl", "", 36, -180., 180.);
  m_ecltiming_phi_psnecl_ftdf    = new TH1F("ecltiming_phi_psnecl_ftdf", "", 36, -180., 180.);

  m_klmhit_phi                   = new TH1F("klmhit_phi", "", 18, -180., 180.);
  m_klmhit_phi_psnecl            = new TH1F("klmhit_phi_psnecl", "", 18, -180., 180.);
  m_klmhit_phi_psnecl_ftdf       = new TH1F("klmhit_phi_psnecl_ftdf", "", 18, -180., 180.);

  m_klmhit_theta                 = new TH1F("klmhit_theta", "", 18, 0, 180);
  m_klmhit_theta_psnecl          = new TH1F("klmhit_theta_psnecl", "", 18, 0, 180);
  m_klmhit_theta_psnecl_ftdf     = new TH1F("klmhit_theta_psnecl_ftdf", "", 18, 0, 180);

  m_eklmhit_phi                  = new TH1F("eklmhit_phi", "", 18, -180., 180.);
  m_eklmhit_phi_psnecl           = new TH1F("eklmhit_phi_psnecl", "", 18, -180., 180.);
  m_eklmhit_phi_psnecl_ftdf      = new TH1F("eklmhit_phi_psnecl_ftdf", "", 18, -180., 180.);

  m_eklmhit_theta                = new TH1F("eklmhit_theta", "", 18, 0, 180);
  m_eklmhit_theta_psnecl         = new TH1F("eklmhit_theta_psnecl", "", 18, 0, 180);
  m_eklmhit_theta_psnecl_ftdf    = new TH1F("eklmhit_theta_psnecl_ftdf", "", 18, 0, 180);







  oldDir->cd();
}

void TRGEFFDQMModule::initialize()
{
  REG_HISTOGRAM

  if (!m_Tracks.isOptional()) {
    B2WARNING("Missing Tracks array");
    return;
  }
  if (!m_ECLClusters.isOptional()) {
    B2WARNING("Missing ECLCLuster array");
    return;
  }
  if (!m_KLMClusters.isOptional()) {
    B2WARNING("Missing KLMCLuster array");
    return;
  }
  if (!m_trgSummary.isOptional()) {
    B2WARNING("Missing TRGSummary");
    return;
  }
}

void TRGEFFDQMModule::beginRun()
{
  if (!m_RecoTracks.isOptional()) {
    B2DEBUG(22, "Missing recoTracks array in beginRun() ");
    return;
  }

}

void TRGEFFDQMModule::event()
{
  if (!m_trgSummary.isValid()) {
    B2WARNING("TRGSummary object not available but require to estimate trg efficiency");
    return;
  }

  if (!m_TrgResult.isValid()) {
    B2WARNING("SoftwareTriggerResult object not available but require to select bhabha/mumu/hadron events skim");
    return;
  }

  const std::map<std::string, int>& fresults = m_TrgResult->getResults();
  if ((fresults.find("software_trigger_cut&skim&accept_bhabha") == fresults.end())
      || (fresults.find("software_trigger_cut&skim&accept_hadron") == fresults.end())) {
    B2WARNING("TRGEFFDQMModule: Can't find required bhabha or mumu or hadron trigger identifier");
    return;
  }


  /*///////////////////////////////////////////////////////////////////
  ///////////- - - - ECL TRG - - - - -////////////////////////////////
  ///////////////////////////////////////////////////////////////////*/

  // calculate the total energy
  double E_ecl_all = 0;       // the ECL total energy
  double E_ecl_hie =
    0;       // the ECL total energy in the thetaID range 2<=ThetaID<=15 (corresponds to 22.49<=theta<=126.80) for ehigh bit
  for (const auto& test_b2eclcluster : m_ECLClusters) {
    if (!(test_b2eclcluster.hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons))) continue;
    double energy = test_b2eclcluster.getEnergyRaw();
    double theta  = test_b2eclcluster.getTheta() / Unit::deg;

    if (energy < 0.1) continue;

    E_ecl_all = E_ecl_all + energy;
    if (theta >= 22.49 && theta <= 126.8) {
      E_ecl_hie = E_ecl_hie + energy;
    }
  }

  bool trg_hie_psncdc       = 0;  // for ECL energy trigger, for hie
  bool trg_hie_Eecl         = 0;  // for ECL energy trigger, for hie
  bool trg_ecltiming_psncdc = 0;  // for ECL energy trigger, for ecltiming
  bool trg_ecltiming_Eecl   = 0;  // for ECL energy trigger. for ecltiming
  bool trg_nobha_hie_Eecl   = 0;

  for (const auto& b2eclcluster : m_ECLClusters) {
    if (!(b2eclcluster.hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons))) continue;
    double phi    = b2eclcluster.getPhi() / Unit::deg;
    double theta  = b2eclcluster.getTheta() / Unit::deg;
    double energy = b2eclcluster.getEnergyRaw();

    if (energy < 0.1) continue;

    bool trg_psncdc    = m_trgSummary->testPsnm("ffy") || m_trgSummary->testPsnm("fyo") || m_trgSummary->testPsnm("stt");
    bool trg_hie       = m_trgSummary->testFtdl("hie");
    bool trg_hie_nobha = m_trgSummary->testInput("ehigh");   // remove the bha_veto for hie bit
    bool trg_ecltiming = m_trgSummary->testFtdl("ecltiming");

    m_ecltiming_theta->Fill(theta);
    m_ecltiming_phi->Fill(phi);

    if (trg_psncdc) {
      trg_hie_psncdc   = 1;
      trg_ecltiming_psncdc = 1;

      m_ecltiming_theta_psnecl->Fill(theta);
      m_ecltiming_phi_psnecl->Fill(phi);
    }

    if (trg_psncdc && trg_hie) {
      trg_hie_Eecl = 1;
    }
    if (trg_psncdc && trg_hie_nobha) {
      trg_nobha_hie_Eecl = 1;
    }
    if (trg_psncdc && trg_ecltiming) {
      trg_ecltiming_Eecl = 1;

      m_ecltiming_theta_psnecl_ftdf->Fill(theta);
      m_ecltiming_phi_psnecl_ftdf->Fill(phi);
    }
  }

  // sum of the ECL total energy
  m_hie_E->Fill(E_ecl_hie);
  m_nobha_hie_E->Fill(E_ecl_hie);
  if (trg_hie_psncdc) {
    m_hie_E_psnecl->Fill(E_ecl_hie);
    m_nobha_hie_E_psnecl->Fill(E_ecl_hie);
  }
  if (trg_hie_Eecl) {
    m_hie_E_psnecl_ftdf->Fill(E_ecl_hie);
  }
  if (trg_nobha_hie_Eecl) {
    m_nobha_hie_E_psnecl_ftdf->Fill(E_ecl_hie);
  }

  m_ecltiming_E->Fill(E_ecl_all);
  if (trg_ecltiming_psncdc) {
    m_ecltiming_E_psnecl->Fill(E_ecl_all);
  }
  if (trg_ecltiming_Eecl) {
    m_ecltiming_E_psnecl_ftdf->Fill(E_ecl_all);
  }






  /*///////////////////////////////////////////////////////////////////
  ///////////- - - - KLM TRG - - - - -////////////////////////////////
  ///////////////////////////////////////////////////////////////////*/
  for (const auto& b2klmcluster : m_KLMClusters) {
    int nlayer = b2klmcluster.getLayers();
    // ROOT::Math::XYZVector position = b2klmcluster.getClusterPosition();

    if (nlayer <= 6)
      continue;

    // double p3        = b2klmcluster.getMomentum().R();  // 3-momentum
    double theta     = b2klmcluster.getMomentum().Theta() / Unit::deg;
    double phiDegree = b2klmcluster.getMomentum().Phi() / Unit::deg;

    bool trg_KLMecl  = m_trgSummary->testPsnm("hie")        || m_trgSummary->testPsnm("c4")   || m_trgSummary->testPsnm("eclmumu") ||
                       m_trgSummary->testPsnm("lml1")       || m_trgSummary->testPsnm("lml2") || m_trgSummary->testPsnm("lml6")    ||
                       m_trgSummary->testPsnm("lml7")       || m_trgSummary->testPsnm("lml8") || m_trgSummary->testPsnm("lml9")    ||
                       m_trgSummary->testPsnm("lml10");
    // these trigger flags are not available in older data
    try {
      trg_KLMecl |= m_trgSummary->testPsnm("ecltaub2b3") || m_trgSummary->testPsnm("hie4");
    } catch (const std::exception&) {
    }


    bool trg_klmhit  = m_trgSummary->testFtdl("klmhit");
    bool trg_eklmhit = m_trgSummary->testFtdl("eklmhit");

    m_klmhit_phi->Fill(phiDegree);
    m_klmhit_theta->Fill(theta);
    m_eklmhit_phi->Fill(phiDegree);
    m_eklmhit_theta->Fill(theta);

    if (trg_KLMecl) {
      // m_klmhit_phi_psnecl->Fill(phiDegree);
      m_klmhit_theta_psnecl->Fill(theta);
      // m_eklmhit_phi_psnecl->Fill(phiDegree);
      m_eklmhit_theta_psnecl->Fill(theta);
    }
    if (trg_KLMecl && trg_klmhit) {
      // m_klmhit_phi_psnecl_ftdf->Fill(phiDegree);
      m_klmhit_theta_psnecl_ftdf->Fill(theta);
    }
    if (trg_KLMecl && trg_eklmhit) {
      // m_eklmhit_phi_psnecl_ftdf->Fill(phiDegree);
      m_eklmhit_theta_psnecl_ftdf->Fill(theta);
    }

    // add theta cut for phi distribution, since the efficiency at some theta range is very low.
    if (trg_KLMecl && theta > 50 && theta < 120) {
      m_klmhit_phi_psnecl->Fill(phiDegree);
    }
    if (trg_KLMecl && ((theta > 20 && theta < 40) || (theta > 120 && theta < 160))) {
      m_eklmhit_phi_psnecl->Fill(phiDegree);
    }
    if (trg_KLMecl && trg_klmhit && theta > 50 && theta < 120) {
      m_klmhit_phi_psnecl_ftdf->Fill(phiDegree);
    }
    if (trg_KLMecl && trg_eklmhit && ((theta > 20 && theta < 40) || (theta > 120 && theta < 160))) {
      m_eklmhit_phi_psnecl_ftdf->Fill(phiDegree);
    }
  }



  /*///////////////////////////////////////////////////////////////////
  ///////////- - - - CDC TRG - - - - -////////////////////////////////
  ///////////////////////////////////////////////////////////////////*/


  vector<double> p_stt_P3_psnecl_ftdf, p_stt_P3_psnecl, p_stt_P3, phi_fyo_dphi, phi_fyo_dphi_psnecl, phi_fyo_dphi_psnecl_ftdf ;
  p_stt_P3_psnecl_ftdf.clear();
  p_stt_P3_psnecl.clear();
  p_stt_P3.clear();

  phi_fyo_dphi.clear();
  phi_fyo_dphi_psnecl.clear();
  phi_fyo_dphi_psnecl_ftdf.clear();

  vector<double> p_nobha_stt_P3_psnecl_ftdf, p_nobha_stt_P3_psnecl, p_nobha_stt_P3, phi_nobha_fyo_dphi, phi_nobha_fyo_dphi_psnecl,
         phi_nobha_fyo_dphi_psnecl_ftdf ;
  p_nobha_stt_P3_psnecl_ftdf.clear();
  p_nobha_stt_P3_psnecl.clear();
  p_nobha_stt_P3.clear();

  phi_nobha_fyo_dphi.clear();
  phi_nobha_fyo_dphi_psnecl.clear();
  phi_nobha_fyo_dphi_psnecl_ftdf.clear();

  int nitrack = 0;    // the i track of the m_Tracks
  for (const auto& b2track : m_Tracks) {
    const Belle2::TrackFitResult* fitresult = b2track.getTrackFitResultWithClosestMass(Const::pion);
    if (!fitresult) {
      B2WARNING("No track fit result found.");
      nitrack++;
      continue;
    }

    // require high NDF track
    int ndf = fitresult->getNDF();
    if (ndf < 20) {
      nitrack++;
      continue;
    }

    // IP tracks at barrel
    if (fabs(fitresult->getD0()) < 1.0 && fabs(fitresult->getZ0()) < 1.0 && fitresult->getHitPatternCDC().getLastLayer() > 50
        && fitresult->getHitPatternCDC().getFirstLayer() < 5) {
      double phiDegree = fitresult->getPhi() / Unit::deg;
      double pt        = fitresult->getTransverseMomentum();
      double p3        = fitresult->getMomentum().R();  // 3-momentum
      double theta     = fitresult->getMomentum().Theta() / Unit::deg;

      bool trg_psnecl  = m_trgSummary->testPsnm("hie")        || m_trgSummary->testPsnm("c4")   || m_trgSummary->testPsnm("eclmumu") ||
                         m_trgSummary->testPsnm("lml1")       || m_trgSummary->testPsnm("lml2") || m_trgSummary->testPsnm("lml6")    ||
                         m_trgSummary->testPsnm("lml7")       || m_trgSummary->testPsnm("lml8") || m_trgSummary->testPsnm("lml9")    ||
                         m_trgSummary->testPsnm("lml10");
      // these trigger flags are not available in older data
      try {
        trg_psnecl |= m_trgSummary->testPsnm("ecltaub2b3") || m_trgSummary->testPsnm("hie4");
      } catch (const std::exception&) {
      }

      bool trg_ftdf    = m_trgSummary->testFtdl("f");

      // for f bit, reomove the Bhabha_veto
      bool trg_itdt2 = (m_trgSummary->testInput("t2_0") || m_trgSummary->testInput("t2_1") || m_trgSummary->testInput("t2_2")
                        || m_trgSummary->testInput("t2_3"));

      // for z, reomove the Bhabha_veto
      bool trg_itdt3 = (m_trgSummary->testInput("t3_0") || m_trgSummary->testInput("t3_1") || m_trgSummary->testInput("t3_2")
                        || m_trgSummary->testInput("t3_3"));

      // for y, reomove the Bhabha_veto
      bool trg_itdt4 = (m_trgSummary->testInput("ty_0") || m_trgSummary->testInput("ty_1") || m_trgSummary->testInput("ty_2")
                        || m_trgSummary->testInput("ty_3"));

      // (t3>0 and !bhaveto and !veto)  for z
      bool trg_ftdz = m_trgSummary->testFtdl("z");

      // (ty>0 and !bhaveto and !veto)  for y
      bool trg_ftdy = m_trgSummary->testFtdl("y");

      // typ and !bha veto and !veto
      bool trg_stt  = m_trgSummary->testFtdl("stt");

      // remove bha_veto
      bool trg_stt_nobha  = m_trgSummary->testInput("typ") ;


      // require pt > 0.3 GeV
      if (pt > 0.3) {
        m_hPhi->Fill(phiDegree);
        if (trg_psnecl) {
          m_hPhi_psnecl->Fill(phiDegree);
        }
        if (trg_psnecl && trg_ftdf) {
          m_hPhi_psnecl_ftdf->Fill(phiDegree);
        }
      }

      m_hPt->Fill(pt);
      m_nobha_hPt->Fill(pt);
      m_nobha_f_phi->Fill(phiDegree);

      m_hP3_z->Fill(p3);
      m_hP3_y->Fill(p3);

      m_nobha_hP3_z->Fill(p3);
      m_nobha_hP3_y->Fill(p3);

      m_nobha_phi_z->Fill(phiDegree);
      m_nobha_phi_y->Fill(phiDegree);

      m_stt_theta->Fill(theta);
      m_stt_phi->Fill(phiDegree);
      p_stt_P3.push_back(p3);

      m_nobha_stt_theta->Fill(theta);
      m_nobha_stt_phi->Fill(phiDegree);
      p_nobha_stt_P3.push_back(p3);


      if (trg_psnecl) {
        m_hPt_psnecl->Fill(pt);
        m_nobha_hPt_psnecl->Fill(pt);
        m_nobha_f_phi_psnecl->Fill(phiDegree);

        m_hP3_z_psnecl->Fill(p3);           // for z bit
        m_hP3_y_psnecl->Fill(p3);           // for y bit

        m_nobha_hP3_z_psnecl->Fill(p3);     // remove bhabha veto for z bit
        m_nobha_hP3_y_psnecl->Fill(p3);     // remove bhabha veto for y bit

        m_nobha_phi_z_psnecl->Fill(phiDegree);
        m_nobha_phi_y_psnecl->Fill(phiDegree);

        m_stt_phi_psnecl->Fill(phiDegree);
        p_stt_P3_psnecl.push_back(p3);
        m_stt_theta_psnecl->Fill(theta);

        m_nobha_stt_phi_psnecl->Fill(phiDegree);
        p_nobha_stt_P3_psnecl.push_back(p3);
        m_nobha_stt_theta_psnecl->Fill(theta);
      }

      if (trg_psnecl && trg_ftdf) {
        m_hPt_psnecl_ftdf->Fill(pt);
      }
      if (trg_psnecl && trg_itdt2) {
        m_nobha_hPt_psnecl_ftdf->Fill(pt);
        m_nobha_f_phi_psnecl_ftdf->Fill(phiDegree);
      }

      if (trg_psnecl && trg_ftdz) {
        m_hP3_z_psnecl_ftdf->Fill(p3);
      }
      if (trg_psnecl && trg_ftdy) {
        m_hP3_y_psnecl_ftdf->Fill(p3);
      }
      if (trg_psnecl && trg_itdt3) {
        m_nobha_hP3_z_psnecl_ftdf->Fill(p3);
        m_nobha_phi_z_psnecl_ftdf->Fill(phiDegree);
      }
      if (trg_psnecl && trg_itdt4) {
        m_nobha_hP3_y_psnecl_ftdf->Fill(p3);
        m_nobha_phi_y_psnecl_ftdf->Fill(phiDegree);
      }

      if (trg_psnecl && trg_stt) {
        m_stt_phi_psnecl_ftdf->Fill(phiDegree);
        p_stt_P3_psnecl_ftdf.push_back(p3);
        m_stt_theta_psnecl_ftdf->Fill(theta);
      }
      if (trg_psnecl && trg_stt_nobha) {
        m_nobha_stt_phi_psnecl_ftdf->Fill(phiDegree);
        p_nobha_stt_P3_psnecl_ftdf.push_back(p3);
        m_nobha_stt_theta_psnecl_ftdf->Fill(theta);
      }


      // the following is for fyo \deleta_phi
      int njtrack = 0;   // the j track of the m_Tracks
      for (const auto& j_b2track : m_Tracks) {
        if (nitrack >= njtrack) {
          njtrack++;
          continue;
        }

        const Belle2::TrackFitResult* jfitresult = j_b2track.getTrackFitResultWithClosestMass(Const::pion);
        if (!jfitresult) {
          B2WARNING("No track fit result found.");
          njtrack++;
          continue;
        }

        // require high NDF track
        int jndf = jfitresult->getNDF();
        if (jndf < 20) {
          njtrack++;
          continue;
        }

        // IP tracks at barrel
        if (fabs(jfitresult->getD0()) < 1.0 && fabs(jfitresult->getZ0()) < 1.0 && jfitresult->getHitPatternCDC().getLastLayer() > 50
            && jfitresult->getHitPatternCDC().getFirstLayer() < 5) {
          double jrk_phiDegree = jfitresult->getPhi() / Unit::deg;
          double deltea_phi    = fabs(phiDegree - jrk_phiDegree);
          double dphi          = deltea_phi;

          if (deltea_phi > 180)
            dphi = 360 - deltea_phi;

          bool trg_fyo = m_trgSummary->testFtdl("fyo");
          bool trg_fyo_nobha = (m_trgSummary->testInput("t2_1")       ||  m_trgSummary->testInput("t2_2") || m_trgSummary->testInput("t2_3"))
                               &&
                               (m_trgSummary->testInput("ty_0")       ||  m_trgSummary->testInput("ty_1") || m_trgSummary->testInput("ty_2")
                                || m_trgSummary->testInput("ty_3")) &&
                               m_trgSummary->testInput("cdc_open90");

          // cout<<"i = "<<nitrack <<"   j= "<<njtrack  <<"   phiDegree = "<<phiDegree<<"    jrk_phiDegree =  "<<jrk_phiDegree<<"     dphi "<<dphi<<endl;

          phi_fyo_dphi.push_back(dphi);
          phi_nobha_fyo_dphi.push_back(dphi);

          if (trg_psnecl) {
            phi_fyo_dphi_psnecl.push_back(dphi);
            phi_nobha_fyo_dphi_psnecl.push_back(dphi);
          }
          if (trg_psnecl && trg_fyo) {
            phi_fyo_dphi_psnecl_ftdf.push_back(dphi);
          }
          if (trg_psnecl && trg_fyo_nobha) {
            phi_nobha_fyo_dphi_psnecl_ftdf.push_back(dphi);
          }
        }
        njtrack++;
      }
    }
    nitrack++;
  }


  // the largest cdc_open angle in an event for fyo bit
  if (phi_fyo_dphi_psnecl_ftdf.size() != 0) {
    auto max_it      = std::max_element(phi_fyo_dphi_psnecl_ftdf.begin(), phi_fyo_dphi_psnecl_ftdf.end());
    double max_value = *max_it;
    m_fyo_dphi_psnecl_ftdf->Fill(max_value);
  }
  if (phi_fyo_dphi_psnecl.size() != 0) {
    auto max_it      = std::max_element(phi_fyo_dphi_psnecl.begin(), phi_fyo_dphi_psnecl.end());
    double max_value = *max_it;
    m_fyo_dphi_psnecl->Fill(max_value);
  }
  if (phi_fyo_dphi.size() != 0) {
    auto max_it      = std::max_element(phi_fyo_dphi.begin(), phi_fyo_dphi.end());
    double max_value = *max_it;
    m_fyo_dphi->Fill(max_value);
  }

  //
  if (phi_nobha_fyo_dphi_psnecl_ftdf.size() != 0) {
    auto max_it      = std::max_element(phi_nobha_fyo_dphi_psnecl_ftdf.begin(), phi_nobha_fyo_dphi_psnecl_ftdf.end());
    double max_value = *max_it;
    m_nobha_fyo_dphi_psnecl_ftdf->Fill(max_value);
  }
  if (phi_nobha_fyo_dphi_psnecl.size() != 0) {
    auto max_it      = std::max_element(phi_nobha_fyo_dphi_psnecl.begin(), phi_nobha_fyo_dphi_psnecl.end());
    double max_value = *max_it;
    m_nobha_fyo_dphi_psnecl->Fill(max_value);
  }
  if (phi_nobha_fyo_dphi.size() != 0) {
    auto max_it      = std::max_element(phi_nobha_fyo_dphi.begin(), phi_nobha_fyo_dphi.end());
    double max_value = *max_it;
    m_nobha_fyo_dphi->Fill(max_value);
  }


  // the largest momentum track p in an event for stt bit
  if (p_stt_P3_psnecl_ftdf.size() != 0) {
    auto max_it      = std::max_element(p_stt_P3_psnecl_ftdf.begin(), p_stt_P3_psnecl_ftdf.end());
    double max_value = *max_it;
    m_stt_P3_psnecl_ftdf->Fill(max_value);
  }
  if (p_stt_P3_psnecl.size() != 0) {
    auto max_it      = std::max_element(p_stt_P3_psnecl.begin(), p_stt_P3_psnecl.end());
    double max_value = *max_it;
    m_stt_P3_psnecl->Fill(max_value);
  }
  if (p_stt_P3.size() != 0) {
    auto max_it      = std::max_element(p_stt_P3.begin(), p_stt_P3.end());
    double max_value = *max_it;
    m_stt_P3->Fill(max_value);
  }

  //
  if (p_nobha_stt_P3_psnecl_ftdf.size() != 0) {
    auto max_it      = std::max_element(p_nobha_stt_P3_psnecl_ftdf.begin(), p_nobha_stt_P3_psnecl_ftdf.end());
    double max_value = *max_it;
    m_nobha_stt_P3_psnecl_ftdf->Fill(max_value);
  }
  if (p_nobha_stt_P3_psnecl.size() != 0) {
    auto max_it      = std::max_element(p_nobha_stt_P3_psnecl.begin(), p_nobha_stt_P3_psnecl.end());
    double max_value = *max_it;
    m_nobha_stt_P3_psnecl->Fill(max_value);
  }
  if (p_nobha_stt_P3.size() != 0) {
    auto max_it      = std::max_element(p_nobha_stt_P3.begin(), p_nobha_stt_P3.end());
    double max_value = *max_it;
    m_nobha_stt_P3->Fill(max_value);
  }




}



void TRGEFFDQMModule::endRun()
{
}

void TRGEFFDQMModule::terminate()
{
}
