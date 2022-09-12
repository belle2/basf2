/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#include <mdst/dbobjects/CollisionInvariantMass.h>
#include <tracking/calibration/InvariantMassAlgorithm.h>
#include <tracking/calibration/InvariantMassMuMuStandAlone.h>
#include <tracking/calibration/InvariantMassBhadStandAlone.h>
#include <tracking/calibration/calibTools.h>
#include <TFile.h>

#include <Eigen/Dense>
#include <iostream>
#include <fstream>
#include <iomanip>

using Eigen::VectorXd;
using Eigen::MatrixXd;

using namespace Belle2;


//Using boostVector collector for the input
InvariantMassAlgorithm::InvariantMassAlgorithm() : CalibrationAlgorithm("BoostVectorCollector")
{
  setDescription("Collision invariant mass calibration algorithm");
}


/** Create InvarinatMass object */
static TObject* getInvariantMassObj(VectorXd vMass, MatrixXd  vMassUnc, MatrixXd vMassSpread)
{
  auto payload = new CollisionInvariantMass();

  double mass    = vMass(0);
  double unc     = vMassUnc(0, 0);
  double spread  = vMassSpread(0, 0);

  payload->setMass(mass, unc, spread);
  TObject* obj  = static_cast<TObject*>(payload);
  return obj;
}


/** Run the InvariantMass analysis using mumu method on the provided events
  @param evts: vector of the mumu events
  @param calibAnalysis: function calculating the calibration parameters on the single calibration interval
  @param lossFunctionOuter: outer loss function defining the calibration intervals
  @param lossFunctionInner: inner loss function defining the calibration intervals
  @return A vector containing calibration data
*/
template<typename Fun>
std::vector<CalibrationData>  runMuMuCalibration(const std::vector<Belle2::InvariantMassMuMuCalib::Event>& evts,  Fun calibAnalysis,
                                                 TString lossFunctionOuter, TString lossFunctionInner)
{
  //Time range for each ExpRun
  std::map<ExpRun, std::pair<double, double>> runsInfoOrg = getRunInfo(evts);
  std::map<ExpRun, std::pair<double, double>> runsRemoved; //map with time intervals of very short runs
  auto runsInfo = filter(runsInfoOrg, 2. / 60, runsRemoved); //include only runs longer than 2mins

  // If nothing remains
  if (runsInfo.size() == 0) {
    B2WARNING("Too short run");
    return  {};
  }

  // Get intervals based on the input loss functions
  Splitter splt;
  auto splits = splt.getIntervals(runsInfo, evts, lossFunctionOuter, lossFunctionInner, 100 /*maximal length of the run */);

  //Loop over all calibration intervals
  std::vector<CalibrationData> calVec;
  for (auto s : splits) {
    CalibrationData calD = runAlgorithm(evts, s, calibAnalysis); // run the calibration over the interval s
    calVec.push_back(calD);
  }

  // extrapolate results to the low-stat intervals
  extrapolateCalibration(calVec);

  // Include removed short runs
  for (auto shortRun : runsRemoved) {
    addShortRun(calVec,  shortRun);
  }

  return calVec;
}


std::vector<CalibrationData> addSpreadAndOffset(const std::vector<CalibrationData>& mumuCalResults, double spread, double spreadUnc,
                                                double offset, double offsetUnc)
{
  std::vector<CalibrationData> mumuCalResultsNew = mumuCalResults;

  for (auto& calibNew :  mumuCalResultsNew) {

    for (unsigned j = 0; j < calibNew.subIntervals.size(); ++j) {
      calibNew.pars.cnt[j](0)      += offset;
      calibNew.pars.spreadMat(0, 0) = spread;
      calibNew.pars.spreadUnc       = spreadUnc;

      calibNew.pars.shift           = offset;
      calibNew.pars.shiftUnc        = offsetUnc;
      calibNew.pars.pulls[j]        = 0;
    }
  }
  return mumuCalResultsNew;
}




