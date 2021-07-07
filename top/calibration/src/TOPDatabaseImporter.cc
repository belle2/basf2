/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <top/calibration/TOPDatabaseImporter.h>
#include <top/geometry/TOPGeometryPar.h>

// framework - Database
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBImportArray.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/database/DBArray.h>
#include <framework/database/DBObjPtr.h>

// framework aux
#include <framework/logging/Logger.h>

// DB objects
#include <top/dbobjects/TOPCalTimebase.h>
#include <top/dbobjects/TOPCalChannelT0.h>
#include <top/dbobjects/TOPCalModuleT0.h>
#include <top/dbobjects/TOPCalChannelMask.h>
#include <top/dbobjects/TOPCalChannelPulseHeight.h>
#include <top/dbobjects/TOPCalChannelThresholdEff.h>
#include <top/dbobjects/TOPCalChannelNoise.h>
#include <top/dbobjects/TOPCalChannelRQE.h>
#include <top/dbobjects/TOPCalChannelThreshold.h>
#include <top/dbobjects/TOPCalCommonT0.h>
#include <top/dbobjects/TOPCalIntegratedCharge.h>
#include <top/dbobjects/TOPCalModuleAlignment.h>
#include <top/dbobjects/TOPCalAsicShift.h>
#include <top/dbobjects/TOPCalTimeWalk.h>

#include <top/dbobjects/TOPPmtGainPar.h>
#include <top/dbobjects/TOPPmtQE.h>
#include <top/dbobjects/TOPPmtInstallation.h>
#include <top/dbobjects/TOPPmtObsoleteData.h>
#include <top/dbobjects/TOPPmtTTSPar.h>
#include <top/dbobjects/TOPPmtTTSHisto.h>

#include <top/dbobjects/TOPFrontEndSetting.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <map>

#include "TFile.h"
#include "TTree.h"

extern "C" {
  float phind_lambda_(float*); // phase refractive index of quartz (top_geo.F)
}

using namespace std;

namespace Belle2 {

  using namespace TOP;

  void TOPDatabaseImporter::importSampleTimeCalibration(string fNames,
                                                        int firstExp, int firstRun,
                                                        int lastExp, int lastRun)
  {

    // make vector out of files separated with space

    vector<string> fileNames;
    stringstream ss(fNames);
    string fName;
    while (ss >> fName) {
      fileNames.push_back(fName);
    }

    // prepare what is needed

    const auto* geo = TOPGeometryPar::Instance()->getGeometry();
    auto syncTimeBase = geo->getNominalTDC().getSyncTimeBase();

    DBImportObjPtr<TOPCalTimebase> timeBase;
    timeBase.construct(syncTimeBase);

    set<int> scrodIDs;

    // read constants from files and put them to DB object

    for (const auto& fileName : fileNames) {
      TFile* file = TFile::Open(fileName.c_str(), "r");
      if (!file) {
        B2ERROR("openFile: " << fileName << " *** failed to open");
        continue;
      }
      B2INFO(fileName << ": open for reading");

      TH1F* hsuccess = (TH1F*) file->Get("success");
      if (!hsuccess) {
        B2ERROR("Fit status histogram '" << hsuccess << "' not found");
        file->Close();
        continue;
      }

      int goodChannels = 0;
      int numChannels = hsuccess->GetNbinsX();
      for (int channel = 0; channel < numChannels; channel++) {
        if (hsuccess->GetBinContent(channel + 1) == 0) continue;

        string hname = "sampleTimes_ch" +  to_string(channel);

        TH1F* hsampleTimes = (TH1F*) file->Get(hname.c_str());
        if (!hsampleTimes) {
          B2ERROR("Histogram '" << hname << "' with calibration constants not found");
          continue;
        }
        // parse scrodID from histogram title
        string title = hsampleTimes->GetTitle();
        auto iscrod = title.find("scrod");
        auto ichannel = title.find("channel");
        if (iscrod == string::npos or ichannel == string::npos) {
          B2ERROR("Unsuccessful parsing of scrodID from '" << title << "'");
          continue;
        }
        iscrod += 5;
        int len = ichannel - iscrod;
        if (len < 1) {
          B2ERROR("Unsuccessful parsing of scrodID from '" << title << "'");
          continue;
        }
        int scrodID = stoi(title.substr(iscrod, len));
        scrodIDs.insert(scrodID);

        double rescale = 1;
        if (hsampleTimes->GetBinContent(257) > 0)
          rescale = 2 * syncTimeBase / hsampleTimes->GetBinContent(257);

        vector<double> sampleTimes;
        for (int isamp = 0; isamp < 256; isamp++) {
          sampleTimes.push_back(hsampleTimes->GetBinContent(isamp + 1) * rescale);
        }
        goodChannels++;

        timeBase->append(scrodID, channel, sampleTimes);
      }

      file->Close();
      B2INFO("--> number of calibrated channels: " << goodChannels);
      B2INFO("file closed");
    }

    // set calibration for missing ones, using previous calibrated channel within asic

    B2INFO("set constants for uncalibrated channels using nearest calibrated channel within an ASIC");
    for (auto scrodID : scrodIDs) {
      int nasic = 128 / 8;
      for (int as = 0; as < nasic; as++) {
        const TOPSampleTimes* sampleTimes = 0;
        for (int ch = 0; ch < 15; ch++) {
          int channel = as * 8 + (ch % 8);
          if (timeBase->isAvailable(scrodID, channel)) {
            sampleTimes = timeBase->getSampleTimes(scrodID, channel);
          } else if (sampleTimes) {
            timeBase->append(scrodID, channel, sampleTimes->getTimeAxis());
          }
        }
        if (!sampleTimes) {
          B2INFO("No calibration available for ASIC " << as << " of scrodID " << scrodID);
        }
      }
    }

    // import constants
    IntervalOfValidity iov(firstExp, firstRun, lastExp, lastRun);
    timeBase.import(iov);

    // final message

    int nall = timeBase->getSampleTimes().size();
    int ncal = 0;
    for (const auto& sampleTimes : timeBase->getSampleTimes()) {
      if (sampleTimes.isCalibrated()) ncal++;
    }

    B2RESULT("Sample time calibration constants imported to database, calibrated channels: "
             << ncal << "/" << nall);
  }


