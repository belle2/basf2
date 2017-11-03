/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <top/calibration/TOPDatabaseImporter.h>
#include <top/geometry/TOPGeometryPar.h>

// framework - core
#include <framework/core/RandomGenerator.h> // gRandom

// framework - Database
#include <framework/database/Database.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBImportArray.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/database/DBArray.h>
#include <framework/database/DBObjPtr.h>

// framework - xml
#include <framework/gearbox/GearDir.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// DB objects
#include <top/dbobjects/TOPCalTimebase.h>
#include <top/dbobjects/TOPCalChannelT0.h>
#include <top/dbobjects/TOPCalChannelMask.h>
#include <top/dbobjects/TOPPmtGainPar.h>
#include <top/dbobjects/TOPPmtQE.h>
#include <top/dbobjects/TOPPmtInstallation.h>
#include <top/dbobjects/TOPPmtObsoleteData.h>
#include <top/dbobjects/TOPPmtTTSPar.h>
#include <top/dbobjects/TOPPmtTTSHisto.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <set>

#include "TFile.h"
#include "TTree.h"


using namespace std;

namespace Belle2 {

  using namespace TOP;


  void TOPDatabaseImporter::importSampleTimeCalibration(string fNames)
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
      TFile* file = new TFile(fileName.c_str(), "r");
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
    IntervalOfValidity iov(0, 0, -1, -1); // all experiments and runs
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


  void TOPDatabaseImporter::importLocalT0Calibration(string fNames)
  {
    vector<string> fileNames;
    stringstream ss(fNames);
    string fName;
    while (ss >> fName) {
      fileNames.push_back(fName);
    }

    auto& chMapper = TOPGeometryPar::Instance()->getChannelMapper();
    const auto* geo = TOPGeometryPar::Instance()->getGeometry();

    DBImportObjPtr<TOPCalChannelT0> channelT0;
    channelT0.construct();

    int nCal[16] = {0}; // number of calibrated channels per slot

    for (const auto& fileName : fileNames) {
      TFile* file = new TFile(fileName.c_str(), "r");
      if (!file) {
        B2ERROR("openFile: " << fileName << " *** failed to open");
        continue;
      }
      B2INFO("--> " << fileName << ": open for reading");

      TTree* treeCal = (TTree*)file->Get("chT0");

      if (!treeCal) {
        B2ERROR("openFile: no tree named chT0 found in " << fileName);
        continue;
      }

      double t0Cal = 0.;
      int channelID = 0; // 1-512
      int slotID = 0;    // 1-16

      treeCal->SetBranchAddress("t0_const", &t0Cal);
      treeCal->SetBranchAddress("channel", &channelID);
      treeCal->SetBranchAddress("slot", &slotID);

      B2INFO("--> importing constats ");

      for (int iCal = 0; iCal < treeCal->GetEntries(); iCal++) {
        treeCal->GetEntry(iCal);
        if (!geo->isModuleIDValid(slotID)) {
          B2ERROR("Slot ID is not valid (fileName = " << fileName << ", SlotID = " << slotID << ", ChannelID = " << channelID <<
                  "). Skipping the entry.");
          continue;
        }
        if (channelID < 1 || channelID > 512) {
          B2ERROR("Channel ID is not valid (fileName = " << fileName << ", SlotID = " << slotID << ", ChannelID = " << channelID <<
                  "). Skipping the entry.");
          continue;
        }
        auto channel = chMapper.getChannel(channelID);
        double err = 0.; // No error provided yet!!
        channelT0->setT0(slotID, channel, t0Cal, err);
        nCal[slotID - 1]++;
      }

      file->Close();
      B2INFO("--> Input file closed");
    }
    IntervalOfValidity iov(0, 0, -1, -1); // all experiments and runs
    channelT0.import(iov);

    short nCalTot = 0;
    B2INFO("Summary: ");
    for (int iSlot = 0; iSlot < 16; iSlot++) {
      B2INFO("--> number of calibrated channels on Slot " << iSlot << " : " << nCal[iSlot] << "/512");
      nCalTot += nCal[iSlot];
    }


    B2RESULT("Channel T0 calibration constants imported to database, calibrated channels: "
             << nCalTot << "/ 8192");
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


  void TOPDatabaseImporter::generateFakeChannelMask(double fractionDead, double fractionHot)
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
    IntervalOfValidity iov(0, 0, -1, -1); // all experiments and runs
    channelMask.import(iov);

    B2WARNING("Generated and imported a fake channel mask to database for testing: "
              << ncall << "/" << nall);
    return;
  }


