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
#include <functional>
#include <TVector3.h>
#include <TTree.h>
#include <Eigen/Dense>

//If compiled within BASF2
#ifdef _PACKAGE_
#include <tracking/calibration/Splitter.h>
#else
#include <Splitter.h>
#endif

namespace Belle2::InvariantMassMuMuCalib {

  static const double realNaN = std::numeric_limits<double>::quiet_NaN();
  static const TVector3 vecNaN(realNaN, realNaN, realNaN);

  /** track parameters (neglecting curvature) */
  struct Track {
    TVector3 p = vecNaN; ///< momentum vector of the track
    double pid = realNaN; ///< particle ID of mu/e separation
  };

  /** Event containing two tracks */
  struct Event {
    int exp   = -1;  ///< experiment number
    int run   = -1;  ///< run number
    int evtNo = -1;  ///< event number


    Track mu0; ///< track in the event
    Track mu1; ///< other track in the event
    double m; ///< dimuon mass

    int nBootStrap = 1;      ///< random bootstrap weight (n=1 -> original sample)
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


  /** convolution of Gaussian with exp tails with the Gaussian smearing kernel
      the Gaus is defined by mean, sigma, the parameters defining transition between Gaus and exp tails
      bMean = (bL + bR) / 2 and bDelta = (bR - bL)/2, where bL or bR is the left or right transition point
      tauL and tauR is the decay parameter of the left and right exp tail
      The smearing Gaussian kernel is defined just by its sigma=sigmaK */
  double gausExpConv(double mean, double sigma, double bMean, double bDelta, double tauL, double tauR, double sigmaK, double x);


  /** Numerical integration based on https://en.wikipedia.org/wiki/Gauss%E2%80%93Kronrod_quadrature_formula
      Function is evaluated at 15 points chosen in a clever way, so that it corresponds to the interpolation
      by a polynomial of order 29 and calculation its area
    @param f: 1D function to integrate
    @param a: lower limit of the integral
    @param b: upper limit of the integral (a <= b)
    @return An integral of f from a to b
  */
  double integrate(std::function<double(double)> f, double a, double b);

}