  void TOPDatabaseImporter::importLocalT0Calibration(string fNames,
                                                     int firstExp, int firstRun,
                                                     int lastExp, int lastRun)
  {
    vector<string> fileNames;
    stringstream ss(fNames);
    string fName;
    while (ss >> fName) {
      fileNames.push_back(fName);
    }

    const auto* geo = TOPGeometryPar::Instance()->getGeometry();

    DBImportObjPtr<TOPCalChannelT0> channelT0;
    channelT0.construct();

    int nCal[16] = {0}; // number of calibrated channels per slot

    for (const auto& fileName : fileNames) {
      TFile* file = TFile::Open(fileName.c_str(), "r");
      B2INFO("--> Opening constants file " << fileName);

      if (!file) {
        B2ERROR("openFile: " << fileName << " *** failed to open");
        continue;
      }
      B2INFO("--> " << fileName << ": open for reading");

      TTree* treeCal = (TTree*)file->Get("chT0");

      if (!treeCal) {
        B2ERROR("openFile: no tree named chT0 found in " << fileName);
        file->Close();
        continue;
      }

      double t0Cal = 0.;
      double t0CalErr = 0.;
      int channelID = 0; // 0-511
      int slotID = 0;    // 1-16
      int fitStatus = 0; // fit status, 0 = OK

      treeCal->SetBranchAddress("channel", &channelID);
      treeCal->SetBranchAddress("slot", &slotID);
      treeCal->SetBranchAddress("channelT0", &t0Cal);
      treeCal->SetBranchAddress("channelT0Err", &t0CalErr);
      treeCal->SetBranchAddress("fitStatus", &fitStatus);

      B2INFO("--> importing constats");

      for (int iCal = 0; iCal < treeCal->GetEntries(); iCal++) {
        treeCal->GetEntry(iCal);
        if (!geo->isModuleIDValid(slotID)) {
          B2ERROR("Slot ID is not valid (fileName = " << fileName
                  << ", SlotID = " << slotID << ", ChannelID = " << channelID <<
                  "). Skipping the entry.");
          continue;
        }
        if (channelID < 0 or channelID > 511) {
          B2ERROR("Channel ID is not valid (fileName = " << fileName
                  << ", SlotID = " << slotID << ", ChannelID = " << channelID <<
                  "). Skipping the entry.");
          continue;
        }
        channelT0->setT0(slotID, channelID, t0Cal, t0CalErr);
        if (fitStatus == 0) {
          nCal[slotID - 1]++;
        } else {
          channelT0->setUnusable(slotID, channelID);
        }
      }

      file->Close();
      B2INFO("--> Input file closed");
    }
    channelT0->suppressAverage();

    IntervalOfValidity iov(firstExp, firstRun, lastExp, lastRun);
    channelT0.import(iov);

    short nCalTot = 0;
    B2INFO("Summary: ");
    for (int iSlot = 1; iSlot < 17; iSlot++) {
      B2INFO("--> Number of calibrated channels on Slot " << iSlot << " : " << nCal[iSlot - 1] << "/512");
      B2INFO("--> Cal on ch 1, 256 and 511:    " << channelT0->getT0(iSlot, 0) << ", " << channelT0->getT0(iSlot,
             257) << ", " << channelT0->getT0(iSlot, 511));
      nCalTot += nCal[iSlot - 1];
    }


    B2RESULT("Channel T0 calibration constants imported to database, calibrated channels: "
             << nCalTot << "/ 8192");
  }


  void TOPDatabaseImporter::importChannelT0(std::string fileName,
                                            int expNo, int firstRun, int lastRun)
  {
    // declare db object to be imported -- and construct it
    DBImportObjPtr<TOPCalChannelT0> channelT0;
    channelT0.construct();

    // open the root file
    TFile* file = TFile::Open(fileName.c_str(), "r");
    if (!file) {
      B2ERROR("openFile: " << fileName << " *** failed to open");
      return;
    }
    B2INFO(fileName << ": open for reading");

    // loop over slots and set channel T0
    int nModules = TOPGeometryPar::Instance()->getGeometry()->getNumModules();
    int count = 0; // counter of calibrated constants
    for (int moduleID = 1; moduleID <= nModules; moduleID++) {
      std::string name = "channelT0_slot";
      if (moduleID < 10) name += "0";
      name += std::to_string(moduleID);
      auto* h = (TH1F*) file->Get(name.c_str());
      if (!h) {
        B2ERROR("Histogram with name '" + name + "' not found");
        continue;
      }
      for (int channel = 0; channel < h->GetNbinsX(); channel++) {
        double value = h->GetBinContent(channel + 1);
        double error = h->GetBinError(channel + 1);
        channelT0->setT0(moduleID, channel, value, error);
        if (error > 0) {
          count++;
        } else {
          channelT0->setUnusable(moduleID, channel);
        }
      }
    }
    file->Close();

    channelT0->suppressAverage();

    // import to database
    IntervalOfValidity iov(expNo, firstRun, expNo, lastRun);
    channelT0.import(iov);

    B2INFO("Channel T0 for exp " << expNo << " run " << firstRun << " to " << lastRun
           << " imported. Calibrated channels: " << count << "/" << nModules * 512);

  }


  void TOPDatabaseImporter::importAsicShifts_BS13d(double s0, double s1, double s2, double s3,
                                                   int expNo, int firstRun, int lastRun)
  {

    std::vector<double> shifts;
    shifts.push_back(s0);
    shifts.push_back(s1);
    shifts.push_back(s2);
    shifts.push_back(s3);

    DBImportObjPtr<TOPCalAsicShift> asicShift;
    asicShift.construct();

    int moduleID = 13;
    unsigned bs = 3;
    for (unsigned carrier = 0; carrier < 4; carrier++) {
      for (unsigned a = 0; a < 4; a++) {
        unsigned asic = a + carrier * 4 + bs * 16;
        asicShift->setT0(moduleID, asic, shifts[carrier]);
      }
    }

    IntervalOfValidity iov(expNo, firstRun, expNo, lastRun);
    asicShift.import(iov);

    B2INFO("ASIC shifts of BS13d imported for exp " << expNo << " run " << firstRun <<
           " to " << lastRun);
  }