  void TOPDatabaseImporter::importPmtQEData(string fileName, string treeName = "qePmtData")
  {

    // declare db objects to be imported
    DBImportArray<TOPPmtQE> pmtQEs;

    static const int nChann = 16;
    std::string serialNum;
    std::vector<float> QE_data[nChann];
    float lambdaFirst, lambdaStep, collEff;

    // open root file and get tree
    TFile* file = new TFile(fileName.c_str(), "r");
    TTree* tQeData = (TTree*)file->Get(treeName.c_str());

    tQeData->SetBranchAddress("serialNum", &serialNum);
    tQeData->SetBranchAddress("lambdaFirst", &lambdaFirst);
    tQeData->SetBranchAddress("lambdaStep", &lambdaStep);
    tQeData->SetBranchAddress("collEff", &collEff);
    for (int ic = 0; ic < nChann; ic++) {
      TString cString = "QE_ch";
      cString += ic + 1;
      tQeData->SetBranchAddress(cString, &QE_data[ic]);
    }

    // loop on input tree entries and construct the pmtQE objects
    int countPMTs = 0;

    for (int ient = 0; ient < tQeData->GetEntries(); ient++) {
      tQeData->GetEntry(ient);
      auto* pmtQE = pmtQEs.appendNew(serialNum, lambdaFirst, lambdaStep, collEff);

      for (int ic = 0; ic < nChann; ic++) {
        pmtQE->setQE(ic + 1, QE_data[ic]);
      }
      countPMTs++;
    }

    IntervalOfValidity iov(0, 0, -1, -1); // all experiments and runs
    pmtQEs.import(iov);

    B2RESULT("QE data imported to database for" << countPMTs << " PMT's.");

    return;
  }


  void TOPDatabaseImporter::importPmtGainData(string fileName, string treeName = "gainPmtData")
  {

    // declare db objects to be imported
    DBImportArray<TOPPmtGainPar> pmtGains;

    static const int nChann = 16;
    std::string serialNum;
    float gain_const[nChann], gain_slope[nChann], gain_ratio[nChann];
    float hv_op;

    // open root file and get tree
    TFile* file = new TFile(fileName.c_str(), "r");
    TTree* tGainData = (TTree*)file->Get(treeName.c_str());

    tGainData->SetBranchAddress("serialNum", &serialNum);
    tGainData->SetBranchAddress("gain_const", &gain_const);
    tGainData->SetBranchAddress("gain_slope", &gain_slope);
    tGainData->SetBranchAddress("gain_ratio", &gain_ratio);
    tGainData->SetBranchAddress("hv_op", &hv_op);


    // loop on input tree entries and construct the pmtGain objects
    int countPMTs = 0;

    for (int ient = 0; ient < tGainData->GetEntries(); ient++) {
      tGainData->GetEntry(ient);
      auto* pmtGain = pmtGains.appendNew(serialNum);

      for (int ic = 0; ic < nChann; ic++) {
        pmtGain->setChannelData(ic + 1, gain_const[ic], gain_slope[ic], gain_ratio[ic]);
        pmtGain->setNominalHV(hv_op);
      }
      countPMTs++;
    }

    IntervalOfValidity iov(0, 0, -1, -1); // all experiments and runs
    pmtGains.import(iov);

    B2RESULT("PMT gain data imported to database for" << countPMTs << " PMT's.");

    return;
  }


