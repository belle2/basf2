/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Radek Zlebcik                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <iostream>
#include <vector>
#include <tuple>
#include "TTree.h"
#include "TVector3.h"
#include <Eigen/Dense>

#include "minimizer.h"

//if compiled within BASF2
#ifdef _PACKAGE_
#include <tracking/calibration/BoostVectorStandAlone.h>
#include <tracking/calibration/Splitter.h>
#include <tracking/calibration/tools.h>
#else
#include "BoostVectorStandAlone.h"
#include "Splitter.h"
#include "tools.h"
#endif

using namespace std;
using Eigen::VectorXd;

namespace Belle2 {

  namespace BoostVectorCalib {

    double median(double* v, int n)
    {
      assert(n > 2);

      if (n % 2 == 1) { // odd number of elements
        std::nth_element(v, v + n / 2, v + n);
        return v[n / 2];
      } else { //even
        std::nth_element(v, v + n / 2, v + n);
        double v1 = v[n / 2];

        std::nth_element(v, v + n / 2 - 1, v + n);
        double v2 = v[n / 2 - 1];

        return (v1 + v2) / 2;
      }
    }





/// read events from TTree to std::vector
    vector<Event> getEvents(TTree* tr)
    {

      vector<Event> events;
      events.reserve(tr->GetEntries());

      Event evt;

      tr->SetBranchAddress("run", &evt.run);
      tr->SetBranchAddress("exp", &evt.exp);
      tr->SetBranchAddress("event", &evt.evtNo);

      TVector3* p0 = nullptr;
      TVector3* p1 = nullptr;

      tr->SetBranchAddress("mu0_p", &p0);
      tr->SetBranchAddress("mu1_p", &p1);

      tr->SetBranchAddress("mu0_pid", &evt.mu0.pid);
      tr->SetBranchAddress("mu1_pid", &evt.mu1.pid);


      tr->SetBranchAddress("time", &evt.t); //time in hours


      for (int i = 0; i < tr->GetEntries(); ++i) {
        tr->GetEntry(i);

        evt.mu0.p = *p0;
        evt.mu1.p = *p1;

        evt.nBootStrap = 1;
        evt.isSig = true;
        events.push_back(evt);
      }

      //sort by time
      sort(events.begin(), events.end(), [](Event e1, Event e2) {return e1.t < e2.t;});


      return events;
    }


    struct FunBoost {
      VectorXd vCos, vSin, vData, res;

      double operator()(double c, double s)
      {

        res = -c * vCos - s * vSin - vData;

        res =  res.array().square();

        double* resA = res.data();

        return median(resA, res.rows());
      }
    };


    VectorXd toVec(vector<double> v)
    {
      VectorXd vec(v.size());
      for (unsigned i = 0; i < v.size(); ++i)
        vec[i] = v[i];
      return vec;
    }

// The input boost vector is used (angleX, anlgeY, rap) (in mili-units)
    vector<double> getRapidities(vector<TVector3> vecs, vector<double> boostDir)
    {
      TVector3 boost(boostDir[0] / 1e3, boostDir[1] / 1e3, 1);
      boost = boost.Unit();

      double th0 = vecs[0].Angle(boost);
      double th1 = vecs[1].Angle(boost);

      double C0, C1;
      double mL  =  105.7; //muon mass
      {
        double pMu0 = vecs[0].Mag();
        double pMu1 = vecs[1].Mag();

        C0 = 1. / sqrt(pow(1e-3 * mL / pMu0, 2) + 1);
        C1 = 1. / sqrt(pow(1e-3 * mL / pMu1, 2) + 1);
      }


      double y0 = atanh(cos(th0) * C0);
      double y1 = atanh(cos(th1) * C1);

      return {y0, y1};
    }

// Fit xy widths (including XZ, YZ slopes), no prior
    vector<double> fitBoostFast(const vector<Event>& evts)
    {
      vector<double> vCos, vSin, vData;

      for (auto e : evts) {
        for (int i = 0; i < e.nBootStrap * e.isSig; ++i) {
          vector<TVector3> vecs = {e.mu0.p, e.mu1.p};


          TVector3 n = vecs[0].Cross(vecs[1]);
          double phi = n.Phi();
          double angle = M_PI / 2 - n.Theta();
          auto res = make_pair(phi, 1e3 * tan(angle));

          vCos.push_back(cos(res.first));
          vSin.push_back(sin(res.first));
          vData.push_back(res.second);
        }
      }

      //cout << "data size: " << vData.size() << endl;

      FunBoost fun;
      fun.vCos  = toVec(vCos);
      fun.vSin  = toVec(vSin);
      fun.vData = toVec(vData);

      auto res = getMinimum(fun, 130 , 170, -20, 20);

      //cout << res[0] <<" "<< res[1] << endl;

      return res;
    }