  void TOPDatabaseImporter::importOfflineCommonT0Calibration(string fileName,
                                                             int firstExp, int firstRun,
                                                             int lastExp, int lastRun)
  {
    TFile* file = TFile::Open(fileName.c_str(), "r");
    B2INFO("--> Opening constants file " << fileName);

    if (!file) {
      B2ERROR("openFile: " << fileName << " *** failed to open");
      return;
    }
    B2INFO("--> " << fileName << ": open for reading");

    TTree* treeCal = (TTree*)file->Get("tree");

    if (!treeCal) {
      B2ERROR("openFile: no tree named tree found in " << fileName);
      file->Close();
      return;
    }

    float t0 = 0.;
    float t0Err = 0;
    float chi2 = 0;
    float integral = 0;
    float sigma = 0;
    int runNum = 0;
    int fitStatus = 0;

    treeCal->SetBranchAddress("offset", &t0);
    treeCal->SetBranchAddress("runNum", &runNum);
    treeCal->SetBranchAddress("sigma", &sigma);
    treeCal->SetBranchAddress("offsetErr", &t0Err);
    treeCal->SetBranchAddress("chi2", &chi2);
    treeCal->SetBranchAddress("integral", &integral);
    treeCal->SetBranchAddress("fitStatus", &fitStatus);

    treeCal->GetEntry(0);

    if (lastRun == -1 and firstRun == -1) {
      lastRun = runNum;
      firstRun = runNum;
      B2INFO("Using the run number from the tree ");
    } else {
      B2INFO("Using the run numbers passed to the importer");
    }
    B2INFO("IOV = (" << firstExp << ", "  << firstRun << ", "
           << lastExp << ", "  << lastRun << ")");

    DBImportObjPtr<TOPCalCommonT0> commonT0;
    commonT0.construct(t0, t0Err);

    if (fitStatus == 0 and integral > 10 and sigma > 0.05 and sigma < 0.33) {
      B2INFO("Good calibration found ");
      B2INFO("t0 = " << t0 << " +- "  << t0Err);
      B2INFO("sigma = " << sigma);
      B2INFO("chi2 = " << chi2);
    } else {
      B2INFO("BAD calibration found - set calibration to 'unusable'");
      B2INFO("t0 = " << t0 << " +- "  << t0Err);
      B2INFO("sigma = " << sigma);
      B2INFO("chi2 = " << chi2);
      B2INFO("fit status = " << fitStatus);
      commonT0->setUnusable();
    }

    file->Close();

    IntervalOfValidity iov(firstExp, firstRun, lastExp, lastRun);
    commonT0.import(iov);
    B2INFO("--> constants imported");
    B2INFO("   ");
  }

  void TOPDatabaseImporter::importCommonT0(double value, double error,
                                           int expNo, int firstRun, int lastRun,
                                           bool roughlyCalibrated)
  {
    DBImportObjPtr<TOPCalCommonT0> commonT0;
    commonT0.construct(value, error);
    if (roughlyCalibrated) commonT0->setRoughlyCalibrated();

    IntervalOfValidity iov(expNo, firstRun, expNo, lastRun);
    commonT0.import(iov);

    B2INFO("--> constants for exp = " << expNo
           << " run = " << firstRun << " to " << lastRun << " imported");
  }

  void TOPDatabaseImporter::importModuleT0Calibration(string fileName,
                                                      int firstExp, int firstRun,
                                                      int lastExp, int lastRun)
  {


    DBImportObjPtr<TOPCalModuleT0> moduleT0;
    moduleT0.construct();


    ifstream inFile(fileName);
    B2INFO("--> Opening constants file " << fileName);

    if (!inFile) {
      B2ERROR("openFile: " << fileName << " *** failed to open");
      return;
    }
    B2INFO("--> " << fileName << ": open for reading");


    B2INFO("--> importing constants");

    while (!inFile.eof()) {
      int slot = 0;
      int dummy = 0;
      double T0 = 0;
      double T0_err = 0;

      inFile >> slot >> dummy >> T0 >> T0_err;
      if (slot < 1 or slot > 16) {
        B2ERROR("Module ID is not valid. Skipping the entry.");
        continue;
      }
      moduleT0->setT0(slot, T0, T0_err);

    }
    inFile.close();
    B2INFO("--> Input file closed");

    moduleT0->suppressAverage();

    IntervalOfValidity iov(firstExp, firstRun, lastExp, lastRun);
    moduleT0.import(iov);

    B2INFO("Summary: ");
    for (int iSlot = 1; iSlot < 17; iSlot++) {
      B2INFO("--> Time offset of Slot " << iSlot << " = " << moduleT0->getT0(iSlot));
    }


  }


  void TOPDatabaseImporter::importModuleT0(std::string fileName,
                                           int expNo, int firstRun, int lastRun)
  {

    // construct DB import object
    DBImportObjPtr<TOPCalModuleT0> moduleT0;
    moduleT0.construct();

    // open the root file
    TFile* file = TFile::Open(fileName.c_str(), "r");
    if (!file) {
      B2ERROR("openFile: " << fileName << " *** failed to open");
      return;
    }
    B2INFO(fileName << ": open for reading");

    // get histogram and set the DB import object
    auto* h = (TH1F*) file->Get("moduleT0");
    if (not h) {
      B2ERROR("no histogram 'moduleT0' found in the file, nothing imported");
      return;
    }
    int count = 0; // counter of calibrated
    for (int slot = 1; slot <= h->GetNbinsX(); slot++) {
      double value = h->GetBinContent(slot);
      double error = h->GetBinError(slot);
      moduleT0->setT0(slot, value, error);
      if (error > 0) {
        count++;
      } else {
        moduleT0->setUnusable(slot);
      }
    }
    file->Close();

    moduleT0->suppressAverage();

    // import the object
    IntervalOfValidity iov(expNo, firstRun, expNo, lastRun);
    moduleT0.import(iov);

    B2INFO("Module T0 for exp " << expNo << " run " << firstRun << " to " << lastRun
           << " imported. Calibrated modules: " << count << "/" << 16);

  }


  void TOPDatabaseImporter::getSampleTimeCalibrationInfo()
  {
    DBObjPtr<TOPCalTimebase> timeBase;
    if (!timeBase.isValid()) {
      B2ERROR("No time base calibration available");
      return;
    }

    const auto* geo = TOPGeometryPar::Instance()->getGeometry();
    int numModules = geo->getNumModules();
    auto& feMapper = TOPGeometryPar::Instance()->getFrontEndMapper();

    cout << "Time base calibration: number of calibrated channels in database" << endl << endl;
    for (int moduleID = 1; moduleID <= numModules; moduleID++) {
      int ncal[4] = {0, 0, 0, 0};
      int scrodID[4] = {0, 0, 0, 0};
      for (int bs = 0; bs < 4; bs++) {
        auto* femap = feMapper.getMap(moduleID, bs);
        if (!femap) {
          B2ERROR("No FrontEnd map available for boardstack " << bs << " of module " << moduleID);
          continue;
        }
        scrodID[bs] = femap->getScrodID();
        for (int channel = 0; channel < 128; channel++) {
          if (timeBase->isAvailable(scrodID[bs], channel)) ncal[bs]++;
        }
      }
      if (ncal[0] + ncal[1] + ncal[2] + ncal[3] == 0) continue;

      cout << "Slot " << moduleID << endl;
      for (int bs = 0; bs < 4; bs++) {
        cout << "  scrodID " << scrodID[bs] << ": " << ncal[bs] << "/128" << endl;
      }
    }

    cout << endl;
  }


