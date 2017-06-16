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
    addParam("histogramFileName", m_histogramFileName, "Output root file for histograms",
             string("")); //output fitting results
    addParam("simFileName", m_simFileName, "Input simulation root file ",
             string("")); //a sim root file as input for a local test

    addParam("fitChannel", m_fitChannel, "set 0 - 511 to a specific channel; set 0 to all channels in the fit",
             512);
    addParam("barID", m_barID, "ID of TOP module to calibrate");
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
    StoreArray<TOPDigit>::required();
  }

  void TOPLaserCalibratorModule::beginRun()
  {
  }

  void TOPLaserCalibratorModule::event()
  {

    if (!m_simFileName.empty()) return;

    StoreArray<TOPDigit> digits;
    for (auto& digit : digits) {
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
    if (!m_simFileName.empty()) {  //get Time from input root file, for local test
      auto simfile = new TFile(m_simFileName.c_str());
      TTree* tree_laser;
      simfile->GetObject("tree_laser", tree_laser);
      double digitTime = 0;
      int channel = 0;
      tree_laser->SetBranchAddress("digitTime", &digitTime);
      tree_laser->SetBranchAddress("channel", &channel);

      int nevt = tree_laser->GetEntries();

      for (int i = 0; i < nevt; i++) {
        tree_laser->GetEntry(i);
        if (digitTime < m_fitRange[1] || digitTime > m_fitRange[2]) continue;
        if ((channel - 1) < c_NumChannels) {
          auto histo = m_histo[channel - 1];
          if (!histo) {
            stringstream ss;
            ss << "channel" << channel - 1 ;
            string name;
            ss >> name;
            string title = "Times " + name;
            histo = new TH1F(name.c_str(), title.c_str(), (int)m_fitRange[0], m_fitRange[1], m_fitRange[2]);
            m_histo[channel - 1] = histo;
          }
          histo->Fill(digitTime);
        }
      }
    }

    std::vector<TH1F*> v_histo;
    for (int i = 0; i < c_NumChannels; ++i) {
      v_histo.push_back(m_histo[i]);
    }

    //auto t0fit = new TOP::LaserCalibratorFit(m_barID); //class LaserCalibratorFit
    TOP::LaserCalibratorFit t0fit(m_barID);
    t0fit.setHist(v_histo); //set time hist vector
    t0fit.setFitMethod(m_fitMethod);
    t0fit.setFitRange(m_fitRange[1], m_fitRange[2]);
    if (m_fitChannel == c_NumChannels) {
      for (int i = 0; i < c_NumChannels; ++i) {
        t0fit.fitChannel(i);
      }
    } else {
      t0fit.fitChannel(m_fitChannel);
    }
    //cout<<"chisq=\t"<<t0fit.getFitChisq(6)<<endl;
    t0fit.writeFile(m_histogramFileName); //write to root file
    // ...
    // write to database; next to do ...
    // ...
  }
} // end Belle2 namespace

