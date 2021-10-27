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
#include <Splitter.h>
#endif

namespace Belle2 {
  namespace InvariantMassBhadCalib {

    static const double realNaN = std::numeric_limits<double>::quiet_NaN();
    static const int intNaN     = std::numeric_limits<int>::quiet_NaN();
    static const TVector3 vecNaN(realNaN, realNaN, realNaN);



    struct Event {

      int exp   = intNaN;
      int run   = intNaN;
      int evtNo = intNaN;

      double mBC = realNaN;
      double deltaE = realNaN;
      int pdg = intNaN;
      int mode = intNaN;
      double Kpid = realNaN;
      double R2 = realNaN;
      double mD = realNaN;
      double dmDstar = realNaN;
      //double cmsE0; // eCMS used to calculate mBC and deltaE

      double t = realNaN;   // time of event

      bool isSig = false;
      int nBootStrap = intNaN;
    };






    std::vector<Event> getEvents(TTree* tr);


    std::vector<std::vector<double>> doBhadFit(const std::vector<Event>& evts, std::vector<std::pair<double, double>> limits,
                                               std::vector<std::pair<double, double>> mumuVals);



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