  void TOPDatabaseImporter::printSampleTimeCalibration()
  {

    DBObjPtr<TOPCalTimebase> timeBase;
    if (!timeBase.isValid()) {
      B2ERROR("No time base calibration available");
      return;
    }

    for (const auto& sampleTimes : timeBase->getSampleTimes()) {
      cout << sampleTimes.getScrodID() << " " << sampleTimes.getChannel() << endl;
      for (const auto& time : sampleTimes.getTimeAxis()) {
        cout << time << " ";
      }
      cout << endl;
    }

  }


  void TOPDatabaseImporter::importChannelMask(std::string fileName,
                                              int expNo, int firstRun, int lastRun)
  {
    // declare db object to be imported -- and construct it
    DBImportObjPtr<TOPCalChannelMask> channelMask;
    channelMask.construct();

    // open the root file
    TFile* file = TFile::Open(fileName.c_str(), "r");
    if (!file) {
      B2ERROR("openFile: " << fileName << " *** failed to open");
      return;
    }
    B2INFO(fileName << ": open for reading");

    // loop over slots and set channel mask
    int nModules = TOPGeometryPar::Instance()->getGeometry()->getNumModules();
    int active = 0, dead = 0, noisy = 0;
    for (int moduleID = 1; moduleID <= nModules; moduleID++) {
      std::string name = "slot_" + std::to_string(moduleID);
      auto* h = (TH1F*) file->Get(name.c_str());
      if (!h) {
        B2ERROR("Histogram with name '" + name + "' not found");
        continue;
      }
      for (int channel = 0; channel < h->GetNbinsX(); channel++) {
        int value = h->GetBinContent(channel + 1);
        if (value == 0) {
          channelMask->setActive(moduleID, channel);
          active++;
        } else if (value == 1) {
          channelMask->setDead(moduleID, channel);
          dead++;
        } else {
          channelMask->setNoisy(moduleID, channel);
          noisy++;
        }
      }
    }
    file->Close();

    // import to database
    IntervalOfValidity iov(expNo, firstRun, expNo, lastRun);
    channelMask.import(iov);

    B2INFO("Channel mask for exp " << expNo << " run " << firstRun << " to " << lastRun
           << " imported. Active channels: " << active << ", dead: " << dead
           << ", noisy: " << noisy);

  }


  void TOPDatabaseImporter::generateFakeChannelMask(double fractionDead,
                                                    double fractionHot,
                                                    int firstExp, int firstRun,
                                                    int lastExp, int lastRun)
  {
    // declare db object to be imported -- and construct it
    DBImportObjPtr<TOPCalChannelMask> channelMask;
    channelMask.construct();

    // set up for loop channel mapper
    auto& chMapper = TOPGeometryPar::Instance()->getChannelMapper();
    const size_t nModules = TOPGeometryPar::Instance()->getGeometry()->getNumModules();
    unsigned ncall = 0;
    unsigned nall = 0;

    // loop over module (1-based)
    for (size_t moduleID = 1; moduleID <= nModules; moduleID++) {

      // loop over boardStack*carrierBoard*assic*channel to get channel (0 to 512)
      // TODO: get these loop limits from some sensible enum somewhere
      for (int boardStack = 0; boardStack < 4; boardStack++) {
        for (int carrierBoard = 0; carrierBoard < 4; carrierBoard++) {
          for (int asic = 0; asic < 4; asic++) {
            for (int chan = 0; chan < 8; chan++) {
              auto channel = chMapper.getChannel(boardStack, carrierBoard, asic, chan);
              nall++;
              if (gRandom->Rndm() < fractionDead) {
                channelMask->setDead(moduleID, channel);
                ncall++;
              }
              if (gRandom->Rndm() < fractionHot) {
                channelMask->setNoisy(moduleID, channel);
                ncall++;
              }
            }
          }
        }
      }
    } // module

    // declare interval of validity
    IntervalOfValidity iov(firstExp, firstRun, lastExp, lastRun);
    channelMask.import(iov);

    B2RESULT("Generated and imported a fake channel mask to database for testing: "
             << ncall << "/" << nall);
    return;
  }


  void TOPDatabaseImporter::importPmtQEData(string fileName, string treeName,
                                            int firstExp, int firstRun,
                                            int lastExp, int lastRun)
  {

    // declare db objects to be imported
    DBImportArray<TOPPmtQE> pmtQEs;

    static const int nChann = 16;
    std::string* serialNum = 0;
    std::vector<float>* QE_data[nChann];
    float lambdaFirst, lambdaStep, collEff0, collEff;

    TBranch* bQE_data[nChann];

    // open root file and get tree
    TFile* file = TFile::Open(fileName.c_str(), "r");
    if (!file) {
      B2ERROR("Cannot open the file " << fileName);
      return;
    }
    TTree* tQeData = (TTree*)file->Get(treeName.c_str());
    if (!tQeData) {
      B2ERROR("No TTree with name " << treeName << " in file " << fileName);
      file->Close();
      return;
    }

    tQeData->SetBranchAddress("serialNum", &serialNum);
    tQeData->SetBranchAddress("lambdaFirst", &lambdaFirst);
    tQeData->SetBranchAddress("lambdaStep", &lambdaStep);
    tQeData->SetBranchAddress("collEff0", &collEff0);
    tQeData->SetBranchAddress("collEff", &collEff);

    for (int ic = 0; ic < nChann; ic++) {
      // must initialize vectors and branches
      QE_data[ic] = new std::vector<float>;
      bQE_data[ic] = new TBranch();

      TString cString = "QE_ch";
      cString += ic + 1;
      tQeData->SetBranchAddress(cString, &QE_data[ic], &bQE_data[ic]);
    }

    // loop on input tree entries and construct the pmtQE objects
    int countPMTs = 0;

    for (int ient = 0; ient < tQeData->GetEntries(); ient++) {

      tQeData->GetEntry(ient);

      auto* pmtQE = pmtQEs.appendNew(*serialNum, lambdaFirst, lambdaStep, collEff0, collEff);

      for (int ic = 0; ic < nChann; ic++) {
        int tEntry = tQeData->LoadTree(ient);
        bQE_data[ic]->GetEntry(tEntry);

        pmtQE->setQE(ic + 1, *QE_data[ic]);
      }   // end loop on channels

      countPMTs++;
    }
    file->Close();

    IntervalOfValidity iov(firstExp, firstRun, lastExp, lastRun);
    pmtQEs.import(iov);

    B2RESULT("PMT QE data imported to database for " << countPMTs << " PMT's.");

    return;
  }


