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

namespace Belle2 {

  namespace BoostVectorCalib {


/// read events from TTree to std::vector
    vector<Event> getEvents(TTree* tr)
    {

      vector<Event> events;
      events.reserve(tr->GetEntries());

      Event evt;

      tr->SetBranchAddress("run", &evt.run);
      tr->SetBranchAddress("exp", &evt.exp);
      tr->SetBranchAddress("event", &evt.evtNo);
      tr->SetBranchAddress("mu0_d0", &evt.mu0.d0);
      tr->SetBranchAddress("mu1_d0", &evt.mu1.d0);
      tr->SetBranchAddress("mu0_z0", &evt.mu0.z0);
      tr->SetBranchAddress("mu1_z0", &evt.mu1.z0);

      tr->SetBranchAddress("mu0_tanlambda", &evt.mu0.tanlambda);
      tr->SetBranchAddress("mu1_tanlambda", &evt.mu1.tanlambda);


      tr->SetBranchAddress("mu0_phi0", &evt.mu0.phi0);
      tr->SetBranchAddress("mu1_phi0", &evt.mu1.phi0);

      tr->SetBranchAddress("time", &evt.t); //time in hours


      for (int i = 0; i < tr->GetEntries(); ++i) {
        tr->GetEntry(i);
        evt.toMicroM();

        evt.nBootStrap = 1;
        evt.isSig = true;
        events.push_back(evt);
      }

      //sort by time
      sort(events.begin(), events.end(), [](Event e1, Event e2) {return e1.t < e2.t;});


      return events;
    }


// Returns tuple with the beamspot parameters
    tuple<vector<TVector3>, vector<TMatrixDSym>, TMatrixDSym>  runBoostVectorAnalysis(vector<Event> evts,
        const vector<double>& splitPoints)
    {
      vector<TVector3>     boostVec;
      vector<TMatrixDSym>  boostVecUnc;
      TMatrixDSym          boostVecSpred;

      return make_tuple(boostVec, boostVecUnc, boostVecSpred);
    }

  }
}
