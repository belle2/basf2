/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <iostream>
#include <vector>
#include <tuple>
#include <TTree.h>
#include <TVector3.h>
#include <Eigen/Dense>

//if compiled within BASF2
#ifdef _PACKAGE_
#include <tracking/calibration/InvariantMassStandAlone.h>
#include <tracking/calibration/Splitter.h>
#include <tracking/calibration/tools.h>
#else
#include <InvariantMassStandAlone.h>
#include <Splitter.h>
#include <tools.h>
#endif

using Eigen::MatrixXd;
using Eigen::VectorXd;

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
      tr->SetBranchAddress("time", &evt.t); //time in hours


      tr->SetBranchAddress("mBC", &evt.mBC);
      tr->SetBranchAddress("deltaE", &evt.deltaE);
      tr->SetBranchAddress("pdg", &evt.pdg);
      tr->SetBranchAddress("mode", &evt.mode);
      tr->SetBranchAddress("Kpid", &evt.Kpid);
      tr->SetBranchAddress("R2", &evt.R2);
      tr->SetBranchAddress("mD", &evt.mD);
      tr->SetBranchAddress("dmDstar", &evt.dmDstar);





      for (int i = 0; i < tr->GetEntries(); ++i) {
        tr->GetEntry(i);

        evt.nBootStrap = 1;
        evt.isSig = true;
        events.push_back(evt);
      }

      //sort by time
      sort(events.begin(), events.end(), [](Event e1, Event e2) {return e1.t < e2.t;});


      return events;
    }


    /** run the collision invariant mass calibration */
    vector<double>  getInvMassPars(const vector<Event>& /*evts*/)
    {
      return { -1, -1, -1}; //dummy values
    }



    // Returns tuple with the invariant mass parameters
    // cppcheck-suppress passedByValue
    tuple<vector<VectorXd>, vector<MatrixXd>, MatrixXd>  runInvariantMassAnalysis(vector<Event> evts,
        const vector<double>& splitPoints)
    {
      int n = splitPoints.size() + 1;
      //no split points, i.e. the spead interval identical to the center interval
      B2ASSERT("No split points for InvariantMass calibration", n == 1);
      vector<VectorXd>     invMassVec(n);
      vector<MatrixXd>  invMassVecUnc(n);
      MatrixXd          invMassVecSpred;

      invMassVec[0].resize(1);       //1D vector for center of the 1D Gauss
      invMassVecUnc[0].resize(1, 1); //1x1 matrix covariance mat of the center
      invMassVecSpred.resize(1, 1);  //1x1 matrix for spread of the 1D Gauss

      auto res =  getInvMassPars(evts);
      invMassVec[0](0) = res[0];
      invMassVecUnc[0](0, 0) = res[1];
      invMassVecSpred(0, 0) = res[2];


      return make_tuple(invMassVec, invMassVecUnc, invMassVecSpred);
    }

  }
}