  void TOPDatabaseImporter::importPmtGainData(string fileName, string treeName,
                                              int firstExp, int firstRun,
                                              int lastExp, int lastRun)
  {

    // declare db objects to be imported
    DBImportArray<TOPPmtGainPar> pmtGains;

    static const int nChann = 16;
    std::string* serialNum = 0;
    float gain_const[nChann], gain_slope[nChann], gain_ratio[nChann];
    float hv_op0, hv_op;

    // open root file and get tree
    TFile* file = TFile::Open(fileName.c_str(), "r");
    if (!file) {
      B2ERROR("Cannot open the file " << fileName);
      return;
    }
    TTree* tGainData = (TTree*)file->Get(treeName.c_str());
    if (!tGainData) {
      B2ERROR("No TTree with name " << treeName << " in file " << fileName);
      file->Close();
      return;
    }

    tGainData->SetBranchAddress("serialNum", &serialNum);
    tGainData->SetBranchAddress("gain_const", &gain_const);
    tGainData->SetBranchAddress("gain_slope", &gain_slope);
    tGainData->SetBranchAddress("gain_ratio", &gain_ratio);
    tGainData->SetBranchAddress("hv_op0", &hv_op0);
    tGainData->SetBranchAddress("hv_op", &hv_op);


    // loop on input tree entries and construct the pmtGain objects
    int countPMTs = 0;

    for (int ient = 0; ient < tGainData->GetEntries(); ient++) {
      tGainData->GetEntry(ient);
      auto* pmtGain = pmtGains.appendNew(*serialNum);

      for (int ic = 0; ic < nChann; ic++) {
        pmtGain->setPmtPixelData(ic + 1, gain_const[ic], gain_slope[ic], gain_ratio[ic]);
        pmtGain->setNominalHV0(-fabs(hv_op0));
        pmtGain->setNominalHV(-fabs(hv_op));
      }
      countPMTs++;
    }
    file->Close();

    IntervalOfValidity iov(firstExp, firstRun, lastExp, lastRun);
    pmtGains.import(iov);

    B2RESULT("PMT gain data imported to database for " << countPMTs << " PMT's.");

    return;
  }


  void TOPDatabaseImporter::importPmtInstallationData(string fileName, string treeName ,
                                                      int firstExp, int firstRun,
                                                      int lastExp, int lastRun)
  {

    // declare db objects to be imported
    DBImportArray<TOPPmtInstallation> pmtInst;

    std::string* serialNum = 0;
    int moduleCNum, slotNum, arrayNum, PMTposition;
    TOPPmtObsoleteData::EType type;

    // open root file and get tree
    TFile* file = TFile::Open(fileName.c_str(), "r");
    if (!file) {
      B2ERROR("Cannot open the file " << fileName);
      return;
    }
    TTree* tInstData = (TTree*)file->Get(treeName.c_str());
    if (!tInstData) {
      B2ERROR("No TTree with name " << treeName << " in file " << fileName);
      file->Close();
      return;
    }

    tInstData->SetBranchAddress("serialNum", &serialNum);
    tInstData->SetBranchAddress("moduleCNum", &moduleCNum);
    tInstData->SetBranchAddress("slotNum", &slotNum);
    tInstData->SetBranchAddress("arrayNum", &arrayNum);
    tInstData->SetBranchAddress("PMTposition", &PMTposition);
    tInstData->SetBranchAddress("type", &type);

    // loop on input tree entries and construct the pmtInstallation objects
    int countPMTs = 0;

    for (int ient = 0; ient < tInstData->GetEntries(); ient++) {
      tInstData->GetEntry(ient);
      pmtInst.appendNew(*serialNum, moduleCNum, slotNum, arrayNum, PMTposition, type);
      countPMTs++;
    }
    file->Close();

    IntervalOfValidity iov(firstExp, firstRun, lastExp, lastRun);
    pmtInst.import(iov);

    B2RESULT("PMT installation data imported to database for " << countPMTs << " PMT's.");

  }


  void TOPDatabaseImporter::importPmtObsoleteData(string fileName, string treeName,
                                                  int firstExp, int firstRun,
                                                  int lastExp, int lastRun)
  {

    // declare db objects to be imported
    DBImportArray<TOPPmtObsoleteData> pmtObsData;

    std::string* serialNum = 0;
    std::string* cathode = 0;
    float hv_spec, dark_spec, qe380_spec;
    TOPPmtObsoleteData::EType type;

    // open root file and get tree
    TFile* file = TFile::Open(fileName.c_str(), "r");
    if (!file) {
      B2ERROR("Cannot open the file " << fileName);
      return;
    }
    TTree* tObsData = (TTree*)file->Get(treeName.c_str());
    if (!tObsData) {
      B2ERROR("No TTree with name " << treeName << " in file " << fileName);
      file->Close();
      return;
    }

    tObsData->SetBranchAddress("serialNum", &serialNum);
    tObsData->SetBranchAddress("cathode", &cathode);
    tObsData->SetBranchAddress("hv_spec", &hv_spec);
    tObsData->SetBranchAddress("dark_spec", &dark_spec);
    tObsData->SetBranchAddress("qe380_spec", &qe380_spec);
    tObsData->SetBranchAddress("type", &type);

    // loop on input tree entries and construct the pmt obsolete data objects
    int countPMTs = 0;

    for (int ient = 0; ient < tObsData->GetEntries(); ient++) {
      tObsData->GetEntry(ient);

      // make sure the HV from specifications is negative
      hv_spec = -fabs(hv_spec);

      pmtObsData.appendNew(*serialNum, type, *cathode, hv_spec, dark_spec, qe380_spec);
      countPMTs++;
    }

    IntervalOfValidity iov(firstExp, firstRun, lastExp, lastRun);
    pmtObsData.import(iov);

    B2RESULT("PMT obsolete data imported to database for " << countPMTs << " PMT's.");

    file->Close();

    delete serialNum;
    delete cathode;

    return;
  }