    vector<Event> filter(vector<Event> evts, vector<double> boostDir, double pidCut, double rapCut)
    {
      vector<Event> evtsF;

      for (auto e : evts) {
        for (int i = 0; i < e.nBootStrap * e.isSig; ++i) {

          //Keep only muons
          if (e.mu0.pid < pidCut || e.mu1.pid < pidCut) {
            continue;
          }

          vector<TVector3> vecs = {e.mu0.p, e.mu1.p};
          vector<double> raps = getRapidities(vecs, boostDir);

          double yDiff = abs(raps[0] - raps[1]) / 2;

          // Exclude events with fwd/bwd muons where det acc is limited
          if (yDiff > rapCut) continue;

          evtsF.push_back(e);
        }
      }

      return evtsF;
    }



// Fit xy widths (including XZ, YZ slopes), no prior
    double fitBoostMagnitude(const vector<Event>& evts, vector<double> boostDir)
    {

      vector<double> yAvgVec;

      for (auto e : evts) {
        for (int i = 0; i < e.nBootStrap * e.isSig; ++i) {
          vector<TVector3> vecs = {e.mu0.p, e.mu1.p};

          vector<double> raps = getRapidities(vecs, boostDir);

          double yAvg = (raps[0] + raps[1]) / 2.;
          yAvgVec.push_back(yAvg);
        }
      }

      return median(yAvgVec.data(), yAvgVec.size());
    }




    TVector3 getBoostVector(vector<Event> evts)
    {
      evts = filter(evts, {151.986 /*TanNomAngle*/, 0}, 0.9/*pid*/,  1.0);
      vector<double> boostDir = fitBoostFast(evts);
      double yMag = fitBoostMagnitude(evts, boostDir);
      double beta = tanh(yMag);

      TVector3 bVec = TVector3(boostDir[0] / 1e3, boostDir[1] / 1e3, 1);
      bVec.Unit();
      bVec *= beta;

      return bVec;
    }




    vector<vector<Event>> separate(const vector<Event>& evts, const vector<double>& splitPoints)
    {
      vector<vector<Event>> evtsOut(splitPoints.size() + 1);

      for (const auto& ev : evts) {
        for (int i = 0; i < int(splitPoints.size()) - 1; ++i) {
          if (splitPoints[i] < ev.t  && ev.t <  splitPoints[i + 1]) {
            evtsOut[i + 1].push_back(ev);
            break;
          }
        }

        if (splitPoints.size() >= 1) {
          if (ev.t < splitPoints.front()) evtsOut.front().push_back(ev);
          if (ev.t > splitPoints.back())  evtsOut.back().push_back(ev);
        } else {
          evtsOut[0].push_back(ev);
        }
      }
      return evtsOut;
    }





// Returns tuple with the beamspot parameters
    tuple<vector<TVector3>, vector<TMatrixDSym>, TMatrixDSym>  runBoostVectorAnalysis(vector<Event> evts,
        const vector<double>& splitPoints)
    {
      int n = splitPoints.size() + 1;
      vector<TVector3>     boostVec(n);
      vector<TMatrixDSym>  boostVecUnc(n);
      TMatrixDSym          boostVecSpred;

      vector<vector<Event>> evtsSep = separate(evts, splitPoints);

      //TVector3 getBoostVector(vector<Event> evts);

      for (int i = 0; i < n; ++i) {
        boostVec[i]    = getBoostVector(evtsSep[i]);
        boostVecUnc[i].ResizeTo(3, 3);
        boostVecUnc[i] = TMatrixDSym(3);

        cout << evtsSep[i][0].run << " " <<  evtsSep[i].size() << " :  " << 1e3 * boostVec[i].X() / boostVec[i].Z() << " " << 1e3 *
             boostVec[i].Y() / boostVec[i].Z() << " :  " << 1e3 * boostVec[i].Mag() << endl;
      }
      boostVecSpred.ResizeTo(3, 3);
      boostVecSpred = TMatrixDSym(3);

      return make_tuple(boostVec, boostVecUnc, boostVecSpred);
    }

  }
}
