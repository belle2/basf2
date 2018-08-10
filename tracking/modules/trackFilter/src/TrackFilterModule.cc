/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//#include <tracking/modules/trackingPerformanceEvaluation/PerformanceEvaluationBaseClass.h>
#include <tracking/modules/trackFilter/TrackFilterModule.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/HitPatternVXD.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <framework/datastore/StoreArray.h>

using namespace Belle2;

double TrackFilterModule::m_min_d0 = -100;
double TrackFilterModule::m_max_d0 = +100;
double TrackFilterModule::m_min_z0 = -500;
double TrackFilterModule::m_max_z0 = +500;
int TrackFilterModule::m_min_NumHitsSVD = 0;
int TrackFilterModule::m_min_NumHitsPXD = 0;
int TrackFilterModule::m_min_NumHitsCDC = 0;
double TrackFilterModule::m_min_pCM = 0;
double TrackFilterModule::m_min_pT = 0;
double TrackFilterModule::m_min_Pval = 0;

TH1F* TrackFilterModule::m_d0_sel = NULL;
TH1F* TrackFilterModule::m_d0_exc = NULL;
TH1F* TrackFilterModule::m_z0_sel = NULL;
TH1F* TrackFilterModule::m_z0_exc = NULL;
TH2F* TrackFilterModule::m_d0z0_sel = NULL;
TH2F* TrackFilterModule::m_d0z0_exc = NULL;
TH1F* TrackFilterModule::m_nPXD_sel = NULL;
TH1F* TrackFilterModule::m_nPXD_exc = NULL;
TH1F* TrackFilterModule::m_nSVD_sel = NULL;
TH1F* TrackFilterModule::m_nSVD_exc = NULL;
TH1F* TrackFilterModule::m_nCDC_sel = NULL;
TH1F* TrackFilterModule::m_nCDC_exc = NULL;
TH1F* TrackFilterModule::m_pT_sel = NULL;
TH1F* TrackFilterModule::m_pT_exc = NULL;
TH1F* TrackFilterModule::m_pCM_sel = NULL;
TH1F* TrackFilterModule::m_pCM_exc = NULL;
TH1F* TrackFilterModule::m_pval_sel = NULL;
TH1F* TrackFilterModule::m_pval_exc = NULL;

bool TrackFilterModule::m_saveControlHistos = true;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TrackFilter)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TrackFilterModule::TrackFilterModule() : Module()
{
  // Set module properties
  setDescription("generates a new StoreArray from the input StoreArray which has all specified Tracks removed");

  // Parameter definitions
  addParam("inputArrayName", m_inputArrayName, "StoreArray with the input tracks", std::string("Tracks"));
  addParam("outputINArrayName", m_outputINArrayName, "StoreArray with the output tracks", std::string(""));
  addParam("outputOUTArrayName", m_outputOUTArrayName, "StoreArray with the output tracks", std::string(""));

  //selection parameter definition
  addParam("min_d0", m_min_d0, "minimum value of the d0", double(-100));
  addParam("max_d0", m_max_d0, "maximum value of the d0", double(+100));
  addParam("min_z0", m_min_z0, "minimum value of the z0", double(-500));
  addParam("max_z0", m_max_z0, "maximum value of the z0", double(+500));
  addParam("min_pCM", m_min_pCM, "minimum value of the center-of-mass-momentum", double(0));
  addParam("min_pT", m_min_pT, "minimum value of the transverse momentum", double(0));
  addParam("min_Pvalue", m_min_Pval, "minimum value of the P-Value of the track fit", double(0));
  addParam("min_NumHitPXD", m_min_NumHitsPXD, "minimum number of PXD hits associated to the trcak", int(0));
  addParam("min_NumHitSVD", m_min_NumHitsSVD, "minimum number of SVD hits associated to the trcak", int(0));
  addParam("min_NumHitCDC", m_min_NumHitsCDC, "minimum number of CDC hits associated to the trcak", int(0));

  addParam("saveControlHistograms", m_saveControlHistos, "if true, generate a rootfile containing histograms ", bool(true));
  addParam("outputFileName", m_rootFileName, "Name of output root file.",
           std::string("TrackFilterControlHistograms.root"));

}

TrackFilterModule::~TrackFilterModule()
{
}