  void TOPDatabaseImporter::importPmtTTSPar(string fileName, string treeName,
                                            int firstExp, int firstRun,
                                            int lastExp, int lastRun)
  {

    // declare db objects to be imported
    DBImportArray<TOPPmtTTSPar> pmtTtsPars;

    static const int nChann = 16;
    std::string* serialNum  = 0;
    std::vector<float>* gausFrac[nChann];
    std::vector<float>* gausMean[nChann];
    std::vector<float>* gausSigma[nChann];

    TBranch* bGFrac[nChann];
    TBranch* bGMean[nChann];
    TBranch* bGSigma[nChann];


    // open root file and get tree
    TFile* file = TFile::Open(fileName.c_str(), "r");
    if (!file) {
      B2ERROR("Cannot open the file " << fileName);
      return;
    }
    TTree* tTtsPar = (TTree*)file->Get(treeName.c_str());
    if (!tTtsPar) {
      B2ERROR("No TTree with name " << treeName << " in file " << fileName);
      file->Close();
      return;
    }

    tTtsPar->SetBranchAddress("serialNum", &serialNum);
    for (int ic = 0; ic < nChann; ic++) {
      // must initialize vectors and branches
      gausFrac[ic] = new std::vector<float>;
      gausMean[ic] = new std::vector<float>;
      gausSigma[ic] = new std::vector<float>;

      bGFrac[ic] = new TBranch();
      bGMean[ic] = new TBranch();
      bGSigma[ic] = new TBranch();


      TString cStringF = "gausFrac_ch";
      TString cStringM = "gausMean_ch";
      TString cStringS = "gausSigma_ch";

      cStringF += ic + 1;
      cStringM += ic + 1;
      cStringS += ic + 1;

      tTtsPar->SetBranchAddress(cStringF, &gausFrac[ic], &bGFrac[ic]);
      tTtsPar->SetBranchAddress(cStringM, &gausMean[ic], &bGMean[ic]);
      tTtsPar->SetBranchAddress(cStringS, &gausSigma[ic], &bGSigma[ic]);
    }

    // loop on input tree entries and construct the pmt tts par objects
    int countPMTs = 0;

    for (int ient = 0; ient < tTtsPar->GetEntries(); ient++) {

      tTtsPar->GetEntry(ient);

      auto* pmtTtsPar = pmtTtsPars.appendNew(*serialNum);

      for (int ic = 0; ic < nChann; ic++) {

        int tEntry = tTtsPar->LoadTree(ient);
        bGFrac[ic]->GetEntry(tEntry);
        bGMean[ic]->GetEntry(tEntry);
        bGSigma[ic]->GetEntry(tEntry);

        // check that the vectors have the same size. Otherwise skip the channel
        if ((gausFrac[ic]->size() != gausMean[ic]->size()) ||
            (gausFrac[ic]->size() != gausSigma[ic]->size())) {

          B2ERROR("The TTSPar vectors for PMT " << serialNum << ", channel " << ic + 1 << " have different sizes! Skipping channel...");
          continue;
        }

        for (uint iv = 0; iv < gausFrac[ic]->size(); iv++) {
          pmtTtsPar->appendGaussian(ic + 1,
                                    gausFrac[ic]->at(iv),
                                    gausMean[ic]->at(iv),
                                    gausSigma[ic]->at(iv));
        }
      }
      countPMTs++;
    }
    file->Close();

    IntervalOfValidity iov(firstExp, firstRun, lastExp, lastRun);
    pmtTtsPars.import(iov);

    B2RESULT("PMT TTS parameters imported to database for " << countPMTs << " PMT's.");

    return;
  }


  void TOPDatabaseImporter::importPmtTTSHisto(string fileName,
                                              string treeName,
                                              int firstExp, int firstRun,
                                              int lastExp, int lastRun)
  {

    // declare db objects to be imported
    DBImportArray<TOPPmtTTSHisto> pmtTtsHistos;

    static const int nChann = 16;
    std::string* serialNum = 0;
    float hv = 0;
    TH1F* histo[nChann] = {0};

    // open root file and get tree
    TFile* file = TFile::Open(fileName.c_str(), "r");
    if (!file) {
      B2ERROR("Cannot open the file " << fileName);
      return;
    }
    TTree* tTtsHisto = (TTree*)file->Get(treeName.c_str());
    if (!tTtsHisto) {
      B2ERROR("No TTree with name " << treeName << " in file " << fileName);
      file->Close();
      return;
    }

    tTtsHisto->SetBranchAddress("serialNum", &serialNum);
    tTtsHisto->SetBranchAddress("hv", &hv);
    for (int ic = 0; ic < nChann; ic++) {
      TString hString = "hist_ch";
      hString += ic + 1;
      tTtsHisto->SetBranchAddress(hString, &histo[ic]);
    }

    // loop on input tree entries and construct the pmt tts histo objects
    int countHists = 0;

    for (int ient = 0; ient < tTtsHisto->GetEntries(); ient++) {

      tTtsHisto->GetEntry(ient);

      // make sure the HV used in the test is negative
      hv = -fabs(hv);

      B2INFO("Saving TTS histograms for PMT " << *serialNum << ", HV = " << hv);

      auto* pmtTtsHisto = pmtTtsHistos.appendNew(*serialNum, hv);
      for (int ic = 0; ic < nChann; ic++) {
        pmtTtsHisto->setHistogram(ic + 1, histo[ic]);
      }
      countHists++;
    }
    file->Close();

    IntervalOfValidity iov(firstExp, firstRun, lastExp, lastRun);
    pmtTtsHistos.import(iov);

    B2RESULT("Imported " << countHists << " sets of TTS histograms from " << fileName << " file.");

    return;
  }

  void TOPDatabaseImporter::importPmtPulseHeightFitResult(std::string fileName,
                                                          int firstExp, int firstRun,
                                                          int lastExp, int lastRun)
  {
    // declare db objects to be imported
    DBImportObjPtr<TOPCalChannelPulseHeight> calChannelPulseHeight;
    DBImportObjPtr<TOPCalChannelThresholdEff> calChannelThresholdEff;
    calChannelPulseHeight.construct();
    calChannelThresholdEff.construct();

    TFile* file = TFile::Open(fileName.c_str());
    if (!file) {
      B2ERROR("openFile: " << fileName << " *** failed to open");
      return;
    }
    TTree* tr = (TTree*)file->Get("tree");   // defined in TOPGainEfficiencyCalculatorModule
    if (!tr) {
      B2ERROR("No TTree with name tree found in " << fileName);
      file->Close();
      return;
    }

    short slotId = 0;
    short pixelId = 0;
    float p1 = -1;
    float p2 = -1;
    float x0 = -1;
    float threshold = -1;
    float efficiency = -1;
    float chisquare = -1;
    int ndf = 0;
    tr->SetBranchAddress("slotId", &slotId);
    tr->SetBranchAddress("pixelId", &pixelId);
    tr->SetBranchAddress("p1UseIntegral", &p1);
    tr->SetBranchAddress("p2UseIntegral", &p2);
    tr->SetBranchAddress("x0UseIntegral", &x0);
    tr->SetBranchAddress("thresholdForIntegral", &threshold);
    tr->SetBranchAddress("efficiencyUseIntegral", &efficiency);
    tr->SetBranchAddress("chisquareUseIntegral", &chisquare);
    tr->SetBranchAddress("ndfUseIntegral", &ndf);

    const auto& channelMapper = TOPGeometryPar::Instance()->getChannelMapper();
    if (!channelMapper.isValid()) {
      B2ERROR("No valid channel mapper found");
      file->Close();
      return;
    }

    long nEntries = tr->GetEntries();
    std::map<short, float> reducedChisqMap;
    for (long iEntry = 0 ; iEntry < nEntries ; iEntry++) {
      tr->GetEntry(iEntry);

      if (efficiency < 0) continue;

      if (!channelMapper.isPixelIDValid(pixelId)) {
        B2ERROR("invalid pixelID" << pixelId);
        continue;
      }
      auto channel = channelMapper.getChannel(pixelId);
      short globalChannelNumber = slotId * 1000 + channel;
      float redChisq = chisquare / ndf;

      //in case entries for the same channel appears multiple time, use data with smaller reduced chisquare
      //(This can happen when distribution is fit manually and results are appended for channels with fit failure)
      if (reducedChisqMap.count(globalChannelNumber) == 0
          or reducedChisqMap[globalChannelNumber] > redChisq) {
        reducedChisqMap[globalChannelNumber] = redChisq;
        calChannelPulseHeight->setParameters(slotId, channel, x0, p1, p2);
        calChannelThresholdEff->setThrEff(slotId, channel, efficiency, (short)threshold);

        if (redChisq > 10.) {
          calChannelPulseHeight->setUnusable(slotId, channel);
          calChannelThresholdEff->setUnusable(slotId, channel);
        }
      }
    }
    file->Close();

    IntervalOfValidity iov(firstExp, firstRun, lastExp, lastRun);
    calChannelPulseHeight.import(iov);
    calChannelThresholdEff.import(iov);

    B2RESULT("Imported channel-by-channel gain and efficiency data from fitting of pulse height distribution for "
             << reducedChisqMap.size() << " channels from " << fileName << " file.");

    return;
  }


