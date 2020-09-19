#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <utility>
#include <cmath>
#include <numeric>
#include "TGraphErrors.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TAxis.h"

#include "Splitter.h"

using namespace std;

//hack for ROOT macros
#undef assert
#define assert(arg)  { if((arg) == false) {cout << __FILE__ <<", line "<<__LINE__ << endl << "" << #arg << " failed" << endl;   exit(0);} }





vector<pair<double, double>> Slice(vector<pair<double, double>> vec, int s, int e)
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


// plot runs on time axis
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

// plot clusters or runs on time axis
void plotSRuns(vector<pair<double, double>>  runs, vector<int> breaks, int offset = 2)
{
  TGraphErrors* gr = new TGraphErrors();

  for (int i = 0; i < int(breaks.size()) + 1; ++i) {
    int s = (i == 0) ? 0 : breaks[i - 1] + 1;
    int e = (i == int(breaks.size())) ? runs.size() - 1 : breaks[i];
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



void printBySize(vector<pair<double, double>>  runs)
{
  vector<double> dist;
  for (auto r : runs) {
    double d = r.second - r.first;
    dist.push_back(d);
  }

  sort(dist.begin(), dist.end());

  for (auto d : dist)
    cout << d << endl;

}


double Splitter::lossFunction(const vector<pair<double, double>>&  vec, int s, int e)
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
  double netTime = 0;
  for (int i = s; i <= e; ++i) {
    netTime += vec[i].second - vec[i].first;
  }


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





double Splitter::getMinLoss(const vector<pair<double, double>>&  vec,   int b, vector<int>& breaks)
{
  // If intry in cache (speed up)
  if (cache[b].first >= 0) {
    breaks = cache[b].second;
    return cache[b].first;
  }


  vector<int> breaksOpt;
  double minVal = 1e30;
  int iMin = -10;
  for (int i = -1; i <= b - 1; ++i) {
    auto breaksNow = breaks;
    double r1 = 0;
    if (i != -1)
      r1 = getMinLoss(vec, i, breaksNow);
    double r2 = lossFunction(vec, i + 1, b);
    double tot = r1 + r2;

    if (tot < minVal) {
      minVal = tot;
      iMin = i;
      breaksOpt = breaksNow;
    }
  }

  if (iMin != -1)
    breaksOpt.push_back(iMin);


  breaks = breaksOpt;
  cache[b] = make_pair(minVal, breaks);
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
  double res = getMinLoss(runs, runs.size() - 1, breaks);

  return breaks;
}


// Convert breaks to intervals, there is one more interval than #breakPoints
vector<pair<double, double>> breaks2intervals(vector<pair<double, double>> currVec, vector<int> breaks)
{

  vector<pair<double, double>> splitsNow;
  for (int i = 0; i < int(breaks.size()) + 1; ++i) {
    int s = (i == 0) ? 0 : breaks[i - 1] + 1;
    int e = (i == int(breaks.size())) ? currVec.size() - 1 : breaks[i];


    double sVal = currVec[s].first;
    double eVal = currVec[e].second;

    splitsNow.push_back({sVal, eVal});
  }

  return splitsNow;
}



//Get run number from time
ExpRun getRun(map<ExpRun, pair<double, double>> runs, double t)
{
  ExpRun rFound(-1, -1);
  int nFound = 0;
  for (auto r : runs) {
    if (r.second.first <= t && t < r.second.second) {
      rFound = r.first;
      ++nFound;
    }
  }

  assert(nFound == 1);
  assert(rFound != ExpRun(-1, -1))
  return rFound;
}


//Get intervals separated according the runs
vector<map<ExpRun, pair<double, double>>> breaks2intervalsSep(map<ExpRun, pair<double, double>> runsMap,
    vector<pair<double, double>> currVec, vector<int> breaks)
{
  vector<map<ExpRun, pair<double, double>>> splitsNow(breaks.size() + 1);
  for (int i = 0; i < int(breaks.size()) + 1; ++i) {
    int s = (i == 0) ? 0 : breaks[i - 1] + 1;
    int e = (i == int(breaks.size())) ? currVec.size() - 1 : breaks[i];


    for (int k = s; k <= e; ++k) {
      ExpRun r = getRun(runsMap, (currVec[k].first + currVec[k].second) / 2.);
      if (splitsNow[i].count(r)) {
        splitsNow[i].at(r).first  = min(splitsNow[i].at(r).first, currVec[k].first);
        splitsNow[i].at(r).second = max(splitsNow[i].at(r).second, currVec[k].second);
      } else {
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
vector<vector<map<ExpRun, pair<double, double>>>>  Splitter::getIntervals(map<ExpRun, pair<double, double>> runs, double tBestSize,
    double tBestVtx, double GapPenalty)
{
  // Divide into small intervals
  auto smallRuns = splitToSmall(runs, 0.1);

  gapPenalty = GapPenalty;

  // Calculate breaks for SizeIntervals
  tBest = tBestSize;
  auto breaksSize = dynamicBreaks(smallRuns);


  vector<vector<pair<double, double>>> splits; //split intervals
  vector<vector<map<ExpRun, pair<double, double>>>> splitsRun; //split intervals with runs


  // Calculate breaks for VtxIntervals
  tBest = tBestVtx;
  vector<int> breaksVtx;
  for (int i = 0; i < int(breaksSize.size()) + 1; ++i) { //loop over all size intervals
    int s = (i == 0) ? 0 : breaksSize[i - 1] + 1;
    int e = (i == int(breaksSize.size())) ? smallRuns.size() - 1 : breaksSize[i];

    auto currVec = Slice(smallRuns, s, e);
    //for (auto x : currVec)
    // cout << "R " << x.first << " " << x.second << endl;

    auto breaksSmall = dynamicBreaks(currVec);
    //cout << "smallBreaksSize " << breaksSmall.size() << endl;


    vector<pair<double, double>> splitsNow = breaks2intervals(currVec, breaksSmall);
    auto splitSeg = breaks2intervalsSep(runs, currVec, breaksSmall);

    splits.push_back(splitsNow);
    splitsRun.push_back(splitSeg);

    if (s != 0) breaksVtx.push_back(s - 1);
    for (auto b : breaksSmall)
      breaksVtx.push_back(b + s);
    if (e != int(breaksSize.size())) breaksVtx.push_back(e);
  }

  return splitsRun;
}

