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


#undef assert
#define assert(arg)  { if((arg) == false) {cout << __FILE__ <<", line "<<__LINE__ << endl << "" << #arg << " failed" << endl;   exit(0);} }





vector<pair<double, double>> Slice(vector<pair<double, double>> vec, int s, int e)
{
  return vector<pair<double, double>>(vec.begin() + s, vec.begin() + e + 1);
}

map<ExpRun, pair<double, double>> load()
{
  ifstream file("inter.txt");

  map<ExpRun, pair<double, double>> runs;
  while (file.good()) {
    int r;
    double a1, a2;
    file >> r >> a1 >> a2;
    if (!file.good()) break;
    cout << r << " : " << a1 << " " << a2 << endl;
    ExpRun R(12, r);
    assert(runs.count(R) == 0);
    runs[R] = {a1, a2};
  }

  return runs;
}

void shift(map<ExpRun, pair<double, double>>& runs)
{
  double Min = 1e30;
  for (auto r : runs) {
    Min = min(Min, r.second.first);
    Min = min(Min, r.second.second);
  }

  for (auto& r : runs) {
    auto& I = r.second;
    I.first  = (I.first  - Min);
    I.second = (I.second - Min);
  }
}

void filter(map<ExpRun, pair<double, double>>& runs, double cut)
{
  map<ExpRun, pair<double, double>> runsCopy;

  for (auto r : runs) {
    auto& I = r.second;
    double d =  I.second - I.first;
    if (d > cut)
      runsCopy[r.first] = r.second;
  }

  runs = runsCopy;
}


void plotRuns(vector<pair<double, double>>  runs)
{
  TGraphErrors* gr = new TGraphErrors();


  for (auto r : runs) {
    double m = (r.first + r.second) / 2;
    double e = (r.second - r.first) / 2;

    gr->SetPoint(gr->GetN(), m, 1);
    gr->SetPointError(gr->GetN() - 1, e, 0);
  }

  //TCanvas *c = new TCanvas("cann", "", 600, 200);
  gStyle->SetEndErrorSize(6);

  gr->SetLineWidth(1);
  gr->SetMarkerSize(40);
  gr->Draw("ape");
  gr->GetYaxis()->SetRangeUser(-10, 10);
  gr->GetXaxis()->SetRangeUser(0, 256);

  gr->GetXaxis()->SetTitle("time [hours]");

}

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

    cout << "Helenka " << a << " " << b << endl;

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


double getRawTime(const vector<pair<double, double>>&  runs)
{
  return (runs.back().second  -  runs[0].first);
}


double getNetTime(const vector<pair<double, double>>&  runs)
{
  double netTime = 0;
  for (auto r : runs)
    netTime += r.second - r.first;
  return netTime;
}


double getMaxGap(const vector<pair<double, double>>&  runs)
{

  double gapMax = 0;
  for (int i = 0; i < int(runs.size()) - 1; ++i) {
    double d = runs[i + 1].first - runs[i].second;
    gapMax = max(gapMax, d);
  }

  return gapMax;
}



vector<pair<double, double>> merge(vector<pair<double, double>>  a, vector<pair<double, double>>  b)
{
  if (a[0].first < b[0].first) {
    a.insert(a.end(), b.begin(), b.end());
    return a;
  } else {
    b.insert(b.end(), a.begin(), a.end());
    return b;
  }
}

double distance(vector<pair<double, double>>  a, vector<pair<double, double>>  b)
{
  double x = (a.front().first + a.back().second) / 2;
  double y = (b.front().first + b.back().second) / 2;

  return pow(x - y, 2);
}


double lossFunction(const vector<pair<double, double>>&  a)
{
  double rawTime = getRawTime(a);
  double netTime = getNetTime(a);

  const double tBest = 1; //1 hour as best

  double q = pow(rawTime / netTime - 1, 2) + 10 * pow(netTime / tBest - 1, 2);

  return q;
}


double tBest = 0.5; //0.5 hour as default
double gapPenalty = 8;

