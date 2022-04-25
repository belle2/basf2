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
#include <TVector3.h>
#include <Eigen/Dense>

//If compiled within BASF2
#ifdef _PACKAGE_
#include <tracking/calibration/Splitter.h>
#else
#include <Splitter.h>
#endif

namespace Belle2::InvariantMassBhadCalib {

  static const double realNaN = std::numeric_limits<double>::quiet_NaN();
  static const TVector3 vecNaN(realNaN, realNaN, realNaN);


  /** structure containing variables relevant to the B-candidate */
  struct Candidate {
    double pBcms = realNaN;   ///< B meson momentum in CMS
    double mB    = realNaN;   ///< mass of B meson
    int pdg = 0;              ///< PDG code of the signal B-meson, 0 for dummy value
    int mode = -1;            ///< integer code identifying the decay channel, -1 is dummy value
    double Kpid = realNaN;    ///< Kaon PID
    double R2 = realNaN;      ///< R2 continuum suppression variable
    double mD = realNaN;      ///< reconstructed mass of the D meson
    double dmDstar = realNaN; ///< reconstructed mass difference between Dstar and D

    bool isSig = false;       ///< isSignal flag (for applying selections)
  };



  /** structure containing variables relevant for the hadronic B decays */
  struct Event {

    int exp   = -1;       ///< experiment number
    int run   = -1;       ///< run number
    int evtNo = -1;       ///< event number
    double t = realNaN;   ///< time of the event


    std::vector<Candidate>  cand = {}; ///< vector of B meson candidates


    int nBootStrap = 1;  ///< bootstap weight, 1 -> data without bootstrapping
  };





  /** load events from TTree to the vector of Events */
  std::vector<Event> getEvents(TTree* tr);


  /** The combined Ecms fit, where the constraints from the mumu fit are defined by limits and mumuVals */
  std::vector<std::vector<double>> doBhadFit(const std::vector<Event>& evts, std::vector<std::pair<double, double>> limits,
                                             std::vector<std::pair<double, double>> mumuVals,  const std::vector<double>& startPars);

  /** The fit based only on the hadronic B decays */
  std::vector<double> doBhadOnlyFit(const std::vector<Event>& evts, const std::vector<std::pair<double, double>>& limits);


  /** Run the InvariantMass analysis where splitPoints are the boundaries of the short calibration intervals
    @param evts: vector of events
    @param splitPoints: the vector containing times of the edges of the short calibration intervals [hours]
    @return A tuple containing vector with invariant mass, vector with invariant mass stat. error and a spread of the invariant mass
  */
  std::tuple<std::vector<Eigen::VectorXd>, std::vector<Eigen::MatrixXd>, Eigen::MatrixXd>  runInvariantMassAnalysis(
    std::vector<Event> evts,
    const std::vector<double>& splitPoints);

}