static std::vector<TTree*> getTrees(TString tag, TFile* f)
{
  if (f == nullptr)
    return {};

  f->cd(tag + "/events");
  TList* l = f->CurrentDirectory().load()->GetListOfKeys();


  std::vector<TTree*> treeVec;

  for (const auto&& obj : *l) {
    TString n = obj->GetName();

    TString trDir = tag + "/events/" + TString(obj->GetName()) + "/events_1";

    treeVec.push_back((TTree*) f->Get(trDir));
  }

  return treeVec;
}


static std::vector<InvariantMassMuMuCalib::Event> readMuMuFiles(std::vector<std::string> files, bool is4S)
{
  std::vector<InvariantMassMuMuCalib::Event> events;
  for (auto fName : files) {
    TFile* f = TFile::Open(fName.c_str(), "READ");
    auto trees = getTrees("BoostVectorCollector", f);

    for (auto tr : trees) {
      if (!tr) continue;
      std::vector<InvariantMassMuMuCalib::Event> eventsNow = InvariantMassMuMuCalib::getEvents(tr, is4S);
      events.insert(events.end(), eventsNow.begin(), eventsNow.end());
    }

    f->Close();
  }

  sort(events.begin(), events.end(), [](InvariantMassMuMuCalib::Event e1, InvariantMassMuMuCalib::Event e2) {return e1.t < e2.t;});
  return events;
}


static std::vector<InvariantMassBhadCalib::Event> readBhadFiles(std::vector<std::string> files)
{
  std::vector<InvariantMassBhadCalib::Event> events;
  for (auto fName : files) {
    TFile* f = TFile::Open(fName.c_str(), "READ");
    auto trees = getTrees("EcmsCollector", f);

    for (auto tr : trees) {
      if (!tr) continue;
      std::vector<InvariantMassBhadCalib::Event> eventsNow = InvariantMassBhadCalib::getEvents(tr);
      events.insert(events.end(), eventsNow.begin(), eventsNow.end());
    }

    f->Close();
  }

  sort(events.begin(), events.end(), [](InvariantMassBhadCalib::Event e1, InvariantMassBhadCalib::Event e2) {return e1.t < e2.t;});
  return events;
}







std::vector<InvariantMassMuMuCalib::Event> InvariantMassAlgorithm::getDataMuMu(const std::vector<std::string>& files, bool is4S)
{
  if (files.size() == 0)
    return {};

  clearCalibrationData();
  setPrefix("BoostVectorCollector");

  setInputFileNames(files);
  fillRunToInputFilesMap();

  TTree* eventsTr = getObjectPtr<TTree>("events").get();

  // Check that there are at least some mumu data
  if (!eventsTr || eventsTr->GetEntries() < 15) {
    if (eventsTr) {
      if (is4S)
        B2WARNING("Small number of events in the 4S mumu sample, only " << eventsTr->GetEntries());
      else
        B2WARNING("Small number of events in the off-resonance mumu sample, only " << eventsTr->GetEntries());
    } else
      B2WARNING("Pointer to the \"events\" object not defined for the mumu sample");

    return {};
  }
  B2INFO("Number of mumu events: " << eventsTr->GetEntries());


  return InvariantMassMuMuCalib::getEvents(eventsTr, is4S);

}

std::vector<InvariantMassBhadCalib::Event> InvariantMassAlgorithm::getDataHadB(const std::vector<std::string>& files)
{
  if (files.size() == 0)
    return {};

  clearCalibrationData();
  setPrefix("EcmsCollector");

  setInputFileNames(files);
  fillRunToInputFilesMap();
  TTree* eventsTr = getObjectPtr<TTree>("events").get();

  // Check that there are at least some mumu data
  if (!eventsTr || eventsTr->GetEntries() < 15) {
    if (eventsTr)
      B2WARNING("Too few data, only " << eventsTr->GetEntries() << " events found in hadronic B decay sample");
    else
      B2WARNING("Pointer to the \"events\" object not defined for hadronic B decay sample");

    return {};
  }
  B2INFO("Number of mumu events: " << eventsTr->GetEntries());


  return InvariantMassBhadCalib::getEvents(eventsTr);

}