  void TOPDatabaseImporter::importPmtInstallationData(string fileName, string treeName = "installationPmtData")
  {

    // declare db objects to be imported
    DBImportArray<TOPPmtInstallation> pmtInst;

    std::string serialNum;
    int moduleCNum, arrayNum, PMTposition;

    // open root file and get tree
    TFile* file = new TFile(fileName.c_str(), "r");
    TTree* tInstData = (TTree*)file->Get(treeName.c_str());

    tInstData->SetBranchAddress("serialNum", &serialNum);
    tInstData->SetBranchAddress("moduleCNum", &moduleCNum);
    tInstData->SetBranchAddress("arrayNum", &arrayNum);
    tInstData->SetBranchAddress("PMTposition", &PMTposition);

    // loop on input tree entries and construct the pmtGain objects
    int countPMTs = 0;

    for (int ient = 0; ient < tInstData->GetEntries(); ient++) {
      tInstData->GetEntry(ient);
      pmtInst.appendNew(serialNum, moduleCNum, arrayNum, PMTposition);
      countPMTs++;
    }

    IntervalOfValidity iov(0, 0, -1, -1); // all experiments and runs
    pmtInst.import(iov);

    B2RESULT("PMT installation data imported to database for" << countPMTs << " PMT's.");

    return;
  }


  void TOPDatabaseImporter::importPmtObsoleteData(string fileName, string treeName = "obsPmtData")
  {

    // declare db objects to be imported
    DBImportArray<TOPPmtObsoleteData> pmtObsData;

    std::string serialNum;
    std::string cathode;
    float hv_spec, dark_spec, qe380_spec;
    TOPPmtObsoleteData::EType type;

    // open root file and get tree
    TFile* file = new TFile(fileName.c_str(), "r");
    TTree* tObsData = (TTree*)file->Get(treeName.c_str());

    tObsData->SetBranchAddress("serialNum", &serialNum);
    tObsData->SetBranchAddress("cathode", &cathode);
    tObsData->SetBranchAddress("hv_spec", &hv_spec);
    tObsData->SetBranchAddress("dark_spec", &dark_spec);
    tObsData->SetBranchAddress("qe380_spec", &qe380_spec);

    // loop on input tree entries and construct the pmtGain objects
    int countPMTs = 0;

    for (int ient = 0; ient < tObsData->GetEntries(); ient++) {
      tObsData->GetEntry(ient);

      // set type to unknown for now
      type = TOPPmtObsoleteData::c_Unknown;

      pmtObsData.appendNew(serialNum, type, cathode, hv_spec, dark_spec, qe380_spec);
      countPMTs++;
    }

    IntervalOfValidity iov(0, 0, -1, -1); // all experiments and runs
    pmtObsData.import(iov);

    B2RESULT("PMT obsolete data imported to database for" << countPMTs << " PMT's.");

    return;
  }


