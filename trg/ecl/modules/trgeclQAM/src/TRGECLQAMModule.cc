/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TRGECLQAMModule.cc
// Section  : TRG ECL
// Owner    : HanEol Cho
// Email    : hecho@hep.hanyang.ac.kr & insoo.lee@belle2.org
//---------------------------------------------------------------
// Description : Quality Assurance Monitor Module for TRG ECL
//               Check data quality run by run
//
//              Checking variables
//             - ECLTRG Total Energy peak and width
//             - ECLTRG Caltiming peak and width
//             - ECLTRG Cluster Energy peak and width
//             - Low Hit TC (Less than 0.1 x Average ) in Forward, Backward endcap and Barrel
//
//
//---------------------------------------------------------------
// 1.00 : 2018/11/29 : First version
// 2.00 : 2019/08/08 : Second version
//---------------------------------------------------------------
//framework headers
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
//trg package headers

#include <trg/ecl/modules/trgeclQAM/TRGECLQAMModule.h>

#include <string.h>
#include <fstream>
#include <TFile.h>
#include <TString.h>

#include <TAxis.h>
#include <TTree.h>

#include <RooRealVar.h>
#include <RooDataHist.h>
#include <RooGaussian.h>
#include <RooAddModel.h>
#include <RooPlot.h>
#include <RooArgList.h>
#include <RooNovosibirsk.h>

using namespace std;

namespace Belle2 {
  //
  //

