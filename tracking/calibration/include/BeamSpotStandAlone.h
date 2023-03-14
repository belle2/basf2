/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vector>
#include <tuple>
#include <TTree.h>
#include <Eigen/Dense>

//If compiled within BASF2
#ifdef _PACKAGE_
#include <tracking/calibration/Splitter.h>
#else
#include <Splitter.h>
#endif

namespace Belle2::BeamSpotCalib {


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



  /** Get vector of events from TTree
    @param tr: the tree from where the events are read
    @return the vector of the event structure
  */
  std::vector<Event> getEvents(TTree* tr);

  /** Run the BeamSpot analysis using the splitPoints for BS-position parameters
    @param evts: vector of events
    @param splitPoints: the vector containing times of the edges of the BS-position calibration intervals [hours]
    @return A tuple containing vector with BS-positions, vector with BS-positions stat. errors and a BS-size covarince matrix
  */
  std::tuple<std::vector<Eigen::VectorXd>, std::vector<Eigen::MatrixXd>, Eigen::MatrixXd>  runBeamSpotAnalysis(
    std::vector<Event> evts,
    const std::vector<double>& splitPoints);


}
