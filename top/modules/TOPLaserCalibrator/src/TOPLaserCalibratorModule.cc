/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Roberto Stroili                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <top/modules/TOPLaserCalibrator/TOPLaserCalibratorModule.h>
#include <framework/core/ModuleManager.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework - Database
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/Database.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// DataStore classes
#include <framework/dataobjects/EventMetaData.h>
#include <top/dataobjects/TOPDigit.h>

// database classes
#include <framework/database/DBStore.h>
#include <top/dbobjects/TOPASICChannel.h>

#include <TFile.h>
#include <TH1F.h>
#include <TF1.h>
#include <TTree.h>
#include <TClonesArray.h>
#include <sstream>

using namespace std;

namespace Belle2 {

  double threegauss(double* x, double* par)
  {
    /**
     * 3 gaussian fitting functions
     * the first two gaussians (ordered in descending mode with the normalization)
     * have a fixed distance (determined with MonteCarlo simulation) and a fixed
     * normalization ratio (also this determined with MonteCarlo simulation)
     */
    double n1 = par[0];
    double m1 = par[1];
    double w1 = par[2];
    double n2 = n1 * par[3];
    double m2 = par[4] + m1;
    double w2 = par[5];
    double n3 = par[6];
    double m3 = par[7];
    double w3 = par[8];
    if (w1 == 0) n1 = 0.;

    double val;
    val = n1 * exp(-0.5 * (x[0] - m1) * (x[0] - m1) / w1 / w1) +
          n2 * exp(-0.5 * (x[0] - m2) * (x[0] - m2) / w2 / w2) +
          n3 * exp(-0.5 * (x[0] - m3) * (x[0] - m3) / w3 / w3);
    return val;
  }

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPLaserCalibrator)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPLaserCalibratorModule::TOPLaserCalibratorModule() : Module(), m_fittingParmTree(0)
  {
    // set module description (e.g. insert text)
    setDescription("iTOP Calibration using the laser calibration system");

    // Add parameters
    addParam("histogramFileName", m_histogramFileName, "Output root file for histograms",
             string(""));
    addParam("referenceFileName", m_referenceFileName, "Input root file for fitting parameters",
             string(""));

    addParam("barID", m_barID, "ID of TOP module to calibrate");

    addParam("runLow", m_runLow, "IOV:  run lowest", 0);
    addParam("runHigh", m_runHigh, "IOV:  run highest", 0);

    addParam("gaussFit", m_gaussFit, "if true (default) fit single gaussian", true);

    for (int i = 0; i < c_NumChannels; ++i) {
      m_histo[i] = 0;
    }

  }

  TOPLaserCalibratorModule::~TOPLaserCalibratorModule()
  {
  }

  void TOPLaserCalibratorModule::initialize()
  {

    StoreArray<TOPDigit>::required();

  }

  void TOPLaserCalibratorModule::beginRun()
  {
  }

  void TOPLaserCalibratorModule::event()
  {

    StoreArray<TOPDigit> digits;

    for (auto& digit : digits) {
      if (digit.getModuleID() != m_barID) continue;
      unsigned channel = digit.getChannel();
      if (channel < c_NumChannels) {
        auto histo = m_histo[channel];
        if (!histo) {
          stringstream ss;
          ss << "chan" << channel ;
          string name;
          ss >> name;
          string title = "Times " + name;
          histo = new TH1F(name.c_str(), title.c_str(), 1000, 10., 30.);
          m_histo[channel] = histo;
        }
        histo->Fill(digit.getTDC() * 0.025);
      }
    }
  }

  void TOPLaserCalibratorModule::endRun()
  {
  }

  void TOPLaserCalibratorModule::terminate()
  {
    StoreObjPtr<EventMetaData> evtMetaData;
    double parms[9];
    int ch;

    TFile* fittingParms = 0;
    if (!m_referenceFileName.empty()) {
      //  temporary solution: get information for the 3-gaussian fit from a root ttree
      //  eventually it will be replaced by informations stored in the conditions DB
      fittingParms = new TFile(m_referenceFileName.c_str());
      fittingParms->GetObject("times", m_fittingParmTree);
      m_fittingParmTree->SetBranchAddress("pix", &m_pix);
      m_fittingParmTree->SetBranchAddress("nfibers", &m_nfibers);
      m_fittingParmTree->SetBranchAddress("fibers", &(m_fibers[0]));
      m_fittingParmTree->SetBranchAddress("phnorm", &(m_phnorm[0]));
      m_fittingParmTree->SetBranchAddress("phmean", &(m_phmean[0]));
      m_fittingParmTree->SetBranchAddress("phstd", &(m_phstd[0]));
      m_fittingParmTree->SetBranchAddress("diginorm", &(m_diginorm[0]));
      m_fittingParmTree->SetBranchAddress("digimean", &(m_digimean[0]));
      m_fittingParmTree->SetBranchAddress("digistd", &(m_digistd[0]));
    }

    if (!m_histogramFileName.empty()) {
      //  output from fits (temporary)
      TFile* file = new TFile(m_histogramFileName.c_str(), "RECREATE");
      TTree* otree = new TTree("fits", "fitted times");
      if (m_gaussFit) {
        //  fit signal with a single gaussian
        otree->Branch("pixel", &ch, "pixel/I");
        otree->Branch("normalization", &(parms[0]), "normalization/D");
        otree->Branch("time", &(parms[1]), "time/D");
        otree->Branch("resolution", &(parms[2]), "resolution/D");
      } else {
        //  fit signal with three gaussian functions
        otree->Branch("pixel", &ch, "pixel/I");
        otree->Branch("norm1", &(parms[0]), "norm1/D");
        otree->Branch("time1", &(parms[1]), "time1/D");
        otree->Branch("reso1", &(parms[2]), "reso1/D");
        otree->Branch("ratio12", &(parms[3]), "ratio12/D");
        otree->Branch("tdif12", &(parms[4]), "tdif12/D");
        otree->Branch("reso1", &(parms[5]), "reso2/D");
        otree->Branch("norm3", &(parms[6]), "norm3/D");
        otree->Branch("time3", &(parms[7]), "time3/D");
        otree->Branch("reso3", &(parms[8]), "reso3/D");
      }
      if (file->IsZombie()) {
        B2ERROR("Couldn't open file '" << m_histogramFileName << "' for writing!");
      } else {
        file->cd();
        for (ch = 0; ch < c_NumChannels; ++ch) {
          TH1F* histo = m_histo[ch];
          if (histo) {
            cout << "histo ID " << ch << endl;
            TF1* func = 0;
            if (m_gaussFit) {
              //  fit with one gaussian
              func = makeGFit(histo);
            } else {
              //  fit with three gaussian functions
              func = makeFit(histo, ch);
            }
            //  save histogram
            histo->Write();
            func->GetParameters(parms);
            otree->Fill();
          }
        }
      }
      //  write ttree
      otree->Write();
      file->Close();
    }

    /*   some leftover stuff from the TOPWFCalibrator module    */

//     for (unsigned i = 0; i < c_NumChannels; i++) {
//       TH1F* histo = m_histo[i];
//       if (!histo) continue;
//     }

//     TClonesArray constants("Belle2::TOPASICChannel");
//     const auto name = DBStore::arrayName<TOPASICChannel>("");

//     for (int channel = 0; channel < c_NumChannels; channel++) {
//       new(constants[channel]) TOPASICChannel(m_barID, channel, numWindows);
//       auto* channelConstants = static_cast<TOPASICChannel*>(constants[channel]);
//       for (int window = 0; window < c_NumWindows; window++) {
//         TProfile* prof = m_profile[channel][window];
//         if (prof) {
//           TOPASICPedestals pedestals(window);
//           pedestals.setPedestals(prof);
//           bool ok = channelConstants->setPedestals(pedestals);
//           if (!ok) {
//             B2ERROR("TOPLaserCalibratorModule: can't set pedestals for channel "
//                     << channel << " storage window " << window);
//           }
//         }
//       }
//     }

//     auto expNo = evtMetaData->getExperiment();
//     IntervalOfValidity iov(expNo, m_runLow, expNo, m_runHigh);
//     Database::Instance().storeData(name, &constants, iov);


//     int all = 0;
//     int incomplete = 0;
//     for (int channel = 0; channel < constants.GetEntriesFast(); channel++) {
//       const auto* chan = static_cast<TOPASICChannel*>(constants[channel]);
//       all++;
//       if (chan->getNumofGoodWindows() != chan->getNumofWindows()) incomplete++;
//     }

//     B2RESULT("TOPLaserCalibratorModule: bar ID = " << m_barID <<
//              ", number of calibrated channels " << all <<
//              " (" << incomplete << " are incomplete)");

  }

  TF1* TOPLaserCalibratorModule::makeFit(TH1F* h, int ch)
  {
    double m = h->GetMean();
    double w = h->GetRMS();
    double n = h->Integral() * h->GetBinWidth(1) / w;
    TF1* func = new TF1("threeg", threegauss, 18., 24., 9);
    func->SetParName(0, "norm1");
    func->SetParName(1, "mean1");
    func->SetParName(2, "sigma1");
    func->SetParName(3, "normRatio");
    func->SetParName(4, "distance");
    func->SetParName(5, "sigma2");
    func->SetParName(6, "norm3");
    func->SetParName(7, "mean3");
    func->SetParName(8, "sigma3");
    double params[9];
    if (m_fittingParmTree) {
      m_fittingParmTree->GetEntry(ch);
      params[0] = n * (1. - m_diginorm[1] / m_diginorm[0]);
      params[1] = m_digimean[0];
      params[2] = 0.08;
      params[3] = m_diginorm[1] / m_diginorm[0];
      params[4] = m_phmean[1] - m_phmean[0];
      params[5] = 0.08;
      params[6] = 5.;
      params[7] = 20.7;
      params[8] = 1.;
    } else {
      params[0] = n;
      params[1] = m;
      params[2] = w;
      params[3] = 0.1;
      params[4] = 0.01;
      params[5] = 0.08;
      params[6] = 5.;
      params[7] = 20.7;
      params[8] = 1.;

    }
    func->SetParameters(params);
    for (int j = 0; j < 9; ++j) {
      func->SetParLimits(j, 0.1 * params[j], 100.*params[j]);
    }

    func->SetParLimits(2, 0.04, 0.2);
    func->SetParLimits(3, params[3] * 0.2, params[3] * 2.);
    func->SetParLimits(5, 0.03, 0.25);
    func->SetParLimits(6, 0., params[0] * 0.1);
    func->SetNpx(10000);
    if (m_fittingParmTree) {
      func->FixParameter(4, params[4]);
      func->FixParameter(3, params[3]);
    } else {
      func->SetParLimits(4, -0.5, 0.5);
    }

    int status = h->Fit("threeg", "Q", "", m - 2.*w, m + 4.*w);

    if (status != 0) {
      status = h->Fit("threeg", "", "", m - w, m + w);
    }
    func->SetNpx(1000);
    func->GetParameters(params);
//     for (int k=0; k<9; ++k) cout << params[k] << " ";
//     cout << endl;
//     cout << "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+" << endl;
    return func;
  }

  TF1* TOPLaserCalibratorModule::makeGFit(TH1F* h)
  {
    double m = h->GetMean();
    double w = h->GetRMS();
    h->Fit("gaus", "Q", "", m - 2.*w, m + 4.*w);
    double parms[3];
    TF1* func = h->GetFunction("gaus");
    func->GetParameters(parms);
    func->SetNpx(1000);
    h->Fit("gaus", "Q", "", m - w, m + w);
    func = h->GetFunction("gaus");
    func->GetParameters(parms);
//     for (int k=0; k<3; ++k) cout << parms[k] << " ";
//     cout << endl;
//     cout << "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+" << endl;
    return func;
  }

} // end Belle2 namespace

