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

//if compiled within BASF2
#ifdef _PACKAGE_
#include <tracking/calibration/InvariantMassStandAlone.h>
#include <tracking/calibration/Splitter.h>
#include <tracking/calibration/tools.h>
#else
#include "InvariantMassStandAlone.h"
#include "Splitter.h"
#include "tools.h"
#endif

using namespace std;

namespace Belle2 {

  namespace InvariantMassCalib {



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



    vector<double>  getInvMassPars(const vector<Event>& evts)
    {
      return { -1, -1, -1}; //dummy values
    }



    // Returns tuple with the invariant mass parameters
    tuple<vector<TVector3>, vector<TMatrixDSym>, TMatrixDSym>  runInvariantMassAnalysis(vector<Event> evts,
        const vector<double>& splitPoints)
    {
      int n = splitPoints.size() + 1;
      assert(n == 1);  //no split points, the spead interval identical to the center interval
      vector<TVector3>     invMassVec(n);
      vector<TMatrixDSym>  invMassVecUnc(n);
      TMatrixDSym          invMassVecSpred;



      invMassVecUnc[0].ResizeTo(3, 3);
      invMassVecSpred.ResizeTo(3, 3);

      auto res =  getInvMassPars(evts);
      invMassVec[0].SetX(res[0]);
      invMassVecUnc[0](0, 0) = res[1];
      invMassVecSpred(0, 0) = res[2];


      return make_tuple(invMassVec, invMassVecUnc, invMassVecSpred);
    }

  }
}
