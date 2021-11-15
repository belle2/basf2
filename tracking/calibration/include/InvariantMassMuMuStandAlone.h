/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
#include <Splitter.h>
#endif

namespace Belle2 {
  namespace InvariantMassMuMuCalib {

    static const double realNaN = std::numeric_limits<double>::quiet_NaN();
    static const int intNaN     = std::numeric_limits<int>::quiet_NaN();
    static const TVector3 vecNaN(realNaN, realNaN, realNaN);

    /** track parameters (neglecting curvature) */
    struct Track {
      TVector3 p = vecNaN; ///< momentum vector of the track
      double pid = realNaN; ///< particle ID of mu/e separation
    };

    /** Event containing two tracks */
    struct Event {
      int exp   = intNaN;  ///< experiment number
      int run   = intNaN;  ///< run number
      int evtNo = intNaN;  ///< event number


      Track mu0; ///< track in the event
      Track mu1; ///< other track in the event
      double m; ///< dimuon mass

      int nBootStrap = intNaN; ///< random bootstrap weight (n=1 -> original sample)
      bool isSig = false;      ///< is not removed by quality cuts?
      double t   = realNaN;    ///< time of event [hours]
      bool is4S = false;       ///< to distinguish between 4S and off-resonance runs

    };


    std::vector<Event> getEvents(TTree* tr, bool is4S);

    /** Run the InvariantMass analysis where splitPoints are the boundaries of the short calibration intervals
      @param evts: vector of events
      @param splitPoints: the vector containing times of the edges of the short calibration intervals [hours]
      @return A tuple containing vector with invariant mass, vector with invariant mass stat. error and a spread of the invariant mass
    */
    std::tuple<std::vector<Eigen::VectorXd>, std::vector<Eigen::MatrixXd>, Eigen::MatrixXd>  runMuMuInvariantMassAnalysis(
      std::vector<Event> evts,
      const std::vector<double>& splitPoints);

  }
}
