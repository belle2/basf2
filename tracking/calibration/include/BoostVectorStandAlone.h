/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <framework/geometry/B2Vector3.h>
#include <vector>
#include <tuple>
#include <Eigen/Dense>

//If compiled within BASF2
#ifdef _PACKAGE_
#include <tracking/calibration/Splitter.h>
#else
#include <Splitter.h>
#endif

namespace Belle2::BoostVectorCalib {

  static const double realNaN = std::numeric_limits<double>::quiet_NaN();
  static const B2Vector3D vecNaN(realNaN, realNaN, realNaN);

  /** track parameters (neglecting curvature) */
  struct Track {
    B2Vector3D p = vecNaN; ///< momentum vector of the track
    double pid = realNaN; ///< particle ID of mu/e separation
  };

  /** Event containing two tracks */
  struct Event {
    int exp   = -1; ///< experiment number
    int run   = -1; ///< run number
    int evtNo = -1; ///< event number


    Track mu0; ///< track in the event
    Track mu1; ///< other track in the event

    int nBootStrap = 1;     ///< random bootstrap weight (n=1 -> original sample)
    bool isSig = false;     ///< is not removed by quality cuts?
    double t = realNaN;     ///< time of event [hours]

  };


  /** Convert events from TTree to std::vector */
  std::vector<Event> getEvents(TTree* tr);


  /** get median of array v of length n */
  double median(double* v, int n);


  /** Run the BoostVector analysis with short calibration intervals defined by the splitPoints
    @param evts: vector of events
    @param splitPoints: the vector containing times of the edges of the short calibration intervals [hours]
    @return A tuple containing vector with BV-centers, vector with BV-centers stat. errors and a BV-spread covariance matrix
  */
  std::tuple<std::vector<Eigen::VectorXd>, std::vector<Eigen::MatrixXd>, Eigen::MatrixXd>  runBoostVectorAnalysis(
    std::vector<Event> evts,
    const std::vector<double>& splitPoints);

}