  void TOPDatabaseImporter::exportPmtTTSHisto(string outFileName)
  {

    // this is just an example on how to retrieve TTS histograms
    DBArray<TOPPmtTTSHisto> elements("TOPPmtTTSHistos");

    TFile file(outFileName.c_str(), "recreate");

    // prints serialNum of PMTs and hv setting used, and saves TTS histograms to root file
    for (const auto& element : elements) {

      B2INFO("serialNum = " << element.getSerialNumber() << ", HV = " << element.getHV());
      for (int ic = 0; ic < element.getNumOfPixels(); ic++) {
        const auto* ttsHisto = element.getHistogram(ic + 1);
        if (ttsHisto) ttsHisto->Write();
      }
    }

    file.Close();

    return;
  }

  void TOPDatabaseImporter::importFrontEndSettings(int lookback, int readoutWin,
                                                   int extraWin, int offset,
                                                   int expNo, int firstRun, int lastRun)
  {
    DBImportObjPtr<TOPFrontEndSetting> feSetting;
    feSetting.construct();

    // write-window depths (write-window is 128 samples)
    std::vector<int> writeDepths;
    for (int i = 0; i < 3; i++) {
      writeDepths.push_back(214);
      writeDepths.push_back(212);
      writeDepths.push_back(214);
    }
    feSetting->setWriteDepths(writeDepths);
    feSetting->setLookbackWindows(lookback);
    feSetting->setReadoutWindows(readoutWin);
    feSetting->setExtraWindows(extraWin);
    feSetting->setOffset(offset);

    // window shifts
    std::vector<int> shifts = {0, 0, 1, 1, 1, 2};
    feSetting->setWindowShifts(shifts);

    IntervalOfValidity iov(expNo, firstRun, expNo, lastRun);
    feSetting.import(iov);

    B2INFO("Front-end settings imported for exp " << expNo << " run " << firstRun <<
           " to " << lastRun);
  }


  void TOPDatabaseImporter::importDummyCalModuleAlignment(int firstExp, int firstRun,
                                                          int lastExp, int lastRun)
  {
    IntervalOfValidity iov(firstExp, firstRun, lastExp, lastRun);
    DBImportObjPtr<TOPCalModuleAlignment> moduleAlignment;
    moduleAlignment.construct();
    moduleAlignment.import(iov);
    B2INFO("Dummy TOPCalModuleAlignment imported");
    return;
  }

  void TOPDatabaseImporter::importDummyCalModuleT0(int firstExp, int firstRun,
                                                   int lastExp, int lastRun)
  {
    IntervalOfValidity iov(firstExp, firstRun, lastExp, lastRun);
    DBImportObjPtr<TOPCalModuleT0> moduleT0;
    moduleT0.construct();
    moduleT0.import(iov);
    B2INFO("Dummy TOPCalModuleT0 imported");
    return;
  }

  void TOPDatabaseImporter::importDummyCalChannelT0(int firstExp, int firstRun,
                                                    int lastExp, int lastRun)
  {
    IntervalOfValidity iov(firstExp, firstRun, lastExp, lastRun);
    DBImportObjPtr<TOPCalChannelT0> channelT0;
    channelT0.construct();
    channelT0.import(iov);
    B2INFO("Dummy TOPCalChannelT0 imported");
    return;
  }

  void TOPDatabaseImporter::importDummyCalTimebase(int firstExp, int firstRun,
                                                   int lastExp, int lastRun)
  {
    IntervalOfValidity iov(firstExp, firstRun, lastExp, lastRun);
    DBImportObjPtr<TOPCalTimebase> timebase;
    timebase.construct();
    timebase.import(iov);
    B2INFO("Dummy TOPCalTimebase imported");
    return;
  }

  void TOPDatabaseImporter::importDummyCalChannelNoise(int firstExp, int firstRun,
                                                       int lastExp, int lastRun)
  {
    IntervalOfValidity iov(firstExp, firstRun, lastExp, lastRun);
    DBImportObjPtr<TOPCalChannelNoise> channelNoise;
    channelNoise.construct();
    channelNoise.import(iov);
    B2INFO("Dummy TOPCalChannelNoise imported");
    return;
  }

  void TOPDatabaseImporter::importDummyCalChannelPulseHeight(int firstExp, int firstRun,
                                                             int lastExp, int lastRun)
  {
    IntervalOfValidity iov(firstExp, firstRun, lastExp, lastRun);
    DBImportObjPtr<TOPCalChannelPulseHeight> pulseHeight;
    pulseHeight.construct();
    pulseHeight.import(iov);
    B2INFO("Dummy TOPCalChannelPulseHeight imported");
    return;
  }

  void TOPDatabaseImporter::importDummyCalChannelRQE(int firstExp, int firstRun,
                                                     int lastExp, int lastRun)
  {
    IntervalOfValidity iov(firstExp, firstRun, lastExp, lastRun);
    DBImportObjPtr<TOPCalChannelRQE> channelRQE;
    channelRQE.construct();
    channelRQE.import(iov);
    B2INFO("Dummy TOPCalChannelRQE imported");
    return;
  }

