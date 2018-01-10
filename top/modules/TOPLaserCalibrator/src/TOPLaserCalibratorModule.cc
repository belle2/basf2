/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Roberto Stroili, Wenlong Yuan                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <top/modules/TOPLaserCalibrator/TOPLaserCalibratorModule.h>
#include <top/modules/TOPLaserCalibrator/LaserCalibratorFit.h>

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
//#include <framework/dataobjects/EventMetaData.h>
#include <top/dataobjects/TOPDigit.h>

// database classes
#include <framework/database/DBStore.h>
#include <top/dbobjects/TOPASICChannel.h>

#include <Math/PdfFuncMathCore.h>
#include <TFile.h>
#include <TH1F.h>
#include <TF1.h>
#include <TMath.h>
#include <TTree.h>
#include <TClonesArray.h>
#include <sstream>


using namespace std;

namespace Belle2 {
  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPLaserCalibrator)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPLaserCalibratorModule::TOPLaserCalibratorModule() : Module()
  {
    setDescription("T0 Calibration using the laser calibration system");

    std::vector<double> frange = {100, 0., 1.};
    // Add parameters
    addParam("dataFitOutput", m_dataFitOutput, "Output root file for data",
             string("")); //output fitting results
    addParam("mcInput", m_mcInput, "Input root file from MC", string(""));
    addParam("channelT0constant", m_chT0C, "Output of channel T0 constant", string(""));
    addParam("fitChannel", m_fitChannel, "set 0 - 511 to a specific channel; set 0 to all channels in the fit",
             512);
    addParam("barID", m_barID, "ID of TOP module to calibrate");
    addParam("refCh", m_refCh, "reference channel of T0 constant");
    addParam("fitMethod", m_fitMethod, "gaus: single gaussian; cb: single Crystal Ball; cb2: double Crystal Ball", string("gauss"));
    addParam("fitRange", m_fitRange, "fit range[nbins, xmin, xmax]", frange);

    for (int i = 0; i < c_NumChannels; ++i) {
      m_histo[i] = 0;
    }
  }

  TOPLaserCalibratorModule::~TOPLaserCalibratorModule()
  {
  }

  void TOPLaserCalibratorModule::initialize()
  {
    m_digits.isRequired();
  }

  void TOPLaserCalibratorModule::beginRun()
  {
  }

  void TOPLaserCalibratorModule::event()
  {
    for (auto& digit : m_digits) {
      if (m_barID != 0 && digit.getModuleID() != m_barID) continue; //if m_barID == 0, use all 16 slots(for MC test)
      unsigned channel = digit.getChannel();
      if (channel < c_NumChannels) {
        auto histo = m_histo[channel];
        if (!histo) {
          stringstream ss;
          ss << "chan" << channel ;
          string name;
          ss >> name;
          string title = "Times " + name;
          histo = new TH1F(name.c_str(), title.c_str(), (int)m_fitRange[0], m_fitRange[1], m_fitRange[2]);
          m_histo[channel] = histo;
        }
        if (digit.getHitQuality() != TOPDigit::EHitQuality::c_Good) continue;
        if (digit.getTime() < m_fitRange[1] || digit.getTime() > m_fitRange[2]) continue;
        histo->Fill(digit.getTime()); //get Time from TOPDigit
      }
    }
  }

  void TOPLaserCalibratorModule::endRun()
  {
  }

  void TOPLaserCalibratorModule::terminate()
  {
    std::vector<TH1F*> v_histo;
    for (int i = 0; i < c_NumChannels; ++i) {
      v_histo.push_back(m_histo[i]);
    }

    double dataT[c_NumChannels] = {0};
    double dataTErr[c_NumChannels] = {0};
    double mcT[c_NumChannels] = {0};

    TOP::LaserCalibratorFit t0fit(m_barID);
    t0fit.setHist(v_histo); //set time hist vector
    t0fit.setFitMethod(m_fitMethod);
    t0fit.setFitRange(m_fitRange[1], m_fitRange[2]);
    if (m_fitChannel == c_NumChannels) {
      for (int i = 0; i < c_NumChannels; ++i) {
        t0fit.fitChannel(i);
        dataT[i] = t0fit.getFitT();
        dataTErr[i] = t0fit.getFitTErr();
      }
    } else {
      t0fit.fitChannel(m_fitChannel);
      dataT[m_fitChannel] = t0fit.getFitT();
    }
    //cout<<"chisq=\t"<<t0fit.getFitChisq(6)<<endl;
    t0fit.writeFile(m_dataFitOutput); //write to root file

    //read laser propagation time from MC (TOPChannelT0MC)
    auto mcFile = new TFile(m_mcInput.c_str());
    auto tree = (TTree*)mcFile->Get("t0MC");
    int channel_mc;
    double maxpos;

    tree->SetBranchAddress("channel", &channel_mc);
    tree->SetBranchAddress("maxpos", &maxpos);
    for (int i = 0; i < tree->GetEntries(); i++) {
      tree->GetEntry(i);
      mcT[channel_mc] = maxpos;
    }

    //w.r.t. ref. channel
    for (int i = 0; i < c_NumChannels; i++) {
      if (i == m_refCh) continue;
      dataT[i] = dataT[i] - dataT[m_refCh];
      mcT[i] = mcT[i] - mcT[m_refCh];
    }

    delete tree;
    mcFile->Close();
    delete mcFile;

    //calculate T0 Const, write to root file
    auto file = new TFile(m_chT0C.c_str(), "RECREATE");
    auto otree = new TTree("chT0", "Channel T0 Const");

    unsigned channel = 0;
    double t0_const = 0;
    double t0ConstRaw = 0;
    double fittedTime = 0;
    double fittedTimeError = 0;
    double mcTime = 0;
    double mcCorrection = 0;
    double t0ConstError = 0;

    otree->Branch("fittedTime", &fittedTime, "fittedTime/D");
    otree->Branch("fittedTimeError", &fittedTimeError, "fittedTimeError/D");
    otree->Branch("mcTime", &mcTime, "mcTime/D");
    otree->Branch("t0ConstRaw", &t0ConstRaw, "t0ConstRaw/D");
    otree->Branch("mcCorrection", &mcCorrection, "mcCorrection/D");
    otree->Branch("t0Const", &t0_const, "t0_const/D");
    otree->Branch("t0ConstError", &t0ConstError, "t0ConstError/D");
    otree->Branch("channel", &channel, "channel/I");
    otree->Branch("slot", &m_barID, "slot/I");

    for (int i = 0; i < c_NumChannels; i++) {
      if (i == m_refCh) {
        fittedTime = dataT[i];
        mcTime = mcT[i];
      } else {
        fittedTime = dataT[i] + dataT[m_refCh];
        mcTime = mcT[i] +  mcT[m_refCh];
      }
      fittedTimeError = dataTErr[i];
      t0ConstRaw = dataT[i];
      mcCorrection = mcT[i];
      channel = i;
      t0_const = dataT[i] - mcT[i];
      // assuming that the MC error is negligible
      t0ConstError = TMath::Sqrt(dataTErr[i] * dataTErr[i] + dataTErr[m_refCh] * dataTErr[m_refCh]);
      if (i == m_refCh) {
        t0_const = 0;
        t0ConstError = dataTErr[m_refCh];
      }
      otree->Fill();
    }
    otree->Write();
    delete otree;
    file->Close();
    delete file;

    // ...
    // write to database; next to do ...
    // ...
  }
} // end Belle2 namespace

