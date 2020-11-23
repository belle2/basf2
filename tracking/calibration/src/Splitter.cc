/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Radek Zlebcik                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <utility>
#include <cmath>
#include <numeric>
#include <TGraphErrors.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TAxis.h>


//If compiled within BASF2
#ifdef _PACKAGE_
#include <tracking/calibration/Splitter.h>
#else
#include "Splitter.h"
#endif


using namespace std;

namespace Belle2 {


  /// Slice the vector to contain only elements with indeces s .. e (included)
  static vector<pair<double, double>> slice(vector<pair<double, double>> vec, int s, int e)
  {
    return vector<pair<double, double>>(vec.begin() + s, vec.begin() + e + 1);
  }



  //return filtered runs intervals (remove very short runs)
  map<ExpRun, pair<double, double>> filter(const map<ExpRun, pair<double, double>>& runs, double cut,
                                           map<ExpRun, pair<double, double>>& runsRemoved)
  {
    map<ExpRun, pair<double, double>> runsCopy;

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
  void plotRuns(vector<pair<double, double>>  runs)
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
  static pair<int, int> getStartEndIndexes(int nIntervals,  vector<int> breaks, int indx)
  {
    B2ASSERT("There must be at least one interval", nIntervals >= 1);
    B2ASSERT("Interval index must be positive", indx >= 0);
    B2ASSERT("Interval index must be smaller than #breaks", indx < int(breaks.size()) + 1); //There is one more interval than #breaks
    int s = (indx == 0) ? 0 : breaks[indx - 1] + 1;
    int e = (indx == int(breaks.size())) ? nIntervals - 1 : breaks[indx];
    return {s, e};
  }

  /// plot clusters or runs on time axis
  void plotSRuns(vector<pair<double, double>>  runs, vector<int> breaks, int offset = 2)
  {
    TGraphErrors* gr = new TGraphErrors();

    for (int i = 0; i < int(breaks.size()) + 1; ++i) {
      int s, e;
      tie(s, e) = getStartEndIndexes(runs.size(), breaks, i);
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
  void printBySize(vector<pair<double, double>>  runs)
  {
    vector<double> dist;
    for (auto r : runs) {
      double d = r.second - r.first;
      dist.push_back(d);
    }

    sort(dist.begin(), dist.end());

    for (auto d : dist)
      B2INFO(d);

  }

  /// the lossFunction formula (it can be modified according to the user's taste)
  double Splitter::lossFunction(const vector<pair<double, double>>&  vec, int s, int e) const
  {

    //raw time
    double rawTime = vec[e].second - vec[s].first;

    //max gap
    double gapMax = 0;
    for (int i = s; i <= e - 1; ++i) {
      double d = vec[i + 1].first - vec[i].second;
      gapMax = max(gapMax, d);
    }

    //net gap
    //double netTime = 0;
    //for (int i = s; i <= e; ++i) {
    //  netTime += vec[i].second - vec[i].first;
    //}


    double loss = pow(rawTime - tBest, 2) + gapPenalty * pow(gapMax, 2);

    return loss;
  }



  // split to many small intervals (atoms)
  vector<pair<double, double>> Splitter::splitToSmall(map<ExpRun, pair<double, double>> runs, double intSize)
  {
    // split into ~1m intervals
    vector<pair<double, double>> smallRuns;

    for (auto r : runs) {
      auto& I = r.second;
      double runTime = I.second - I.first;
      int nSplits = runTime / intSize; //1-m intervals
      nSplits = max(1, nSplits); //at least 1 interval

      for (int i = 0; i < nSplits; ++i) {
        double L = I.first + i * (runTime / nSplits);
        double H = I.first + (i + 1) * (runTime / nSplits);
        smallRuns.push_back({L, H});
      }
    }
    return smallRuns;
  }




  // Get the optimal clustering of the atoms with indeces 0 .. e (recursive function with cache)
  double Splitter::getMinLoss(const vector<pair<double, double>>&  vec, int e, vector<int>& breaks)
  {
    // If entry in cache (speed up)
    if (cache[e].first >= 0) {
      breaks = cache[e].second;
      return cache[e].first;
    }


    vector<int> breaksOpt;
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
    cache[e] = make_pair(minVal, breaks); //store solution to cache
    return minVal;
  }

  //Get the indexes of the break-points
  vector<int> Splitter::dynamicBreaks(vector<pair<double, double>>  runs)
  {
    //reset cache
    cache.resize(runs.size());
    for (auto& c : cache)
      c = make_pair(-1.0, vector<int>({}));


    vector<int> breaks;
    getMinLoss(runs, runs.size() - 1, breaks); //the minLoss (output) currently not used, only breaks

    return breaks;
  }


  /** Convert breaks to intervals, notice that the count of the output intervals is one more than number of the input breaks.
   *  I.e. this function merge non-divisible intervals defined in currVec into bigger clusters according to the integer indexes of the breaks.
   * @param currVec: vector with time limits of the atoms (small non-divisible time intervals)
   * @param breaks: integer indexes where to break the whole currVec. Notice that index 0 means break between first and second atom
   * @return: Vector with times of the intervals, each intervals is defined by it's start- and end-time.
   **/
  static vector<pair<double, double>> breaks2intervals(vector<pair<double, double>> currVec, vector<int> breaks)
  {

    vector<pair<double, double>> splitsNow;
    for (int i = 0; i < int(breaks.size()) + 1; ++i) {
      int s, e;
      tie(s, e) = getStartEndIndexes(currVec.size(), breaks, i);
      double sVal = currVec[s].first;
      double eVal = currVec[e].second;

      splitsNow.push_back({sVal, eVal});
    }

    return splitsNow;
  }


  /** Get exp number + run number from time
   * @param runs: map, where key contain the exp-run number and value the start- and end-time of the run
   * @param t: time of interest [hours]
   * @return:  the exp-run number at the input time t
   **/
  static ExpRun getRun(map<ExpRun, pair<double, double>> runs, double t)
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



  /** Get calibration intervals according to the indexes of the breaks
   * @param runsMap: map defining the time ranges of the runs
   * @param currVec: vector with time intervals of the atoms (small non-divisible time intervals)
   * @param breaks: vector with integer indexes of the breaks
   * @return: a vector of the calib. interwas where each interval is a map with exp-run as a key and start- end-time as a value
   **/
  static vector<map<ExpRun, pair<double, double>>> breaks2intervalsSep(const map<ExpRun, pair<double, double>>& runsMap,
      const vector<pair<double, double>>& currVec, const vector<int>& breaks)
  {
    vector<map<ExpRun, pair<double, double>>> splitsNow(breaks.size() + 1);
    for (int i = 0; i < int(breaks.size()) + 1; ++i) {
      int s, e;
      tie(s, e) = getStartEndIndexes(currVec.size(), breaks, i);

      // loop over atoms in single calib interval
      for (int k = s; k <= e; ++k) {
        ExpRun r = getRun(runsMap, (currVec[k].first + currVec[k].second) / 2.); //runexp of the atom
        if (splitsNow[i].count(r)) { //if already there
          splitsNow[i].at(r).first  = min(splitsNow[i].at(r).first, currVec[k].first);
          splitsNow[i].at(r).second = max(splitsNow[i].at(r).second, currVec[k].second);
        } else { //if new
          splitsNow[i][r].first  = currVec[k].first;
          splitsNow[i][r].second = currVec[k].second;
        }
      }
    }

    return splitsNow;
  }

  //Merge two intervals together
  map<ExpRun, pair<double, double>> Splitter::mergeIntervals(map<ExpRun, pair<double, double>> I1,
                                                             map<ExpRun, pair<double, double>> I2)
  {
    map<ExpRun, pair<double, double>>  I = I1;
    for (auto r : I2) {
      ExpRun run = r.first;
      if (I.count(run) == 0)
        I[run] = r.second;
      else {
        I.at(run) = make_pair(min(I1.at(run).first, I2.at(run).first),   max(I1.at(run).second, I2.at(run).second));
      }
    }
    return I;
  }

  //Get the optimal time intervals with two levels of segmentation
  vector<vector<map<ExpRun, pair<double, double>>>>  Splitter::getIntervals(const map<ExpRun, pair<double, double>>& runs,
      double tBestSize,
      double tBestVtx, double GapPenalty)
  {
    // Divide into small intervals
    auto smallRuns = splitToSmall(runs, 0.1);

    gapPenalty = GapPenalty;

    // Calculate breaks for SizeIntervals
    tBest = tBestSize;
    vector<int> breaksSize = dynamicBreaks(smallRuns);


    vector<vector<pair<double, double>>> splits; //split intervals
    vector<vector<map<ExpRun, pair<double, double>>>> splitsRun; //split intervals with runNumber info


    // Calculate breaks for VtxIntervals
    tBest = tBestVtx;
    vector<int> breaksVtx;
    for (int i = 0; i < int(breaksSize.size()) + 1; ++i) { //loop over all size intervals
      int s, e;
      tie(s, e) = getStartEndIndexes(smallRuns.size(), breaksSize, i);

      // Store only atoms in the current BS-size calib. intervals
      auto currVec = slice(smallRuns, s, e);

      // Get optimal breaks in particular BS-size calib. interval
      vector<int> breaksSmall = dynamicBreaks(currVec);

      // Transform breaks to the time intervals
      vector<pair<double, double>> splitsNow = breaks2intervals(currVec, breaksSmall);

      // Incorporate the runNumber information
      auto splitSeg = breaks2intervalsSep(runs, currVec, breaksSmall);

      // Put the vector of VtxInterval (representing division of BS-size interval)
      // into the vector
      splitsRun.push_back(splitSeg);
      splits.push_back(splitsNow);

      if (s != 0) breaksVtx.push_back(s - 1);
      for (auto b : breaksSmall)
        breaksVtx.push_back(b + s);
      if (e != int(breaksSize.size())) breaksVtx.push_back(e);
    }

    return splitsRun;
  }

}
