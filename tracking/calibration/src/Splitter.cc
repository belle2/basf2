/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include <utility>
#include <cmath>
#include <numeric>
#include <TGraphErrors.h>
#include <TStyle.h>
#include <TAxis.h>


//If compiled within BASF2
#ifdef _PACKAGE_
#include <tracking/calibration/Splitter.h>
#else
#include <Splitter.h>
#endif


namespace Belle2 {


  //return filtered runs intervals (remove very short runs)
  std::map<ExpRun, std::pair<double, double>> filter(const std::map<ExpRun, std::pair<double, double>>& runs, double cut,
                                                     std::map<ExpRun, std::pair<double, double>>& runsRemoved)
  {
    std::map<ExpRun, std::pair<double, double>> runsCopy;

    for (auto r : runs) {
      auto& I = r.second;
      double d =  I.second - I.first;
      if (d > cut)
        runsCopy[r.first] = r.second;
      else
        runsRemoved[r.first] = r.second;
    }

    return runsCopy;
  }


  /// plot runs on time axis
  void plotRuns(std::vector<std::pair<double, double>>  runs)
  {
    TGraphErrors* gr = new TGraphErrors();


    for (auto r : runs) {
      double m = (r.first + r.second) / 2;
      double e = (r.second - r.first) / 2;

      gr->SetPoint(gr->GetN(), m, 1);
      gr->SetPointError(gr->GetN() - 1, e, 0);
    }

    gStyle->SetEndErrorSize(6);

    gr->SetLineWidth(1);
    gr->SetMarkerSize(40);
    gr->Draw("ape");
    gr->GetYaxis()->SetRangeUser(-10, 10);
    gr->GetXaxis()->SetRangeUser(0, 256);

    gr->GetXaxis()->SetTitle("time [hours]");

  }

  /// get the range of interval with nIntervals and breaks stored in a vector
  std::pair<int, int> getStartEndIndexes(int nIntervals,  std::vector<int> breaks, int indx)
  {
    B2ASSERT("There must be at least one interval", nIntervals >= 1);
    B2ASSERT("Interval index must be positive", indx >= 0);
    B2ASSERT("Interval index must be smaller than #breaks", indx < int(breaks.size()) + 1); //There is one more interval than #breaks
    int s = (indx == 0) ? 0 : breaks[indx - 1] + 1;
    int e = (indx == int(breaks.size())) ? nIntervals - 1 : breaks[indx];
    return {s, e};
  }

  /// plot clusters or runs on time axis
  void plotSRuns(std::vector<std::pair<double, double>>  runs, std::vector<int> breaks, int offset = 2)
  {
    TGraphErrors* gr = new TGraphErrors();

    for (int i = 0; i < int(breaks.size()) + 1; ++i) {
      int s, e;
      std::tie(s, e) = getStartEndIndexes(runs.size(), breaks, i);
      double a = runs[s].first;
      double b = runs[e].second;

      double m = (a + b) / 2;
      double err = (b - a) / 2;

      gr->SetPoint(gr->GetN(), m, offset);
      gr->SetPointError(gr->GetN() - 1, err, 0);

    }

    gr->SetLineColor(kRed);
    gr->SetMarkerColor(kRed);
    gr->Draw("pe same");
  }


  /// print sorted lenghts of the runs
  void printBySize(std::vector<std::pair<double, double>>  runs)
  {
    std::vector<double> dist;
    for (auto r : runs) {
      double d = r.second - r.first;
      dist.push_back(d);
    }

    sort(dist.begin(), dist.end());

    for (auto d : dist)
      B2INFO(d);

  }

  /// the lossFunction formula (it can be modified according to the user's taste)
  double Splitter::lossFunction(const std::vector<Atom>&  vec, int s, int e) const
  {

    //raw time
    double rawTime = vec[e].t2 - vec[s].t1;

    //max gap
    double maxGap = 0;
    for (int i = s; i <= e - 1; ++i) {
      double d = vec[i + 1].t1 - vec[i].t2;
      maxGap = std::max(maxGap, d);
    }

    //net time
    double netTime = 0;
    for (int i = s; i <= e; ++i) {
      netTime += vec[i].t2 - vec[i].t1;
    }

    // Number of events
    double nEv = 0;
    for (int i = s; i <= e; ++i) {
      nEv += vec[i].nEv;
    }
    if (nEv == 0) nEv = 0.1;

    //double loss = pow(rawTime - tBest, 2) + gapPenalty * pow(maxGap, 2);
    //double loss = 1./nEv  + timePenalty * pow(rawTime, 2);

    lossFun->SetParameters(rawTime, netTime, maxGap, nEv);
    double lossNew = lossFun->Eval(0);

    return lossNew;
  }



