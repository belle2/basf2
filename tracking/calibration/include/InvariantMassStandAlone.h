/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Radek Zlebcik                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <vector>
#include <utility>
#include <tuple>
#include <TVector3.h>
#include <Eigen/Dense>

//If compiled within BASF2
#ifdef _PACKAGE_
#include <tracking/calibration/Splitter.h>
#else
#include "Splitter.h"
#endif

namespace Belle2 {
  namespace InvariantMassCalib {
    /** track parameters (neglecting curvature) */
    struct Track {
      TVector3 p; ///< momentum vector of the track
      double pid; ///< particle ID of mu/e separation
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

    };


    std::vector<Event> getEvents(TTree* tr);

    /** Run the InvariantMass analysis where splitPoints are the boundaries of the short calibration intervals
      @param evts: vector of events
      @param splitPoints: the vector containing times of the edges of the short calibration intervals [hours]
      @return A tuple containing vector with invariant mass, vector with invariant mass stat. error and a spread of the invariant mass
    */
    std::tuple<std::vector<Eigen::VectorXd>, std::vector<Eigen::MatrixXd>, Eigen::MatrixXd>  runInvariantMassAnalysis(
      std::vector<Event> evts,
      const std::vector<double>& splitPoints);

  }
}