  void TOPDatabaseImporter::importPmtTTSPar(string fileName, string treeName = "ttsPmtPar")
  {

    // declare db objects to be imported
    DBImportArray<TOPPmtTTSPar> pmtTtsPars;

    static const int nChann = 16;
    std::string serialNum;
    std::vector<float> gausFrac[nChann];
    std::vector<float> gausMean[nChann];
    std::vector<float> gausSigma[nChann];


    // open root file and get tree
    TFile* file = new TFile(fileName.c_str(), "r");
    TTree* tTtsPar = (TTree*)file->Get(treeName.c_str());

    tTtsPar->SetBranchAddress("serialNum", &serialNum);
    for (int ic = 0; ic < nChann; ic++) {
      TString cStringF = "gausF_ch";
      TString cStringM = "gausM_ch";
      TString cStringS = "gausS_ch";

      cStringF += ic + 1;
      cStringM += ic + 1;
      cStringS += ic + 1;

      tTtsPar->SetBranchAddress(cStringF, &gausFrac[ic]);
      tTtsPar->SetBranchAddress(cStringM, &gausMean[ic]);
      tTtsPar->SetBranchAddress(cStringS, &gausSigma[ic]);
    }

    // loop on input tree entries and construct the pmtQE objects
    int countPMTs = 0;

    for (int ient = 0; ient < tTtsPar->GetEntries(); ient++) {
      tTtsPar->GetEntry(ient);
      auto* pmtTtsPar = pmtTtsPars.appendNew(serialNum);

      for (int ic = 0; ic < nChann; ic++) {

        // check that the vectors have the same size. Otherwise skip the channel
        if ((gausFrac[ic].size() != gausMean[ic].size()) ||
            (gausFrac[ic].size() != gausSigma[ic].size())) {

          B2ERROR("The TTSPar vectors for PMT " << serialNum << ", channel " << ic + 1 << " have different sizes! Skipping channel...");
          continue;
        }

        for (uint iv = 0; iv < gausFrac[ic].size(); iv++) {
          pmtTtsPar->appendGaussian(ic + 1,
                                    gausFrac[ic].at(iv),
                                    gausMean[ic].at(iv),
                                    gausSigma[ic].at(iv));
        }
      }
      countPMTs++;
    }

    IntervalOfValidity iov(0, 0, -1, -1); // all experiments and runs
    pmtTtsPars.import(iov);

    B2RESULT("TTS parameters imported to database for" << countPMTs << " PMT's.");

    return;
  }


  void TOPDatabaseImporter::importPmtTTSHisto(string fileName, string treeName = "ttsPmtHisto")
  {

    // declare db objects to be imported
    DBImportArray<TOPPmtTTSHisto> pmtTtsHistos;

    static const int nChann = 16;
    std::string serialNum;
    std::vector<float> hv[nChann];
    std::vector<TH1F*> histo[nChann];

    // open root file and get tree
    TFile* file = new TFile(fileName.c_str(), "r");
    TTree* tTtsHisto = (TTree*)file->Get(treeName.c_str());

    tTtsHisto->SetBranchAddress("serialNum", &serialNum);
    for (int ic = 0; ic < nChann; ic++) {
      TString cStringHV = "hv_ch";
      TString cStringHist = "histo_ch";

      cStringHV += ic + 1;
      cStringHist += ic + 1;

      tTtsHisto->SetBranchAddress(cStringHV, &hv[ic]);
      tTtsHisto->SetBranchAddress(cStringHist, &histo[ic]);
    }

    // loop on input tree entries and construct the pmtQE objects
    int countPMTs = 0;

    for (int ient = 0; ient < tTtsHisto->GetEntries(); ient++) {
      tTtsHisto->GetEntry(ient);
      auto* pmtTtsHisto = pmtTtsHistos.appendNew(serialNum);

      for (int ic = 0; ic < nChann; ic++) {

        // check that the vectors have the same size. Otherwise skip the channel
        if (hv[ic].size() != histo[ic].size()) {
          B2ERROR("Mismatch between HV values and TTS histograms for PMT " << serialNum << ", channel " << ic + 1 << ". Skipping channel...");
          continue;
        }

        for (uint iv = 0; iv < hv[ic].size(); iv++) {
          pmtTtsHisto->appendHistogram(ic + 1, hv[ic].at(iv), histo[ic].at(iv));
        }
      }
      countPMTs++;
    }

    IntervalOfValidity iov(0, 0, -1, -1); // all experiments and runs
    pmtTtsHistos.import(iov);

    B2RESULT("TTS histograms imported to database for" << countPMTs << " PMT's.");

    return;
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
      pmtGain->setChannelData(channel, -13.77, 0.0042, 0.4);
    }

    pmtGain = pmtGains.appendNew("JT02135");
    pmtGain->setNominalHV(3450);
    for (unsigned channel = 1; channel <= 16; channel++) {
      pmtGain->setChannelData(channel, -12.77, 0.0045, 0.4);
    }

    for (const auto& gain : pmtGains) gain.print();

    //  IntervalOfValidity iov(0, 0, -1, -1); // all experiments and runs
    //  pmtGains.import(iov);


  }


} // end Belle2 namespace