  // split to many small intervals (atoms)
  std::vector<std::pair<double, double>> Splitter::splitToSmall(std::map<ExpRun, std::pair<double, double>> runs, double intSize)
  {
    // split into small intervals
    std::vector<std::pair<double, double>> smallRuns;

    for (auto r : runs) {
      auto& I = r.second;
      if (intSize < 0) {
        smallRuns.push_back(I);
        continue;
      }

      double runTime = I.second - I.first;
      int nSplits = runTime / intSize; //1-m intervals
      nSplits = std::max(1, nSplits); //at least 1 interval

      for (int i = 0; i < nSplits; ++i) {
        double L = I.first + i * (runTime / nSplits);
        double H = I.first + (i + 1) * (runTime / nSplits);
        smallRuns.push_back({L, H});
      }
    }
    return smallRuns;
  }






  // Get the optimal clustering of the atoms with indeces 0 .. e (recursive function with cache)
  double Splitter::getMinLoss(const std::vector<Atom>&  vec, int e, std::vector<int>& breaks)
  {
    // If entry in cache (speed up)
    if (cache[e].first >= 0) {
      breaks = cache[e].second;
      return cache[e].first;
    }


    std::vector<int> breaksOpt;
    double minVal = 1e30;
    int iMin = -10;
    for (int i = -1; i <= e - 1; ++i) {
      auto breaksNow = breaks;
      double r1 = 0;
      if (i != -1)
        r1 = getMinLoss(vec, i, breaksNow);
      double r2 = lossFunction(vec, i + 1, e);
      double tot = r1 + r2;

      if (tot < minVal) { //store minimum
        minVal = tot;
        iMin = i;
        breaksOpt = breaksNow;
      }
    }

    if (iMin != -1)
      breaksOpt.push_back(iMin);


    breaks = breaksOpt;
    cache[e] = std::make_pair(minVal, breaks); //store solution to cache
    return minVal;
  }

  //Get the indexes of the break-points
  std::vector<int> Splitter::dynamicBreaks(const std::vector<Atom>& runs)
  {
    //reset cache
    cache.resize(runs.size());
    for (auto& c : cache)
      c = std::make_pair(-1.0, std::vector<int>({}));


    std::vector<int> breaks;
    getMinLoss(runs, runs.size() - 1, breaks); //the minLoss (output) currently not used, only breaks

    return breaks;
  }



  /** Get exp number + run number from time
   * @param runs: map, where key contain the exp-run number and value the start- and end-time of the run
   * @param t: time of interest [hours]
   * @return:  the exp-run number at the input time t
   **/
  static ExpRun getRun(std::map<ExpRun, std::pair<double, double>> runs, double t)
  {
    ExpRun rFound(-1, -1);
    int nFound = 0;
    for (auto r : runs) { //Linear search over runs
      if (r.second.first <= t && t < r.second.second) {
        rFound = r.first;
        ++nFound;
      }
    }

    B2ASSERT("Exactly one interval should be found", nFound == 1);
    B2ASSERT("Assert that something was found", rFound != ExpRun(-1, -1));
    return rFound;
  }



  std::vector<std::map<ExpRun, std::pair<double, double>>> breaks2intervalsSep(const std::map<ExpRun, std::pair<double, double>>&
      runsMap,
      const std::vector<Atom>& currVec, const std::vector<int>& breaks)
  {
    std::vector<std::map<ExpRun, std::pair<double, double>>> splitsNow(breaks.size() + 1);
    for (int i = 0; i < int(breaks.size()) + 1; ++i) {
      int s, e;
      std::tie(s, e) = getStartEndIndexes(currVec.size(), breaks, i);

      // loop over atoms in single calib interval
      for (int k = s; k <= e; ++k) {
        ExpRun r = getRun(runsMap, (currVec[k].t1 + currVec[k].t2) / 2.); //runexp of the atom
        if (splitsNow[i].count(r)) { //if already there
          splitsNow[i].at(r).first  = std::min(splitsNow[i].at(r).first, currVec[k].t1);
          splitsNow[i].at(r).second = std::max(splitsNow[i].at(r).second, currVec[k].t2);
        } else { //if new
          splitsNow[i][r].first  = currVec[k].t1;
          splitsNow[i][r].second = currVec[k].t2;
        }
      }
    }

    return splitsNow;
  }

  //Merge two intervals together
  std::map<ExpRun, std::pair<double, double>> Splitter::mergeIntervals(std::map<ExpRun, std::pair<double, double>> I1,
                                           std::map<ExpRun, std::pair<double, double>> I2)
  {
    std::map<ExpRun, std::pair<double, double>>  I = I1;
    for (auto r : I2) {
      ExpRun run = r.first;
      if (I.count(run) == 0)
        I[run] = r.second;
      else {
        I.at(run) = std::make_pair(std::min(I1.at(run).first, I2.at(run).first),   std::max(I1.at(run).second, I2.at(run).second));
      }
    }
    return I;
  }


}