  void TOPDatabaseImporter::importDummyCalChannelThresholdEff(int firstExp, int firstRun,
                                                              int lastExp, int lastRun)
  {
    IntervalOfValidity iov(firstExp, firstRun, lastExp, lastRun);
    DBImportObjPtr<TOPCalChannelThresholdEff> channelThresholdEff;
    channelThresholdEff.construct();
    channelThresholdEff.import(iov);
    B2INFO("Dummy TOPCalChannelThresholdEff imported");
    return;
  }

  void TOPDatabaseImporter::importDummyCalChannelThreshold(int firstExp, int firstRun,
                                                           int lastExp, int lastRun)
  {
    IntervalOfValidity iov(firstExp, firstRun, lastExp, lastRun);
    DBImportObjPtr<TOPCalChannelThreshold> channelThreshold;
    channelThreshold.construct();
    channelThreshold.import(iov);
    B2INFO("Dummy TOPCalChannelThreshold imported");
    return;
  }

  void TOPDatabaseImporter::importDummyCalCommonT0(int firstExp, int firstRun,
                                                   int lastExp, int lastRun)
  {
    IntervalOfValidity iov(firstExp, firstRun, lastExp, lastRun);
    DBImportObjPtr<TOPCalCommonT0> commonT0;
    commonT0.construct();
    commonT0.import(iov);
    B2INFO("Dummy TOPCalCommonT0 imported");
    return;
  }

  void TOPDatabaseImporter::importDummyCalIntegratedCharge(int firstExp, int firstRun,
                                                           int lastExp, int lastRun)
  {
    IntervalOfValidity iov(firstExp, firstRun, lastExp, lastRun);
    DBImportObjPtr<TOPCalIntegratedCharge> integratedCharge;
    integratedCharge.construct();
    integratedCharge.import(iov);
    B2INFO("Dummy TOPCalIntegratedCharge imported");
    return;
  }

  void TOPDatabaseImporter::importDummyCalAsicShift(int firstExp, int firstRun,
                                                    int lastExp, int lastRun)
  {
    IntervalOfValidity iov(firstExp, firstRun, lastExp, lastRun);
    DBImportObjPtr<TOPCalAsicShift> asicShift;
    asicShift.construct();
    asicShift.import(iov);
    B2INFO("Dummy TOPCalAsicShift imported");
  }

  void TOPDatabaseImporter::correctTOPPmtQE()
  {
    B2ERROR("Function disabled since the corrected payload TOPPmtQEs already imported");
    return;

    DBArray<TOPPmtQE> pmtQEData;
    DBImportArray<TOPPmtQE> pmtQECorrected;

    for (const auto& pmt : pmtQEData) {
      auto* pmtCorr = pmtQECorrected.appendNew(pmt.getSerialNumber(),
                                               pmt.getLambdaFirst(),
                                               pmt.getLambdaStep(),
                                               pmt.getCE(false),
                                               pmt.getCE(true));
      for (unsigned pmtPixel = 1; pmtPixel <= TOPPmtQE::c_NumPmtPixels; pmtPixel++) {
        auto qeData = pmt.getQE(pmtPixel);
        float lambda = pmt.getLambdaFirst();
        float step = pmt.getLambdaStep();
        for (auto& qe : qeData) {
          double n = phind_lambda_(&lambda); // phase refractive index of quartz
          double reflectance = pow((n - 1) / (n + 1), 2);
          qe /= (1 - reflectance);
          lambda += step;
        }
        pmtCorr->setQE(pmtPixel, qeData);
      }
    }

    IntervalOfValidity iov(0, 0, -1, -1);
    pmtQECorrected.import(iov);

    B2RESULT("Corrected PMT QE data imported to database for "
             << pmtQECorrected.getEntries() << " PMT's.");

  }


  void TOPDatabaseImporter::importTimeWalk(PyObject* list, double a, double b,
                                           int firstExp, int firstRun,
                                           int lastExp, int lastRun)
  {

    std::vector<double> params;
    if (PyList_Check(list)) {
      for (Py_ssize_t i = 0; i < PyList_Size(list); i++) {
        PyObject* value = PyList_GetItem(list, i);
        params.push_back(PyFloat_AsDouble(value));
        B2INFO(i << " " << params.back());
      }
    } else {
      B2ERROR("Input Python object is not a list");
      return;
    }

    DBImportObjPtr<TOPCalTimeWalk> timeWalk;
    timeWalk.construct();
    timeWalk->set(params, a, b);

    IntervalOfValidity iov(firstExp, firstRun, lastExp, lastRun);
    timeWalk.import(iov);

    B2RESULT("Time-walk constants imported");
  }



//---- for testing only -- will be removed --------------------------------

  void TOPDatabaseImporter::importTest(int runNumber, double syncTimeBase)
  {

    DBImportObjPtr<TOPCalTimebase> timeBase;
    vector<double> timeAxis;
    for (int i = 0; i < 256; i++) {
      timeAxis.push_back(syncTimeBase / 128.0 * i);
    }


    timeBase.construct(syncTimeBase);
    for (unsigned scrodID = 0; scrodID < 64; scrodID++) {
      for (unsigned channel = 0; channel < 128; channel++) {
        timeBase->append(scrodID, channel, timeAxis);
      }
    }

    if (runNumber == 3) {
      timeBase.addEventDependency(10);
      timeBase.construct(syncTimeBase + 100);
      for (unsigned scrodID = 0; scrodID < 64; scrodID++) {
        for (unsigned channel = 0; channel < 128; channel++) {
          timeBase->append(scrodID, channel, timeAxis);
        }
      }
    }

    IntervalOfValidity iov(1, runNumber, 1, runNumber);
    timeBase.import(iov);

  }


  void TOPDatabaseImporter::importTest()
  {

    DBImportArray<TOPPmtGainPar> pmtGains;

    auto* pmtGain = pmtGains.appendNew("JT00123");
    pmtGain->setNominalHV(3520);
    for (unsigned channel = 1; channel <= 16; channel++) {
      pmtGain->setPmtPixelData(channel, -13.77, 0.0042, 0.4);
    }

    pmtGain = pmtGains.appendNew("JT02135");
    pmtGain->setNominalHV(3450);
    for (unsigned channel = 1; channel <= 16; channel++) {
      pmtGain->setPmtPixelData(channel, -12.77, 0.0045, 0.4);
    }

    for (const auto& gain : pmtGains) gain.print();

    //  IntervalOfValidity iov(0, 0, -1, -1); // all experiments and runs
    //  pmtGains.import(iov);


  }


} // end Belle2 namespace

