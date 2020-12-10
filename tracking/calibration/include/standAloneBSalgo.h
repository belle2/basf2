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

//If compiled within BASF2
#ifdef _PACKAGE_
#include <tracking/calibration/Splitter.h>
#else
#include "Splitter.h"
#endif

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
  std::tuple<std::vector<TVector3>, std::vector<TMatrixDSym>, TMatrixDSym>  runBeamSpotAnalysis(std::vector<Event> evts,
      const std::vector<double>& splitPoints);

  /** Convert splitPoints [hours] to breakPoints in ExpRunEvt
    @param events: vector of events
    @param splitPoints: the vector containing times of the edges of the calibration intervals [hours]
    @return a vector with calibration break-points in the exp-run-evt format
   */
  std::vector<ExpRunEvt> convertSplitPoints(const std::vector<Event>& events, std::vector<double> splitPoints);

  /** Get the exp-run-evt number from the event time [hours]
    @param events: vector of events
    @param tEdge: the event time of the event of interest [hours]
    @return the position of the time point in the exp-run-evt format
   */
  ExpRunEvt getPosition(const std::vector<Event>& events, double tEdge);

  /** Get the map of runs, where each run contains pair with start/end time [hours]
    @param evts: vector of events
    @return a map where the key is exp-run and value start/end time of the particular run [hours]
  */
  std::map<ExpRun, std::pair<double, double>> getRunInfo(const std::vector<Event>& evts);

}