double Splitter::lossFunction(const vector<pair<double, double>>&  vec, int s, int e)
{

  static int tBestCheck = tBest;
  if (tBestCheck != tBest) {
    cout << "radek " << tBest << endl;
    //exit(0);
  }

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





/*
vector<vector<pair<double,double>>> cluster(vector<pair<double,double>>  runs)
{
    //to super runs
    vector<vector<pair<double,double>>> sruns;
    for(auto r : runs) {
        sruns.push_back({r});
    }

    vector<vector<pair<double,double>>> srunsFinal;

    //cluster
    for(int k = 0; k < 300; ++k) {

        //find the smallest run

        if(sruns.size() == 0)
            break;
        if(sruns.size() == 1) {
            srunsFinal.push_back(sruns[0]);
            break;
        }
        cout << sruns.size() << endl;

        int iMin = -1;
        double dMin = 1e20;
        int dir = 0;
        for(int i = 0; i < int(sruns.size()); ++i) {
            double lDist = 1e10, rDist = 1e10;
            if(i >= 1)              lDist = sruns[i][0].first  -  sruns[i-1].back().second;
            if(i <= sruns.size()-2) rDist = sruns[i+1][0].first  -  sruns[i].back().second;

            double dMinNow = min(lDist, rDist);

            if(dMinNow < dMin) {
                dMin = dMinNow;
                iMin = i;
                if(dMinNow == lDist)
                    dir = -1;
                else
                    dir = +1;

            }
        }

        if(iMin == -1) break;


        auto srunM = merge(sruns[iMin], sruns[iMin+dir]);
        //if enough events -> remove
        if(getNetTime(srunM) > 0.5) {
            cout << "Enough events, puting to final intervals" << endl;
            srunsFinal.push_back(srunM);
            if(dir < 0)
                sruns.erase(sruns.begin()+iMin-1, sruns.begin()+iMin+1);
            else
                sruns.erase(sruns.begin()+iMin, sruns.begin()+iMin+2);
        }
        else {
            cout << "Merging" << endl;
            sruns[iMin] = srunM;
            sruns.erase(sruns.begin()+iMin+dir);
        }
    }


    return srunsFinal;
}
*/


/*
vector<vector<pair<double,double>>> gapBasedClusters(vector<pair<double,double>>  runs)
{
    //to super runs
    vector<vector<pair<double,double>>> sruns;
    for(auto r : runs) {
        sruns.push_back({r});
    }

    vector<vector<pair<double,double>>> srunsFinal;

    //cluster
    for(int k = 0; k < 300; ++k) {

        //find the largest gap

        if(sruns.size() == 0)
            break;
        if(sruns.size() == 1) {
            srunsFinal.push_back(sruns[0]);
            break;
        }
        cout << sruns.size() << endl;

        int iMin = -1;
        double dMin = 1e20;
        int dir = 0;
        for(int i = 0; i < int(sruns.size()); ++i) {
            double lDist = 1e10, rDist = 1e10;
            if(i >= 1)              lDist = sruns[i][0].first  -  sruns[i-1].back().second;
            if(i <= sruns.size()-2) rDist = sruns[i+1][0].first  -  sruns[i].back().second;

            double dMinNow = min(lDist, rDist);

            if(dMinNow < dMin) {
                dMin = dMinNow;
                iMin = i;
                if(dMinNow == lDist)
                    dir = -1;
                else
                    dir = +1;

            }
        }

        if(iMin == -1) break;


        auto srunM = merge(sruns[iMin], sruns[iMin+dir]);
        //if enough events -> remove
        if(getNetTime(srunM) > 0.5) {
            cout << "Enough events, puting to final intervals" << endl;
            srunsFinal.push_back(srunM);
            if(dir < 0)
                sruns.erase(sruns.begin()+iMin-1, sruns.begin()+iMin+1);
            else
                sruns.erase(sruns.begin()+iMin, sruns.begin()+iMin+2);
        }
        else {
            cout << "Merging" << endl;
            sruns[iMin] = srunM;
            sruns.erase(sruns.begin()+iMin+dir);
        }
    }


    return srunsFinal;
}
*/






/*
void getVectors( vector<vector<pair<double,double>>> &sruns,
                 vector<double> &orgLoss,
                 vector<double> &rLloss, vector<double> &rRloss,
                 vector<double> &aLloss, vector<double> &aRloss)
{

    orgLoss.clear();
    rLloss.clear();
    rRloss.clear();
    aLloss.clear();
    aRloss.clear();

    for(auto &sr : sruns) {
        //org loss function
        double loss = lossFunction(sr);
        orgLoss.push_back(loss);

        //remove from right
        auto srR = sr;
        srR.pop_back();
        double lessR = lossFunction(srR);
        rRloss.push_back(lessR);

        //remove from left
        auto srL = sr;
        srL.erase(srL.begin());
        double lessL = lossFunction(srL);
        rLloss.push_back(lessL);

    }

    for(int i = 0; i < sruns.size(); ++i) {
        // add to right
        auto srR = sruns[i];
        if(i != sruns.size() -1) srR.push_back(sruns[i+1].front());
        double moreR = lossFunction(srR);
        aRloss.push_back(moreR);

        // add to left
        auto srL = sruns[i];
        if(i != 0) srL.insert(srL.begin(), sruns[i-1].back());
        double moreL = lossFunction(srL);
        aLloss.push_back(moreL);
    }

}
*/



vector<vector<pair<double, double>>> seqSplitter(vector<pair<double, double>>  runs)
{
  const double netTime = 0.7; // 1hour
  const double maxGap  = 0.5;

  double totTime = getNetTime(runs);

  cout << "totTime " << totTime << endl;

  // split into ~1m intervals
  vector<pair<double, double>> smallRuns;

  for (auto r : runs) {
    double runTime = getNetTime({r});
    int nSplits = runTime * 60; //1-m intervals
    nSplits = max(1, nSplits); //at least 1 interval

    for (int i = 0; i < nSplits; ++i) {
      double L = r.first + i * (runTime / nSplits);
      double H = r.first + (i + 1) * (runTime / nSplits);
      smallRuns.push_back({L, H});
    }
  }

  cout << "number of 1d intervals : " << smallRuns.size() << endl;

  // do ~1 hour preliminary segmentation

  vector<vector<pair<double, double>>> sruns;
  sruns.push_back({});
  //int nSplits = totTime / netTime; // 1 hour segmentation
  //int segmentSize = smallRuns.size() / nSplits;

  cout << "I am here " << endl;
  for (auto r : smallRuns) {

    sruns.back().push_back(r);

    if (getNetTime(sruns.back()) > netTime) //next interval
      sruns.push_back({});

    if (sruns.back().size() > 0 && getRawTime(sruns.back()) - getNetTime(sruns.back()) > maxGap) {

      auto el = sruns.back().back();
      sruns.back().pop_back();
      sruns.push_back({el});
    }
    assert((sruns.back().size() == 0 ||  getRawTime(sruns.back()) < 2.7));

    if (sruns.back().size() > 0) {
      if (getRawTime(sruns.back()) > 1.7) {
        cout << "Something bad" << endl;
        cout << getRawTime(sruns.back()) << endl;
        exit(0);
      }
    }

  }

  for (auto r : sruns) {
    double t = getRawTime(r);
    if (t > 8) {
      for (auto a : r) {
        cout << a.first << " " << a.second << endl;
      }
    }
  }


  return sruns;
}



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


/*
// https://en.wikipedia.org/wiki/Jenks_natural_breaks_optimization
vector<vector<pair<double,double>>> jenksBreaks(vector<pair<double,double>>  runs)
{

    double totTime = getNetTime(runs);

    cout << "totTime " << totTime << endl;

    // split into ~1m intervals
    vector<pair<double,double>> smallRuns;

    for(auto r : runs) {
        double runTime = getNetTime({r});
        int nSplits = runTime * 60; //1-m intervals
        nSplits = max(1, nSplits); //at least 1 interval

        for(int i = 0; i < nSplits; ++i) {
            double L = r.first + i*(runTime/nSplits);
            double H = r.first + (i+1)*(runTime/nSplits);
            smallRuns.push_back({L, H});
        }
    }

    cout << "number of 1d intervals : " << smallRuns.size() << endl;

    // do ~1 hour preliminary segmentation

    vector<vector<pair<double,double>>> sruns;
    sruns.push_back({});
    int nSplits = totTime / 1.; // 1 hour segmentation
    int segmentSize = smallRuns.size() / nSplits;

    int i = 0, j = 0;
    for(auto r : smallRuns) {

        sruns.back().push_back(r);
        if(i > segmentSize) {
            sruns.push_back({});
            i = 0;
        }
        ++i;
    }

    //return sruns;
    cout << sruns.size() << " "<< sruns.front().size() << endl;


    //iterate to get best segmentation
    vector<double> orgLoss;
    vector<double> rLloss, rRloss;
    vector<double> aLloss, aRloss;

    getVectors(sruns, orgLoss, rLloss, rRloss, aLloss, aRloss);


    for(int k = 0; k < 1000; ++k) {
        double maxGain = 0;

        //moving element to right
        int iR = -1;
        double changeR = 10000000;
        for(i = 0; i < sruns.size() - 1; ++i) {
            double change = - (orgLoss[i] + orgLoss[i+1]) + (rRloss[i] + aLloss[i+1]);
            if(change < changeR) {
                changeR = change;
                iR = i;
            }
        }

        //moving element to left
        int iL = -1;
        double changeL = 10000000;
        for(i = 1; i < sruns.size(); ++i) {
            double change = - (orgLoss[i] + orgLoss[i-1]) + (rRloss[i] + aLloss[i-1]);
            if(change < changeL) {
                changeL = change;
                iL = i;
            }
        }

        if(min(changeL, changeR) >= 0)
            break;

        //move to the right
        if(changeR < changeL) {
            auto el = sruns[iR].back();
            sruns[iR].pop_back();
            assert(sruns[iR].size() > 0);
            sruns[iR+1].insert(sruns[iR+1].begin(), el);
        }
        //move to the left
        else {
            auto el = sruns[iL].front();
            sruns[iL].erase(sruns[iL].begin());
            assert(sruns[iL].size() > 0);
            sruns[iL-1].push_back(el);
        }

        getVectors(sruns, orgLoss, rLloss, rRloss, aLloss, aRloss);

        cout << "Changes " <<    changeL <<" "<< changeR << endl;
        cout << "ids " << iL  <<" "<< iR << endl;
        cout << "Radek " << accumulate(orgLoss.begin(), orgLoss.end(), 0.) << endl;
    }


    cout << "Sizes are" << endl;

    for(auto r : sruns)
        cout << r.size() << endl;




    //to super runs
    //vector<vector<pair<double,double>>> sruns;
    //for(auto r : runs) {
    //    sruns.push_back({r});
    //}

    //vector<vector<pair<double,double>>> srunsFinal;

    ////cluster
    //for(int k = 0; k < 300; ++k) {

    //    sruns

    //}



    return {sruns};
}
*/

/*
double getLoss(const vector<pair<double,double>>  &vec,    vector<int> breaks)
{
    double tot = 0;

    if(breaks.size() == 0)
        return lossFunction(vec, 0, vec.size()-1);


    for(int i = 0; i < breaks.size() + 1; ++i) {
        int s = (i==0) ? 0 : breaks[i-1]+1;
        int e = (i==breaks.size()) ? vec.size()-1 : breaks[i];
        double r = lossFunction(vec, s, e);

        tot += r;
    }
    return tot;
}
*/



//vector<pair<double, vector<int>>> cache;


double Splitter::getMinLoss(const vector<pair<double, double>>&  vec,   int b, vector<int>& breaks)
{
  //cout << "Helenka tBest " << tBest << " "<< cache.size() <<" "<< b << " "<<cache[b].first << endl;
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
    if (tBest < 1.0)
      cout << "Checking b,i: " << b << " " << i << " : " << tot << endl;

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


vector<int> Splitter::dynamicBreaks(vector<pair<double, double>>  runs)
{
  cache.resize(runs.size());

  for (auto& c : cache)
    c = make_pair(-1.0, vector<int>({}));


  vector<int> breaks;
  double res = getMinLoss(runs, runs.size() - 1, breaks);

  cout << "Res is " << res << endl;
  return breaks;
}


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


    //double sVal = currVec[s].first;
    //double eVal = currVec[e].second;

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

  //vector<map<int,pair<double,double>>>

  // Calculate breaks for VtxIntervals
  tBest = tBestVtx;
  vector<int> breaksVtx;
  for (int i = 0; i < int(breaksSize.size()) + 1; ++i) { //loop over all size intervals
    int s = (i == 0) ? 0 : breaksSize[i - 1] + 1;
    int e = (i == int(breaksSize.size())) ? smallRuns.size() - 1 : breaksSize[i];

    cout << "Radecek " << s << " " << e << endl;

    auto currVec = Slice(smallRuns, s, e);
    for (auto x : currVec)
      cout << "R " << x.first << " " << x.second << endl;

    auto breaksSmall = dynamicBreaks(currVec);
    cout << "smallBreaksSize " << breaksSmall.size() << endl;


    //vector<pair<double,double>> splitsNow;
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


void splitter()
{
  auto runs = load();
  shift(runs);
  filter(runs, 2. / 60);

  //vector<pair<double,double>>  runs = { {0,0.1}, {0.11,0.4}, {0.4,0.8},  {1.7,2.3}, {2.35,2.45}, {6.3, 7.3}};

  Splitter splt;
  //splt.tBest = 4.;
  //splt.gapPenalty = 10;
  auto res = splt.getIntervals(runs, 2, 0.5, 10);

  for (auto r : res) {
    double s, e;
    tie(s, e) = Splitter::getStartEnd(r);
    cout << s << " " << e << endl;


    auto breaks =  Splitter::getBreaks(r);

  }

  /*
  auto smallRuns = splt.splitToSmall(runs, 0.1);

  tBest = 4.0; //0.5 hour as default
  gapPenalty = 10;
  auto breaksSize = splt.dynamicBreaks(smallRuns);

  vector<int> breaksMy = {8,17,26,35,44,53,61,71,82,94,101,110,118,127,137,146,157,165,177,187,197,205,212,223,234,244,254,264,275,286,296,299,312,314,319,327,335,344,354,364,374,384,394,403,411,418,423,433,443,454,464,474,484,494,504,514,516,526,538,548,558,567,576,588,598,609,620,630,640,650,658,666,675,684,692,701,710,719,729,734,742,752,760,767,768,774,782,785,791,798,806,810,822,833,846,861,871,881,890,900,909,918,929,941,951,961,970,979,988,996,1006,1015,1024,1033,1039,1050,1058,1066,1074,1075,1083,1091,1100,1109,1120,1132,1141,1144,1152,1161,1170,1179,1187,1195,1207,1211,1221,1230,1241,1249,1260,1271,1281,1291,1301,1310,1316,1323,1333,1342,1351,1360,1369,1377,1387,1399,1410,1420,1428,1437,1447,1456,1465,1474,1484,1494,1503,1514,1521,1530,1539,1546,1555,1564,1573,1582,1591,1600,1610,1620,1630,1640,1647,1657,1667,1676,1686,1696,1709,1719,1730,1742,1753,1767,1778,1787,1797,1807,1817,1827,1837,1849,1859,1870,1879,1888,1897,1900,1909,1920,1930,1940,1950};

  for(auto b : breaksSize)
      cout << b <<" ";
  cout << endl;



  tBest = 0.5; //0.5 hour as default
  gapPenalty = 10;

  vector<int> breaksVtx;
  for(int i = 0; i < breaksSize.size() + 1; ++i) {
     int s = (i==0) ? 0 : breaksSize[i-1]+1;
     int e = (i==breaksSize.size()) ? smallRuns.size()-1 : breaksSize[i];

     cout << "Radecek " << s << " "<< e << endl;

     auto currVec = Slice(smallRuns, s, e);
     for(auto x : currVec)
         cout << "R " << x.first <<" " << x.second << endl;

     auto breaksSmall = dynamicBreaks(currVec);
     cout << "smallBreaksSize " << breaksSmall.size() << endl;


    // if(i == 0) {
    //     for(auto x : breaksSmall)
    //         cout << "H " << x << endl;
    //     exit(0);
    // }


     if(s != 0) breaksVtx.push_back(s-1);
     for(auto b : breaksSmall)
        breaksVtx.push_back(b + s);
     if(e != breaksSize.size()) breaksVtx.push_back(e);
  }


  //cout <<  getLoss(smallRuns,   breaksMy) << endl;
  //cout <<  getLoss(runs,   {}) << endl;

  plotRuns(runs);
  plotSRuns(smallRuns, breaksVtx, 2);
  plotSRuns(smallRuns, breaksSize, 3);

  return;

  //auto result = jenksBreaks(runs);

  auto result = seqSplitter(runs);
  */

  //return;
  //printBySize(runs);
  //auto res = cluster(runs);
}