void TrackFilterModule::initialize()
{

  B2INFO("TrackFilterModule::inputArrayName: " << m_inputArrayName);
  B2INFO("TrackFilterModule::outputINArrayName: " <<  m_outputINArrayName);
  B2INFO("TrackFilterModule::outputOUTArrayName: " <<  m_outputOUTArrayName);


  StoreArray<Track> inputArray(m_inputArrayName);
  inputArray.isRequired();

  m_selectedTracks.registerSubset(inputArray, m_outputINArrayName);
  m_selectedTracks.inheritAllRelations();

  m_notSelectedTracks.registerSubset(inputArray, m_outputOUTArrayName);
  m_notSelectedTracks.inheritAllRelations();

  if (m_saveControlHistos) {
    //set the ROOT File
    m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");
    //create list of histograms to be saved in the rootfile
    m_histoList_selected = new TList;
    m_histoList_excluded = new TList;

    //D0
    m_d0_sel = new TH1F("d0Sel", "Transverse Impact Parameter (selected tracks)", 200, -5, 5);
    m_d0_sel->GetXaxis()->SetTitle("d0 (cm)");
    m_histoList_selected->Add(m_d0_sel);
    m_d0_exc = new TH1F("d0Exc", "Transverse Impact Parameter (excluded tracks)", 200, -5, 5);
    m_d0_exc->GetXaxis()->SetTitle("d0 (cm)");
    m_histoList_excluded->Add(m_d0_exc);
    //Z0
    m_z0_sel = new TH1F("z0Sel", "Longitudinal Parameter (selected tracks)", 200, -5, 5);
    m_z0_sel->GetXaxis()->SetTitle("z0 (cm)");
    m_histoList_selected->Add(m_z0_sel);
    m_z0_exc = new TH1F("z0Exc", "Longitudinal Impact Parameter (excluded tracks)", 200, -5, 5);
    m_z0_exc->GetXaxis()->SetTitle("z0 (cm)");
    m_histoList_excluded->Add(m_z0_exc);
    //D0 VS Z0
    m_d0z0_sel = new TH2F("d0z0Sel", "Transverse VS Longitudinal Parameter (selected tracks)", 200, -5, 5, 200, -5, 5);
    m_d0z0_sel->GetXaxis()->SetTitle("z0 (cm)");
    m_d0z0_sel->GetYaxis()->SetTitle("d0 (cm)");
    m_histoList_selected->Add(m_d0z0_sel);
    m_d0z0_exc = new TH2F("d0z0Exc", "Transverse VS Longitudinal Parameter (excluded tracks)", 200, -5, 5, 200, -5, 5);
    m_d0z0_exc->GetXaxis()->SetTitle("z0 (cm)");
    m_d0z0_exc->GetYaxis()->SetTitle("d0 (cm)");
    m_histoList_excluded->Add(m_d0z0_exc);

    //PXD HITS
    m_nPXD_sel = new TH1F("PXDhitsSel", "Number Of PXD Hits (selected tracks)", 10, 0, 10);
    m_nPXD_sel->GetXaxis()->SetTitle("# PXD hits");
    m_histoList_selected->Add(m_nPXD_sel);
    m_nPXD_exc = new TH1F("PXDhitsExc", "Number Of PXD Hits (excluded tracks)", 10, 0, 10);
    m_nPXD_exc->GetXaxis()->SetTitle("# PXD hits");
    m_histoList_excluded->Add(m_nPXD_exc);
    //SVD HITS
    m_nSVD_sel = new TH1F("SVDhitsSel", "Number Of SVD Hits (selected tracks)", 20, 0, 20);
    m_nSVD_sel->GetXaxis()->SetTitle("# SVD hits");
    m_histoList_selected->Add(m_nSVD_sel);
    m_nSVD_exc = new TH1F("SVDhitsExc", "Number Of SVD Hits (excluded tracks)", 20, 0, 20);
    m_nSVD_exc->GetXaxis()->SetTitle("# SVD hits");
    m_histoList_excluded->Add(m_nSVD_exc);
    //CDC HITS
    m_nCDC_sel = new TH1F("CDChitsSel", "Number Of CDC Hits (selected tracks)", 150, 0, 150);
    m_nCDC_sel->GetXaxis()->SetTitle("# CDC hits");
    m_histoList_selected->Add(m_nCDC_sel);
    m_nCDC_exc = new TH1F("CDChitsExc", "Number Of CDC Hits (excluded tracks)", 150, 0, 150);
    m_nCDC_exc->GetXaxis()->SetTitle("# CDC hits");
    m_histoList_excluded->Add(m_nCDC_exc);
    //P-Value
    m_pval_sel = new TH1F("pValSel", "Track Fit P-value (selected tracks)", 1000, 0, 1);
    m_pval_sel->GetXaxis()->SetTitle("track P-value");
    m_histoList_selected->Add(m_pval_sel);
    m_pval_exc = new TH1F("pValExc", "Track Fit P-value (excted tracks)", 1000, 0, 1);
    m_pval_exc->GetXaxis()->SetTitle("track P-value");
    m_histoList_excluded->Add(m_pval_exc);
    //P-CMS
    m_pCM_sel = new TH1F("pCMSel", "Center of Mass Momentum (selected tracks)", 100, 0, 6);
    m_pCM_sel->GetXaxis()->SetTitle("p* (GeV/c)");
    m_histoList_selected->Add(m_pCM_sel);
    m_pCM_exc = new TH1F("pCMExc", "Center of Mass Momentum (excluded tracks)", 100, 0, 6);
    m_pCM_exc->GetXaxis()->SetTitle("p* (GeV/c)");
    m_histoList_excluded->Add(m_pCM_exc);
    //P-CMS
    m_pT_sel = new TH1F("ptSel", "Transverse Momentum (selected tracks)", 100, 0, 5.5);
    m_pT_sel->GetXaxis()->SetTitle("pT (GeV/c)");
    m_histoList_selected->Add(m_pT_sel);
    m_pT_exc = new TH1F("ptExc", "Transverse  Momentum (excluded tracks)", 100, 0, 5.5);
    m_pT_exc->GetXaxis()->SetTitle("pT (GeV/c)");
    m_histoList_excluded->Add(m_pT_exc);

  }


}


