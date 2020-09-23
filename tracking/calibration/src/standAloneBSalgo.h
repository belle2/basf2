/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Radek Zlebcik                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>
#include <tuple>
#include <TVector3.h>
#include <TMatrixDSym.h>
#include <TTree.h>
#include "Splitter.h"

namespace Belle2 {

  /** struct with expNum, runNum, evtNum */
  struct ExpRunEvt {
    int exp; ///< experiment number
    int run; ///< run number
    int evt; ///< event number
    /** simple constructor */
    ExpRunEvt(int Exp, int Run, int Evt) : exp(Exp), run(Run), evt(Evt) {}
  };


  /** track parameters (neglecting curvature) */
  struct Track {
    double d0;   ///< signed distance to the POCA in x-y plane
    double z0;   ///< z position of the POCA
    double phi0; ///< angle of the track in x-y plane
    double tanlambda; ///< tan of angle in the z-dirrection, if zero no movement in z
  };

  /** Event containing two tracks */
  struct Event {
    int exp;   ///< experiment number
    int run;   ///< run number
    int evtNo; ///< event number


    Track mu0; ///< track in the event
    Track mu1; ///< other track in the event

    int nBootStrap; ///< random bootstrap weight (n=1 -> original sample)
    bool isSig;     ///< is not removed by quality cuts?
    double t;       ///< time of event [hours]

    /** from [cm] to [um] */
    void toMicroM()
    {
      mu0.d0 *= 1e4;
      mu1.d0 *= 1e4;
      mu0.z0 *= 1e4;
      mu1.z0 *= 1e4;
    }
  };


  // get vector of events from TTree
  std::vector<Event> getEvents(TTree* tr);

  // run the BeamSpot analysis using the splitPoints for BS-position parameters
  std::tuple<std::vector<TVector3>, std::vector<TMatrixDSym>, TMatrixDSym>  runBeamSpotAnalysis(std::vector<Event> evts,
      const std::vector<double>& splitPoints);

  // Convert splitPoints [hours] to breakPoints in ExpRunEvt
  std::vector<ExpRunEvt> convertSplitPoints(const std::vector<Event>& events, std::vector<double> splitPoints);

  // get exp-run-evt number from time tEdge [hours]
  ExpRunEvt getPosition(const std::vector<Event>& events, double tEdge);

  // get the map of runs, where each run contains pair with start/end time [hours]
  std::map<ExpRun, std::pair<double, double>> getRunInfo(const std::vector<Event>& evts);

}
