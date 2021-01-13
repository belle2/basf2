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
#include <map>
#include <utility>
#include <tuple>
#include "TMatrixDSym.h"
#include "TVector3.h"

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
      TVector3 p;
      double pid;
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

    // Returns tuple with the beamspot parameters
    std::tuple<std::vector<TVector3>, std::vector<TMatrixDSym>, TMatrixDSym>  runInvariantMassAnalysis(std::vector<Event> evts,
        const std::vector<double>& splitPoints);

  }
}