void TrackFilterModule::beginRun()
{
}

void TrackFilterModule::event()
{

  m_selectedTracks.select(isSelected);

  m_notSelectedTracks.select([](const Track * track) {
    return !isSelected(track);
  }
                            );

}

void TrackFilterModule::endRun()
{
}

void TrackFilterModule::terminate()
{
  if (m_rootFilePtr != NULL) {
    m_rootFilePtr->cd();

    TDirectory* oldDir = gDirectory;

    TDirectory* dir_selected = oldDir->mkdir("selected");
    dir_selected->cd();
    TIter nextH_selected(m_histoList_selected);
    TObject* obj;
    while ((obj = nextH_selected()))
      obj->Write();

    TDirectory* dir_excluded = oldDir->mkdir("excluded");
    dir_excluded->cd();
    TIter nextH_excluded(m_histoList_excluded);
    while ((obj = nextH_excluded()))
      obj->Write();

    m_rootFilePtr->Close();

  }
}

bool TrackFilterModule::isSelected(const Track* track)
{

  bool isExcluded = false;
  int pionCode = 211;

  const TrackFitResult*  tfr = track->getTrackFitResult(Const::ChargedStable(pionCode));
  if (tfr == NULL)
    return false;

  if (tfr->getD0() < m_min_d0 || tfr->getD0() > m_max_d0)
    isExcluded = true;

  if (tfr->getZ0() < m_min_z0 || tfr->getZ0() > m_max_z0)
    isExcluded = true;

  if (tfr->getPValue() < m_min_Pval)
    isExcluded = true;

  HitPatternVXD hitPatternVXD = tfr->getHitPatternVXD();
  if (hitPatternVXD.getNSVDHits() < m_min_NumHitsSVD ||  hitPatternVXD.getNPXDHits() < m_min_NumHitsPXD)
    isExcluded = true;

  HitPatternCDC hitPatternCDC = tfr->getHitPatternCDC();
  if (hitPatternCDC.getNHits() < m_min_NumHitsCDC)
    isExcluded = true;

  if (m_saveControlHistos)
    fillControlHistograms(track, !isExcluded);

  return !isExcluded;

}

void TrackFilterModule::fillControlHistograms(const Track* track , bool isSelected)
{

  int pionCode = 211;
  const TrackFitResult*  tfr = track->getTrackFitResult(Const::ChargedStable(pionCode));
  HitPatternVXD hitPatternVXD = tfr->getHitPatternVXD();
  HitPatternCDC hitPatternCDC = tfr->getHitPatternCDC();

  double d0 = tfr->getD0();
  double z0 = tfr->getZ0();
  double pt = tfr->getMomentum().Pt();
  TLorentzVector pStar = tfr->get4Momentum();
  pStar.Boost(0, 0, 3. / 11);
  double pCM = pStar.P();
  double pVal = tfr->getPValue();
  int nPXDhits = hitPatternVXD.getNPXDHits();
  int nSVDhits = hitPatternVXD.getNSVDHits();
  int nCDChits = hitPatternCDC.getNHits();

  if (isSelected) {
    m_d0_sel->Fill(d0);
    m_z0_sel->Fill(z0);
    m_d0z0_sel->Fill(z0, d0);
    m_nPXD_sel->Fill(nPXDhits);
    m_nSVD_sel->Fill(nSVDhits);
    m_nCDC_sel->Fill(nCDChits);
    m_pval_sel->Fill(pVal);
    m_pCM_sel->Fill(pCM);
    m_pT_sel->Fill(pt);
  } else {
    m_d0_exc->Fill(d0);
    m_z0_exc->Fill(z0);
    m_d0z0_exc->Fill(z0, d0);
    m_nPXD_exc->Fill(nPXDhits);
    m_nSVD_exc->Fill(nSVDhits);
    m_nCDC_exc->Fill(nCDChits);
    m_pval_exc->Fill(pVal);
    m_pCM_exc->Fill(pCM);
    m_pT_exc->Fill(pt);
  }

}
