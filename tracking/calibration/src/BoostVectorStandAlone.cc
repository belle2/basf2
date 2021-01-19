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
#include <TTree.h>
#include <TVector3.h>
#include <Eigen/Dense>

//if compiled within BASF2
#ifdef _PACKAGE_
#include <tracking/calibration/BoostVectorStandAlone.h>
#include <tracking/calibration/Splitter.h>
#include <tracking/calibration/tools.h>
#include <tracking/calibration/minimizer.h>
#else
#include "BoostVectorStandAlone.h"
#include "Splitter.h"
#include "tools.h"
#include "minimizer.h"
#endif

using namespace std;
using Eigen::VectorXd;
using Eigen::Vector3d;
using Eigen::MatrixXd;
using Eigen::Matrix3d;

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
      MatrixXd mat;
      VectorXd res;

      double operator()(double c, double s)
      {
        Vector3d pars(-c, -s, -1);
        res = mat * pars;
        res =  res.array().square();

        double* resA = res.data();

        return median(resA, res.rows());
      }
    };



    MatrixXd toMat(const vector<vector<double>>& vecs)
    {
      MatrixXd mat(vecs[0].size(), vecs.size());

      for (unsigned k = 0; k < vecs.size(); ++k)
        for (unsigned i = 0; i < vecs[k].size(); ++i)
          mat(i, k) = vecs[k][i];
      return mat;
    }


// The input boost vector is used (angleX, anlgeY, rap) (in mili-units)
    vector<double> getRapidities(vector<TVector3> vecs, vector<double> boostDir)
    {
      TVector3 boost(boostDir[0] / 1e3, boostDir[1] / 1e3, 1);
      boost = boost.Unit();

      double th0 = vecs[0].Angle(boost);
      double th1 = vecs[1].Angle(boost);

      double C0, C1;
      double mL  = 105.6583745; //muon mass [MeV]
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

      for (const auto& e : evts) {
        if (e.nBootStrap * e.isSig == 0) continue;
        vector<TVector3> vecs = {e.mu0.p, e.mu1.p};


        TVector3 n = vecs[0].Cross(vecs[1]);
        double phi = n.Phi();
        double angle = M_PI / 2 - n.Theta();
        auto res = make_pair(phi, 1e3 * tan(angle));

        for (int i = 0; i < e.nBootStrap * e.isSig; ++i) {
          vCos.push_back(cos(res.first));
          vSin.push_back(sin(res.first));
          vData.push_back(res.second);
        }
      }


      FunBoost fun;

      fun.mat = toMat({vCos, vSin, vData});
      fun.res.resize(vCos.size());

      auto res = getMinimum(fun, 130 , 170, -20, 20);


      return res;
    }



    vector<Event> filter(const vector<Event>& evts, const vector<double>& boostDir, double pidCut, double rapCut)
    {
      vector<Event> evtsF;

      for (const auto& e : evts) {
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
    double fitBoostMagnitude(const vector<Event>& evts, const vector<double>& boostDir)
    {

      vector<double> yAvgVec;

      for (auto e : evts) {
        if (e.nBootStrap * e.isSig == 0) continue;
        vector<TVector3> vecs = {e.mu0.p, e.mu1.p};

        vector<double> raps = getRapidities(vecs, boostDir);

        double yAvg = (raps[0] + raps[1]) / 2.;
        for (int i = 0; i < e.nBootStrap * e.isSig; ++i) {
          yAvgVec.push_back(yAvg);
        }
      }

      return median(yAvgVec.data(), yAvgVec.size());
    }


    struct vectorVar {
      vector<Vector3d> vecs;

      void add(Vector3d v) { vecs.push_back(v); }

      Vector3d getNom() const { return vecs[0]; }

      MatrixXd getCov() const
      {
        Matrix3d res = Matrix3d::Zero();
        for (unsigned i = 1; i < vecs.size(); ++i) {
          Vector3d diff = vecs[i] - vecs[0];
          res += diff * diff.transpose();
        }
        if (vecs.size() > 1)
          res *= 1. / (vecs.size() - 1);
        return res;

      }


    };


    TVector3 getBoostVector(const vector<Event>& evts)
    {
      vector<double> boostDir = fitBoostFast(evts);
      double yMag = fitBoostMagnitude(evts, boostDir);
      double beta = tanh(yMag); //from rapidity to velocity

      TVector3 bVec = TVector3(boostDir[0] / 1e3, boostDir[1] / 1e3, 1);
      bVec = beta * bVec.Unit();
      return bVec;
    }

    pair<Vector3d, Matrix3d>  getBoostAndError(vector<Event> evts)
    {
      evts = filter(evts, {151.986 /*TanNomAngle*/, 0}, 0.9/*muon pid*/,  1.0 /*rap cut*/);

      vectorVar var;

      const int nBoost = 10;
      for (int i = 0; i < nBoost; ++i) {
        if (i != 0)
          for (auto& e : evts)
            e.nBootStrap = gRandom->Poisson(1);

        TVector3 b = getBoostVector(evts);
        Vector3d boost(b.X(), b.Y(), b.Z());
        var.add(boost);
      }
      return make_pair(var.getNom(), var.getCov());

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
    // cppcheck-suppress passedByValue
    tuple<vector<VectorXd>, vector<MatrixXd>, MatrixXd>  runBoostVectorAnalysis(vector<Event> evts,
        const vector<double>& splitPoints)
    {
      int n = splitPoints.size() + 1;
      vector<VectorXd>     boostVec(n);
      vector<MatrixXd>  boostVecUnc(n);
      MatrixXd          boostVecSpred;

      vector<vector<Event>> evtsSep = separate(evts, splitPoints);


      for (int i = 0; i < n; ++i) {

        tie(boostVec[i], boostVecUnc[i]) =  getBoostAndError(evtsSep[i]);


        // TanThetaXZ of boostVector
        double aX = 1e3 * boostVec[i][0] / boostVec[i][2];
        // TanThetaYZ of boostVector
        double aY = 1e3 * boostVec[i][1] / boostVec[i][2];

        // TanThetaXZ unc of boostVector
        double eX = 1e3 * sqrt(boostVecUnc[i](0, 0)) / boostVec[i](2);
        // TanThetaYZ unc of boostVector
        double eY = 1e3 * sqrt(boostVecUnc[i](1, 1)) / boostVec[i](2);

        B2INFO(evtsSep[i][0].run << " " << i << " " <<  evtsSep[i].size() << " :  " << aX << "+-" << eX << "  " << aY << "+-" << eY <<
               "  :  " << 1e3 * boostVec[i].norm());
      }

      // Spread is currently not calculated
      boostVecSpred = MatrixXd::Zero(3, 3);

      return make_tuple(boostVec, boostVecUnc, boostVecSpred);
    }

  }
}