/** Get the err-weighted average of two variables */
double weightAvg(double x, double xe, double y, double ye)
{
  double xe2 = xe * xe;
  double ye2 = ye * ye;
  double res = (ye2 * x + xe2 * y) / (xe2 + ye2);
  return res;
}


/** print payloads to the file */
void printToFile(const std::vector<CalibrationData>&  CalResults, TString outFileName)
{
  //Store info to the text file
  std::ofstream finalOut(outFileName);
  finalOut <<
           "t1   t2      exp1   run1     exp2   run2     state     Ecms   EcmsUnc     pull   shift   shiftUnc     spread   spreadUnc" <<
           std::endl;

  for (auto cal : CalResults) {

    for (unsigned j = 0; j < cal.subIntervals.size(); ++j) {
      double cnt     = cal.pars.cnt[j](0);
      double cntUnc  = cal.pars.cntUnc[j](0, 0);
      double spread  = cal.pars.spreadMat(0, 0);
      double spreadUnc = cal.pars.spreadUnc;

      double shift    = cal.pars.shift;
      double shiftUnc = cal.pars.shiftUnc;
      double pull     = cal.pars.pulls[j];

      double s = cal.subIntervals[j].begin()->second.first;
      double e = cal.subIntervals[j].rbegin()->second.second;

      double exp1 = cal.subIntervals[j].begin()->first.exp;
      double exp2 = cal.subIntervals[j].rbegin()->first.exp;
      double run1 = cal.subIntervals[j].begin()->first.run;
      double run2 = cal.subIntervals[j].rbegin()->first.run;


      finalOut <<  std::setprecision(8) <<  s << " " << e << " " << exp1 << " " << run1 << " " << exp2 << " " << run2  << " " << j << " "
               <<  1e3 * cnt << "   " << 1e3 * cntUnc << "   " <<   pull <<
               " " << 1e3 * shift << "    " << 1e3 *
               shiftUnc << "   " << 1e3 * spread << "   " << 1e3 * spreadUnc <<  std::endl;

    }

  }
  finalOut.close();
}



std::vector<std::vector<CalibrationData>> InvariantMassAlgorithm::adjustOffResonanceEnergy(std::vector<std::vector<CalibrationData>>
                                       CalResultsBlocks,
                                       const std::vector<std::vector<InvariantMassMuMuCalib::Event>>&  evtsMuMuBlocks)
{
  // Adjust the energy in the off-resoncance runs
  for (int b = 0; b < int(CalResultsBlocks.size()); ++b) {
    // only deal with off-res runs
    if (evtsMuMuBlocks[b][0].is4S) continue;

    std::vector<CalibPars> parsEdges;
    if (b > 0 && evtsMuMuBlocks[b - 1][0].is4S)
      parsEdges.push_back(CalResultsBlocks[b - 1].back().pars);
    if (b < int(CalResultsBlocks.size()) - 1 && evtsMuMuBlocks[b + 1][0].is4S)
      parsEdges.push_back(CalResultsBlocks[b + 1].front().pars);
    std::vector<double> shifts, shiftsUnc, spreads, spreadsUnc;
    for (auto p : parsEdges) {
      shifts.push_back(p.shift);
      shiftsUnc.push_back(p.shiftUnc);
      spreads.push_back(p.spreadMat(0, 0));
      spreadsUnc.push_back(p.spreadUnc);
    }

    // by default take the offset from the steering
    double spread    = m_eCMSmumuSpread;
    double spreadUnc = 0.2e-3; // default spread for off-resonance [GeV]
    double shift     = m_eCMSmumuShift;
    double shiftUnc  = 0.3e-3; // default shift unc [GeV]

    // in case of single neighboring 4S block
    if (shifts.size() == 1) {
      spread    = spreads[0];
      spreadUnc = spreadsUnc[0];
      shift     = shifts[0];
      shiftUnc  = shiftsUnc[0];
    }
    // if neighbors are from both sides take the average
    else if (shifts.size() == 2) {
      spread = weightAvg(spreads[0], spreadsUnc[0], spreads[1], spreadsUnc[1]);
      shift  = weightAvg(shifts[0], shiftsUnc[0], shifts[1], shiftsUnc[1]);

      spreadUnc = sqrt(pow(spreads[0] - spreads[1], 2) + (pow(spreadsUnc[0], 2)  + pow(spreadsUnc[1], 2)) / 2);
      shiftUnc  = sqrt((pow(shift - shifts[0], 2) + pow(shift - shifts[1], 2)) / 2 + (pow(shiftsUnc[0], 2)  + pow(shiftsUnc[1], 2)) / 2);
    }

    CalResultsBlocks[b] = addSpreadAndOffset(CalResultsBlocks[b], spread, spreadUnc, shift, shiftUnc);
  }
  return CalResultsBlocks;
}