  //! Register module name
  REG_MODULE(TRGECLQAM);
  //
  //
  //
  string
  TRGECLQAMModule::version() const
  {
    return string("TRGECLQAMModule 1.00");
  }
  //
  //
  //
  TRGECLQAMModule::TRGECLQAMModule()
    : Module::Module(),
      m_nevent(1), m_outputfile("")
  {

    string desc = "TRGECLQAMModule(" + version() + ")";
    setDescription(desc);
    setPropertyFlags(c_ParallelProcessingCertified);

    addParam("outputFileName", m_outputfile, "TRGECL QAM file", m_outputfile);

    m_nRun   = 0;
    m_nevent    = 1;

    //Problem TC Check
    m_FWD = 0;
    m_BAR = 0;
    m_BWD = 0;
    m_ALL = 0;
    memset(TCID, 0, sizeof(TCID));


    //Total Energy

    m_etot = 0;
    m_etot_mean = 0;
    m_etot_error = 0;
    m_etot_sigma = 0;

    //  std::vector<int> etot;
    etot.clear();

    //Caltime

    m_caltime_mean = 0;
    m_caltime_error = 0;
    m_caltime_sigma = 0;
    caltime.clear();

    //Cluster E

    clusterE = 0;
    cluster.clear();

    B2DEBUG(100, "TRGECLQAMModule ... created");
  }
  //
  //
  //
  TRGECLQAMModule::~TRGECLQAMModule()
  {

    B2DEBUG(100, "TRGECLQAMModule ... destructed ");
  }
  //
  //
  //
  void
  TRGECLQAMModule::initialize()
  {

    B2DEBUG(100, "TRGECLQAMModule::initialize ... options");


    m_TRGECLUnpackerStore.registerInDataStore();
    m_TRGECLUnpackerEvtStore.registerInDataStore();

    m_TRGSummary.registerInDataStore();
    //  EvtMeta.registterInDataStore();


  }
  //
  //
  //
  void
  TRGECLQAMModule::beginRun()
  {

    B2DEBUG(200, "TRGECLQAMModule ... beginRun called ");

  }
  //
  //
  //
  void
  TRGECLQAMModule::event()
  {

    B2DEBUG(200, "TRGECLFAMMoudle ... event called");



    //QAM ECL Hit Map

    //    StoreArray<TRGECLUnpackerStore> m_TRGECLUnpackerStore;
    for (int ii = 0; ii < m_TRGECLUnpackerStore.getEntries(); ii++) {
      TRGECLUnpackerStore* m_TRGECLUnpacker = m_TRGECLUnpackerStore[ii];


      int TCId = (m_TRGECLUnpacker->getTCId());
      int HitEnergy = (m_TRGECLUnpacker->getTCEnergy());

      m_etot += HitEnergy;
      if (TCId < 1 || TCId > 576 || HitEnergy == 0) {continue;}
      TCID[TCId - 1]++;
      int Caltime = m_TRGECLUnpacker->getTCCALTime();
      if (Caltime != 0) {
        caltime.push_back(Caltime);
      }
    }

    etot.push_back(m_etot);

    m_etot = 0;

    //Unpacker EvtStore
    //for ( int ii = 0; ii < m_TRGECLUnpackerSumStore.getEntries(); ii++){
    TRGECLUnpackerEvtStore* m_TRGECLUnpackerEvt = m_TRGECLUnpackerEvtStore[0];

    if (m_TRGECLUnpackerEvt->get3DBhabhaS() == 1) {
      //Cluster Energy 2D Check
      const int* clE = m_TRGECLUnpackerEvt->getCLEnergy();

      int clER[6] = {0};

      for (int i = 0; i < 6; i++) {
        clER[i] = clE[i];
      }

      sort(clER, clER + 6);

      clusterE = clER[5] + clER[4]; // Ecl1+Ecl2
      cluster.push_back(clusterE);
    }

    StoreObjPtr<EventMetaData> bevt;
    m_nRun = bevt->getRun();
    m_nevent++;

  }
  //
  //
  //
  void
  TRGECLQAMModule::endRun()
  {

    double mean_FWD = 0;
    double mean_BAR = 0;
    double mean_BWD = 0;

    //QAM TC Hit Map Check
    for (int TCId = 1; TCId < 577; TCId++) {
      if (TCId < 81) { //Forward Endcap
        mean_FWD += TCID[TCId - 1];
      } else if (TCId > 80 && TCId < 513) { //Barrel
        mean_BAR += TCID[TCId - 1];
      } else if (TCId > 512) { //Backward Endcap
        mean_BWD += TCID[TCId - 1];
      }
    }
    mean_FWD /= 80;
    mean_BAR /= (512 - 80);
    mean_BWD /= (576 - 512);

    for (int TCId = 1; TCId < 577; TCId++) {
      if (TCId < 81) { //Forward Endcap
        if (TCID[TCId - 1] < mean_FWD * 0.1)m_FWD++;
      } else if (TCId > 80 && TCId < 513) { //Barrel
        if (TCID[TCId - 1] < mean_BAR * 0.1)m_BAR++;
      } else if (TCId > 512) { //Backward Endcap
        if (TCID[TCId - 1] < mean_BWD * 0.1)m_BWD++;
      }
    }

    m_ALL = m_FWD + m_BAR + m_BWD;
    const int etot_size = etot.size();
    for (int i = 0; i < etot_size; i++) {
      h_etot->Fill(etot[i]);
    }
    //Caluate E_total peak and width

    RooRealVar* E_tot = new RooRealVar("E_tot", "E_tot", 0, 4000);
    RooDataHist* data_E_tot = new RooDataHist("data_E_tot", "dataset with E_tot", *E_tot, h_etot);
    RooRealVar* mean_check = new RooRealVar("mean_check", "Mean for checking", 2000, 1000, 3000);
    RooRealVar* sigma_check = new RooRealVar("sigma_check", "Sigma for checking", 100, 0, 300);
    RooGaussian* gauss_check = new RooGaussian("gauss_check", "Gaussian for checking", *E_tot, *mean_check, *sigma_check);
    gauss_check->fitTo(*data_E_tot);
    double mean_ch = mean_check->getVal();

    RooRealVar* mean_E_tot = new RooRealVar("mean_E_tot", "Mean of Gaussian", mean_ch, mean_ch - 200, mean_ch + 200);
    //  RooRealVar sigma_E_tot("sigma_E_tot","Width of Gaussian", 10, 0, 50);

    RooRealVar* sigma1_E_tot = new RooRealVar("sigma1_E_tot", "Width of Gaussian", 30, 0, 60);
    RooGaussian* gauss1_E_tot = new RooGaussian("gauss1_E_tot", "gauss(x,mean,sigma)", *E_tot, *mean_E_tot, *sigma1_E_tot);
    RooRealVar* sigma2_E_tot = new RooRealVar("sigma2_E_tot", "Width of Gaussian", 100, 30, 250);
    RooGaussian* gauss2_E_tot = new RooGaussian("gauss2_E_tot", "gauss(x,mean,sigma)", *E_tot, *mean_E_tot, *sigma2_E_tot);

    RooRealVar* frac_core_E_tot = new RooRealVar("frac_core", "core fraction", 0.85, 0.0, 1.0);
    RooAddModel* gaussm_E_tot = new RooAddModel("gaussm", "core+tail gauss", RooArgList(*gauss1_E_tot, *gauss2_E_tot),
                                                RooArgList(*frac_core_E_tot));

    //  RooGaussian gauss_E_tot("gauss_E_tot","gauss(x,mean,sigma)",E_tot,mean_E_tot,sigma_E_tot);

    gaussm_E_tot->fitTo(*data_E_tot, RooFit::Range(mean_ch - 300, mean_ch + 300));
    //RooPlot* xframe_E_tot = E_tot->frame();
    RooPlot* xframe_E_tot = E_tot->frame(RooFit::Title("energy_tot"));
    //data_E_tot->plotOn(xframe_E_tot);
    data_E_tot->plotOn(xframe_E_tot, RooFit::MarkerStyle(7));
    //  gauss_E_tot->plotOn(xframe_E_tot);
    gaussm_E_tot->plotOn(xframe_E_tot, RooFit::Name("Cal_energy"), RooFit::LineColor(2));
    double v_m_E_tot  = mean_E_tot->getVal();
    double e_m_E_tot  = mean_E_tot->getError();

    double v1_s1_E_tot = sigma1_E_tot->getVal();
    //    double e1_s1_E_tot = sigma1_E_tot->getError();
    double v2_s1_E_tot = sigma2_E_tot->getVal();
    //    double e2_s1_E_tot = sigma2_E_tot->getError();

    double frac_E_tot = frac_core_E_tot->getVal();
    double v_s1_E_tot = v1_s1_E_tot * (frac_E_tot) + v2_s1_E_tot * (1 - frac_E_tot);



    for (long unsigned int ii = 0; ii < caltime.size(); ii++) {
      h_caltime->Fill(caltime[ii]);
    }


    //Calculate Caltime peak and width
    RooRealVar T_c("T_c", "T_c", -100, 100);
    RooDataHist data_T_c("data_T_c", "dataset with T_c", T_c, h_caltime);

    RooRealVar mean_T_c("mean_T_c", "Mean of Gaussian", 0, -100, 100);
    RooRealVar sigma1_T_c("sigma1_T_c", "Width of Gaussian", 10, 0, 50);
    RooGaussian gauss1_T_c("gauss1_T_c", "gauss(x,mean,sigma)", T_c, mean_T_c, sigma1_T_c);
    RooRealVar sigma2_T_c("sigma2_T_c", "Width of Gaussian", 30, 10, 100);
    RooGaussian gauss2_T_c("gauss2_T_c", "gauss(x,mean,sigma)", T_c, mean_T_c, sigma2_T_c);

    RooRealVar frac_core_T_c("frac_core", "core fraction", 0.85, 0.0, 1.0);
    RooAddModel gaussm_T_c("gaussm", "core+tail gauss", RooArgList(gauss1_T_c, gauss2_T_c), RooArgList(frac_core_T_c));

    gaussm_T_c.fitTo(data_T_c, RooFit::Range(-45, 45));
    RooPlot* xframe_T_c = T_c.frame(RooFit::Title("Caltime"));
    data_T_c.plotOn(xframe_T_c, RooFit::MarkerStyle(7));
    gaussm_T_c.plotOn(xframe_T_c, RooFit::Name("Caltime"), RooFit::LineColor(2));
    gaussm_T_c.plotOn(xframe_T_c, RooFit::Components("gauss1_T_c"), RooFit::LineColor(kGreen));
    gaussm_T_c.plotOn(xframe_T_c, RooFit::Components("gauss2_T_c"), RooFit::LineColor(kMagenta));

    xframe_T_c->GetXaxis()->SetTitle("Caltime[ns]");
    xframe_T_c->GetYaxis()->SetTitle("Entries");

    double v_m_T_c  = mean_T_c.getVal();
    double e_m_T_c  = mean_T_c.getError();

    double v1_s1_T_c = sigma1_T_c.getVal();
    //    double e1_s1_T_c = sigma1_T_c.getError();
    double v2_s1_T_c = sigma2_T_c.getVal();
    //    double e2_s1_T_c = sigma2_T_c.getError();

    double frac_T_c = frac_core_T_c.getVal();
    double v_s1_T_c = v1_s1_T_c * (frac_T_c) + v2_s1_T_c * (1 - frac_T_c);



    //Cluster E sum for Bhabha skim
    const int cluster_size = cluster.size();
    for (int ii = 0; ii < cluster_size; ii++) {
      h_clusterE->Fill(cluster[ii]);
    }

    RooRealVar x("x", "esum", 0, 4000);
    RooDataHist dh("dh", "ecltrg",   x, h_clusterE);

    RooRealVar mean("mean",    "mean", 2000, 1000, 2500);
    RooRealVar sigma("sigma", "sigma",  150 ,   0,   300);
    RooRealVar tail("tail",    "tail",  0.45,     0,     1);
    RooNovosibirsk novo("novo", "", x, mean, sigma, tail);
    novo.fitTo(dh, RooFit::Extended(0), RooFit::Range(1500, 2200));


    double clusterE_vm, clusterE_em, clusterE_vs;

    clusterE_vm = mean.getVal();
    clusterE_em = mean.getError();
    clusterE_vs = sigma.getVal();


    TString outputfile = m_outputfile;
    TFile file(outputfile, "RECREATE");
    TTree* tree = new TTree("tree", "tree");
    int nevent;
    double FWD;
    double BAR;
    double BWD;
    double ALL;

    nevent = m_nevent;
    FWD = m_FWD;
    BAR = m_BAR;
    BWD = m_BWD;
    ALL = m_ALL;

    m_etot_mean = v_m_E_tot;
    m_etot_error = e_m_E_tot;
    m_etot_sigma = v_s1_E_tot;

    m_caltime_mean = v_m_T_c;
    m_caltime_error = e_m_T_c;
    m_caltime_sigma = v_s1_T_c;

    tree->Branch("m_nRun",  &m_nRun,   "m_nRun/I");
    tree->Branch("m_nevent",  &nevent,   "m_nevent/I");
    tree->Branch("m_FWD",  &FWD,   "m_FWD/D");
    tree->Branch("m_BAR",  &BAR,   "m_BAR/D");
    tree->Branch("m_BWD",  &BWD,   "m_BWD/D");
    tree->Branch("m_ALL",  &ALL,   "m_ALL/D");
    tree->Branch("m_etot_mean",  &m_etot_mean,   "m_etot_mean/D");
    tree->Branch("m_etot_error",  &m_etot_error,   "m_etot_error/D");
    tree->Branch("m_etot_sigma",  &m_etot_sigma,   "m_etot_sigma/D");
    tree->Branch("m_caltime_mean",  &m_caltime_mean,   "m_caltime_mean/D");
    tree->Branch("m_caltime_error",  &m_caltime_error,   "m_caltime_error/D");
    tree->Branch("m_caltime_sigma",  &m_caltime_sigma,   "m_caltime_sigma/D");
    tree->Branch("m_clusterE_mean",  &clusterE_vm,   "m_clusterE_mean/D");
    tree->Branch("m_clusterE_error",  &clusterE_em,   "m_clusterE_error/D");
    tree->Branch("m_clusterE_sigma",  &clusterE_vs,   "m_clusterE_sigma/D");

    tree->Fill();
    tree->Write();
    file.Close();



    B2INFO("ECL Trigger QAM result "
           << LogVar("Run Number",  m_nRun)
           << LogVar("Total Event",  nevent)
           << LogVar("The # of Low Hit TC in Forward Endcap",  FWD)
           << LogVar("The # of Low Hit TC in Barrel",  BAR)
           << LogVar("The # of Low Hit TC in Backward Endcap",  BWD)
           << LogVar("Total Energy Peak",  m_etot_mean)
           << LogVar("Total Energy Peak width",  m_etot_sigma)
           << LogVar("Caltime Peak",  m_caltime_mean)
           << LogVar("Caltime Peak width",  m_caltime_sigma)
           << LogVar("Cluster Energy Peak",  clusterE_vm)
           << LogVar("Cluster Energy Peak width",  clusterE_vs));


    B2DEBUG(100, "TRGECLQAMModule ... endRun called ");

  }
  //
  //
  //
  void  TRGECLQAMModule::terminate()
  {
    B2DEBUG(100, "TRGECLQAMModule ... terminate called ");

  }
  //
  //
  //
} // namespace Belle2
