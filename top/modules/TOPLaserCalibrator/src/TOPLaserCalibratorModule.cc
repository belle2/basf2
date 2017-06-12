/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
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
#include <framework/dataobjects/EventMetaData.h>
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

  TOPLaserCalibratorModule::TOPLaserCalibratorModule() : Module(), m_fittingParmTree(0)
  {
    setDescription("T0 Calibration using the laser calibration system");

    // Add parameters
    addParam("histogramFileName", m_histogramFileName, "Output root file for histograms",
             string("")); //output fitting results
    addParam("simFileName", m_simFileName, "Input simulation root file ",
             string("")); //a sim root file as input for a local test

    addParam("fitPixelID", m_fitPixelID, "set 0 - 511 to a specific pixelID; set 512 to all pixels in the fit",
             512);
    addParam("barID", m_barID, "ID of TOP module to calibrate");
    addParam("fitMethod", m_fitMethod, "gaus: single gaussian; cb: single Crystal Ball; cb2: double Crystal Ball", string("gauss"));
    addParam("fitRange", m_fitRange, "fit range[nbins, xmin, xmax]");

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
      unsigned channel = digit.getPixelID() - 1; //define pixelID as channel here
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
      double digitTime;
      int pixelID;
      tree_laser->SetBranchAddress("digitTime", &digitTime);
      tree_laser->SetBranchAddress("pixelID", &pixelID);

      int nevt = tree_laser->GetEntries();

      for (int i = 0; i < nevt; i++) {
        tree_laser->GetEntry(i);
        if (digitTime < m_fitRange[1] || digitTime > m_fitRange[2]) continue;
        if ((pixelID - 1) < c_NumChannels) {
          auto histo = m_histo[pixelID - 1];
          if (!histo) {
            stringstream ss;
            ss << "pixel" << pixelID - 1 ;
            string name;
            ss >> name;
            string title = "Times " + name;
            histo = new TH1F(name.c_str(), title.c_str(), (int)m_fitRange[0], m_fitRange[1], m_fitRange[2]);
            m_histo[pixelID - 1] = histo;
          }
          histo->Fill(digitTime);
        }
      }
    }

    auto t0fit = new  LaserCalibratorFit(m_barID); //class LaserCalibratorFit
    t0fit->setHist(m_histo); //set time hist array including 512 channels
    t0fit->setFitMethod(m_fitMethod);
    t0fit->setFitRange(m_fitRange[1], m_fitRange[2]);
    if (m_fitPixelID == 512) {
      t0fit->fitAllPixels();
    } else {
      t0fit->fitPixel(m_fitPixelID);
    }
    //cout<<"chisq=\t"<<t0fit->getPixelChisq(6)<<endl;
    t0fit->writeFile(m_histogramFileName); //write to root file
    // ...
    // write to database; next to do ...
    // ...
  }

} // end Belle2 namespace