/* Main calibration method calling dedicated functions */
CalibrationAlgorithm::EResult InvariantMassAlgorithm::calibrate()
{
  std::vector<std::string> files = getVecInputFileNames();

  std::vector<std::string> filesHad4S, filesMuMu4S, filesMuMuOff;
  for (auto f : files) {
    if (f.find("/dimuon_4S/") != std::string::npos)
      filesMuMu4S.push_back(f);
    else if (f.find("/dimuon_Off/") != std::string::npos)
      filesMuMuOff.push_back(f);
    else if (f.find("/hadB_4S/") != std::string::npos)
      filesHad4S.push_back(f);
    else {
      B2FATAL("Unrecognised data type");
    }
  }


  for (auto r : filesMuMu4S)
    B2INFO("MuMu4SFile name " << r);
  for (auto r : filesMuMuOff) {
    B2INFO("MuMuOffFile name " << r);
  }
  for (auto r : filesHad4S)
    B2INFO("Had4SFile name " << r);



  // load the mumu data
  const auto evtsMuMuOff = readMuMuFiles(filesMuMuOff, false /*is4S*/);
  const auto evtsMuMu4S  = readMuMuFiles(filesMuMu4S, true /*is4S*/);

  if (evtsMuMuOff.size() + evtsMuMu4S.size() < 50) {
    B2WARNING("Not enough data, there are only " << evtsMuMuOff.size() + evtsMuMu4S.size() << " mumu events");
    return CalibrationAlgorithm::EResult::c_NotEnoughData;
  }


  // load hadB data
  const auto evtsHad = readBhadFiles(filesHad4S);

  // merge mumu data
  std::vector<InvariantMassMuMuCalib::Event> evtsMuMuTemp = evtsMuMu4S;
  evtsMuMuTemp.insert(evtsMuMuTemp.end(), evtsMuMuOff.begin(), evtsMuMuOff.end());

  if (evtsMuMuTemp.size() < 50) {
    B2WARNING("Not enough mumu data, there are only " << evtsMuMuTemp.size() << " mumu events");
    return CalibrationAlgorithm::EResult::c_NotEnoughData;
  }


  // sort by time
  std::sort(evtsMuMuTemp.begin(), evtsMuMuTemp.end(), [](const InvariantMassMuMuCalib::Event & evt1,
  const InvariantMassMuMuCalib::Event & evt2) { return evt1.t < evt2.t; });

  //split the mumu events into the blocks with identical run type
  std::vector<std::vector<InvariantMassMuMuCalib::Event>> evtsMuMuBlocks;
  bool is4Sold = evtsMuMuTemp[0].is4S;
  evtsMuMuBlocks.push_back({}); // empty vec to start
  for (const auto& ev : evtsMuMuTemp) {
    if (is4Sold != ev.is4S) {
      evtsMuMuBlocks.push_back({}); // adding new entry
      is4Sold = ev.is4S;
    }
    evtsMuMuBlocks.back().push_back(ev);
  }
  B2INFO("Number of mumu 4S events " << evtsMuMu4S.size());
  B2INFO("Number of mumu off-res events " << evtsMuMuOff.size());
  B2INFO("Total number of mumu events " << evtsMuMuTemp.size());
  B2INFO("Number of hadronic B events " << evtsHad.size());

  B2INFO("Number of main calibration blocks " << evtsMuMuBlocks.size());

  std::ofstream BonlyOut("BonlyEcmsCalib.txt");
  BonlyOut << "t1    t2   exp1   run1      exp2   run2      Ecms   EcmsUnc    spread   spreadUnc" << std::endl;

  std::vector<std::vector<CalibrationData>> CalResultsBlocks;
  //calibrate each run-type block separately
  for (const auto& evtsMuMu : evtsMuMuBlocks) {

    // Run the mumuCalibration
    const std::vector<CalibrationData> mumuCalResults = runMuMuCalibration(evtsMuMu,
                                                        InvariantMassMuMuCalib::runMuMuInvariantMassAnalysis,
                                                        m_lossFunctionOuter, m_lossFunctionInner);


    //if off-res block or hadB is turned off
    if (!m_runHadB || evtsMuMu[0].is4S == false) {
      CalResultsBlocks.push_back(mumuCalResults);
      continue;
    }


    // If its 4S type and hadB allowed, run the combined Calib
    auto combCalResults = mumuCalResults;
    //Update the calibration with Bhad data
    for (unsigned i = 0; i < mumuCalResults.size(); ++i) {
      const auto& calibMuMu = mumuCalResults[i];
      auto& calibComb = combCalResults[i];

      std::vector<std::pair<double, double>> limits, mumuVals;

      for (unsigned j = 0; j < calibMuMu.subIntervals.size(); ++j) {
        double s = calibMuMu.subIntervals[j].begin()->second.first;
        double e = calibMuMu.subIntervals[j].rbegin()->second.second;
        limits.push_back({s, e});

        double val = calibMuMu.pars.cnt[j](0);
        double unc = calibMuMu.pars.cntUnc[j](0, 0);
        mumuVals.push_back({val, unc});
      }


      //////////////////////////////////////
      // run B mesons stand-alone calibration
      //////////////////////////////////////
      auto resB = InvariantMassBhadCalib::doBhadOnlyFit(evtsHad, limits);

      // store it to file
      double s = calibMuMu.subIntervals.front().begin()->second.first; // start time in hours
      double e = calibMuMu.subIntervals.back().rbegin()->second.second;// end time in hours

      double exp1 = calibMuMu.subIntervals.front().begin()->first.exp;
      double exp2 = calibMuMu.subIntervals.back().rbegin()->first.exp;
      double run1 = calibMuMu.subIntervals.front().begin()->first.run;
      double run2 = calibMuMu.subIntervals.back().rbegin()->first.run;

      BonlyOut <<  std::setprecision(8) <<  s << " " << e << " " << exp1 << " " << run1 << " " << exp2 << " " << run2  << " " <<
               1e3 * resB[0] << "  " << 1e3 * resB[1] << " " <<  1e3 * resB[2] << "  " << 1e3 * resB[3] <<  std::endl;



      //////////////////////////////////////
      //run the combined calibration
      //////////////////////////////////////
      auto res = InvariantMassBhadCalib::doBhadFit(evtsHad, limits, mumuVals, resB);

      // update calibration data
      for (unsigned j = 0; j < calibMuMu.subIntervals.size(); ++j) {
        calibComb.pars.cnt[j](0)       = res[j][0];
        calibComb.pars.cntUnc[j](0, 0) = res[j][1];
        calibComb.pars.spreadMat(0, 0) = res[j][2];

        calibComb.pars.spreadUnc = res[j][3];
        calibComb.pars.shift     = res[j][4];
        calibComb.pars.shiftUnc  = res[j][5];
        calibComb.pars.pulls[j]  = res[j][6];
      }


    }

    CalResultsBlocks.push_back(combCalResults);

  }

  assert(CalResultsBlocks.size() == evtsMuMuBlocks.size());

  CalResultsBlocks = adjustOffResonanceEnergy(CalResultsBlocks, evtsMuMuBlocks);


  std::vector<CalibrationData>  CalResults;
  for (auto cb : CalResultsBlocks)
    for (auto c : cb)
      CalResults.push_back(c);

  //Store info to the text file
  printToFile(CalResults, "finalEcmsCalib.txt");


  storePayloadsNoIntraRun(CalResults, "CollisionInvariantMass", getInvariantMassObj);


  return CalibrationAlgorithm::EResult::c_OK;
}
