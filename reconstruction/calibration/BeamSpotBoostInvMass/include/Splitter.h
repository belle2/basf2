/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <map>
#include <vector>
#include <TF1.h>


//If compiled within BASF2
#ifdef _PACKAGE_
#include <framework/logging/Logger.h>
#else
#ifndef B2FATAL
#define B2FATAL(arg) { std::cout << arg << std::endl; std::exit(1);}
#define B2ASSERT(str, cond) { if((cond) == false) {std::cout << __FILE__ <<", line "<<__LINE__ << std::endl << "" << #cond << " failed" << std::endl;   std::exit(1);} }
#endif
#define B2INFO(arg) { std::cout << arg << std::endl;}
#define B2WARNING(arg) { std::cout << "WARNING : "<< arg << std::endl;}
#endif




namespace Belle2 {

  // The data are divided into many small atoms, where atom has some specific target time length (few mins)
  // but cannot (by definition) span over two runs.
  // The task of the Splitter class is to cluster atoms into the calibration intervals according to th lossFunction
  // In the next step the atoms in each calibration interval are clustered into the calibration subintervals

  /** Very small (few mins) calibration interval which cannot be further divided  : Atom */
  struct Atom {
    double t1 = 0; ///< Start time of the Atom
    double t2 = 0; ///< End time of the Atom
    int nEv = 0; ///< Number of events in Atom
  };



  /** Struct containing exp number and run number */
  struct ExpRun {
    int exp; ///< experiment number
    int run; ///< run number
    /** Simple constructor */
    ExpRun(int Exp, int Run) : exp(Exp), run(Run) {}
  };


  /** struct with expNum, runNum, evtNum */
  struct ExpRunEvt {
    int exp; ///< experiment number
    int run; ///< run number
    int evt; ///< event number
    /** simple constructor */
    ExpRunEvt(int Exp, int Run, int Evt) : exp(Exp), run(Run), evt(Evt) {}
  };



  /** Not equal for ExpRun */
  inline bool operator!=(ExpRun a, ExpRun b) { return (a.exp != b.exp || a.run != b.run); }

  /** less than for ExpRun */
  inline bool operator<(ExpRun a, ExpRun b) {return ((a.exp < b.exp) || (a.exp == b.exp && a.run < b.run));}

  /** filter events to remove runs shorter than cut, it stores removed runs in runsRemoved */
  std::map<ExpRun, std::pair<double, double>> filter(const std::map<ExpRun, std::pair<double, double>>& runs, double cut,
                                                     std::map<ExpRun, std::pair<double, double>>& runsRemoved);

  /// get the range of interval with nIntervals and breaks stored in a vector
  std::pair<int, int> getStartEndIndexes(int nIntervals,  std::vector<int> breaks, int indx);


  /// Slice the vector to contain only elements with indexes s .. e (included)
  inline std::vector<Atom> slice(std::vector<Atom> vec, int s, int e)
  {
    return std::vector<Atom>(vec.begin() + s, vec.begin() + e + 1);
  }


  /** Get calibration intervals according to the indexes of the breaks
   * @param runsMap: map defining the time ranges of the runs
   * @param currVec: vector with time intervals of the atoms (small non-divisible time intervals)
   * @param breaks: vector with integer indexes of the breaks
   * @return: a vector of the calib. intervals where each interval is a map with exp-run as a key and start- end-time as a value
   **/
  std::vector<std::map<ExpRun, std::pair<double, double>>> breaks2intervalsSep(const std::map<ExpRun,
      std::pair<double, double>>& runsMap, const std::vector<Atom>& currVec, const std::vector<int>& breaks);



  /** Class that allows to split runs into the intervals of intended properties given by the lossFunction.
   *  It typically avoids of having large time gaps within calibration intervals.
   *  The algorithm is based on minimizing the total loss function
   *  Notice, that the calibration interval is defined as a map,
   *  where for each run the included period is defined by the start and end time
   */
  class Splitter {
  public:

    Splitter() : lossFun(nullptr)  {}




    /** Get the start/end time of the calibration interval (vector of the calib. subintervals).
      * @param res: A single calibration interval (for example from getIntervals function)
      * @return: A pair with start/end time of the whole calib. interval
      **/
    static std::pair<double, double> getStartEnd(std::vector<std::map<ExpRun, std::pair<double, double>>> res)
    {
      return {res.front().begin()->second.first,
              res.back().rbegin()->second.second};
    }


    /** Get vector with breaks of the calib. interval
      * @param res: A single calibration interval
      * @return: Vector of times [hours] of the break points, i.e. the subintervals boundaries
      **/
    static std::vector<double> getBreaks(std::vector<std::map<ExpRun, std::pair<double, double>>> res)
    {
      std::vector<double> breaks;
      for (int k = 0; k < int(res.size()) - 1; ++k) {
        double e = res.at(k).rbegin()->second.second; //end of the previous interval
        double s = res.at(k + 1).begin()->second.first; //start of the next interval
        breaks.push_back((e + s) / 2.);  //the break time is in between
      }
      return breaks;
    }


    /** Merge two subintervals into one subinterval
      * @param I1: First subinterval to merge
      * @param I2: Second subinterval to merge
      * @return: The resulting subinterval
      **/
    static std::map<ExpRun, std::pair<double, double>> mergeIntervals(std::map<ExpRun, std::pair<double, double>> I1,
                                                    std::map<ExpRun, std::pair<double, double>> I2);


    /** Function to merge/divide runs into the calibration intervals of given
      * characteristic length.
      * @param runs: A map with key containing expNum+runNum and value start+end time in hours of the run
      * @param evts: A vector with all events
      * @param lossFunctionOuter: A formula of the outer loss function (for calib. intervals)
      * @param lossFunctionInner: A formula of the inner loss function (for calib. subintervals)
      * @param atomSize: lenght of the small calibration interval - atom (in hours)
      * @return: Vector of the calib. intervals always containing vector of calib. subintervals.
      *          Each subinterval is defined as a map spanning in general over several runs
      **/
    template<typename Evt>
    std::vector<std::vector<std::map<ExpRun, std::pair<double, double>>>>  getIntervals(
      const std::map<ExpRun, std::pair<double, double>>& runs,  std::vector<Evt> evts,
      TString lossFunctionOuter, TString lossFunctionInner, double atomSize = 3. / 60)
    {
      //sort events by time
      std::sort(evts.begin(), evts.end(), [](const Evt & e1, const Evt & e2) { return (e1.t > e2.t); });

      // Divide into small intervals
      std::vector<std::pair<double, double>> smallRuns = splitToSmall(runs, atomSize);

      std::vector<Atom> atoms = createAtoms(smallRuns, evts);


      // Calculate breaks for the calib. intervals
      lossFun = toTF1(lossFunctionOuter);
      std::vector<int> breaksSize = dynamicBreaks(atoms);

      std::vector<std::vector<std::map<ExpRun, std::pair<double, double>>>> splitsRun; //split intervals with runNumber info


      // Calculate breaks for the calib. subintervals
      lossFun = toTF1(lossFunctionInner);
      std::vector<int> breaksVtx;
      for (int i = 0; i < int(breaksSize.size()) + 1; ++i) { //loop over all calib. intervals
        int s, e;
        std::tie(s, e) = getStartEndIndexes(atoms.size(), breaksSize, i);

        // Store only atoms lying in the current calib. interval
        auto currVec = slice(atoms, s, e);

        // Get optimal breaks for particular calib. interval
        std::vector<int> breaksSmall = dynamicBreaks(currVec);

        // Incorporate the runNumber information
        auto splitSeg = breaks2intervalsSep(runs, currVec, breaksSmall);

        // Put the vector of subintervals (representing division of the interval)
        // into the vector
        splitsRun.push_back(splitSeg);

        if (s != 0) breaksVtx.push_back(s - 1);
        for (auto b : breaksSmall)
          breaksVtx.push_back(b + s);
        if (e != int(breaksSize.size())) breaksVtx.push_back(e);
      }

      return splitsRun;
    }


  private:

    /** Get optimal break points using algorithm based on dynamic programing
      * @param runs: Vector of atoms, where each atom is an intervals in time
      * @return: Optimal indexes of the break points
     **/
    std::vector<int> dynamicBreaks(const std::vector<Atom>&  runs);



    /** Recursive function to evaluate minimal sum of the lossFuctions for the optimal clustering.
      * It return the minimum of the lossFunction and optimal break points giving such value.
      * It acts only on atoms with indexes between 0 and e
      * @param vec: Vector of atoms, where each atom is an intervals in time
      * @param e: the index of the last atom included in the optimisation problem
      * @param[out] breaks: Output vector with indexes of optimal break points
      * @return: Minimal value of the summed loss function
     **/
    double getMinLoss(const std::vector<Atom>&  vec,   int e, std::vector<int>& breaks);


    /** lossFunction of the calibration interval consisting of several "atoms" stored in vector vec
      * The atoms included in calibration interval have indices between s and e
      * @param vec: Vector of atoms, where each atom is an intervals in time
      * @param s: First index of the calib. interval
      * @param e: Last index of the calib. interval
      * @return: A value of the loss function
      **/
    double lossFunction(const std::vector<Atom>&  vec, int s, int e) const;


    /** Split the runs into small calibration intervals (atoms) of a specified size.
      * By definition each of these intervals spans only over single run.
      * These will be clustered into larger intervals in the next steps
      * @param runs: Runs to split into the atoms
      * @param intSize: Intended size of the small intervals
      * @return: A vector with resulting time boundaries of the atoms
      **/
    static std::vector<std::pair<double, double>> splitToSmall(std::map<ExpRun, std::pair<double, double>> runs,
                                                               double intSize = 1. / 60);

    /** Get the vector with parameters of the calibration Atoms
      * @param atomsTimes: vector with start/end times of the atoms
      * @param evts: vector with events
      * @return  A vector of atoms, each including start/end times and the number of events inside
      **/
    template<typename Evt>
    inline std::vector<Atom> createAtoms(const std::vector<std::pair<double, double>>& atomsTimes,  const std::vector<Evt>& evts)
    {
      std::vector<Atom> atoms(atomsTimes.size());

      // Store start/end times to atoms
      for (unsigned a = 0; a < atoms.size(); ++a) {
        atoms[a].t1 = atomsTimes[a].first;
        atoms[a].t2 = atomsTimes[a].second;
      }


      // count the number of events in each atom
      for (unsigned i = 0; i < evts.size(); ++i) {
        for (unsigned a = 0; a < atoms.size(); ++a)
          if (atoms[a].t1 <= evts[i].t  && evts[i].t < atoms[a].t2)
            ++atoms[a].nEv;

      }



      return atoms;
    }


    /** Convert lossFunction from string to TF1 */
    TF1* toTF1(TString LossString)
    {
      LossString.ReplaceAll("rawTime", "[0]");
      LossString.ReplaceAll("netTime", "[1]");
      LossString.ReplaceAll("maxGap",  "[2]");
      LossString.ReplaceAll("nEv",     "[3]");

      return (new TF1("lossFun", LossString));
    }


    TF1*   lossFun; ///< loss function used for clustering of Atoms


    /** cache used by the clustering algorithm (has to be reset every time) */
    std::vector<std::pair<double, std::vector<int>>> cache;


  };


  /** Get the map of runs, where each run contains pair with start/end time [hours]
    @param evts: vector of events
    @return a map where the key is exp-run and value start/end time of the particular run [hours]
  */
  template<typename Evt>
  inline std::map<ExpRun, std::pair<double, double>> getRunInfo(const std::vector<Evt>& evts)
  {
    std::map<ExpRun, std::pair<double, double>> runsInfo;

    for (auto& evt : evts) {
      int Exp = evt.exp;
      int Run = evt.run;
      double time = evt.t;
      if (runsInfo.count(ExpRun(Exp, Run))) {
        double tMin, tMax;
        std::tie(tMin, tMax) = runsInfo.at(ExpRun(Exp, Run));
        tMin = std::min(tMin, time);
        tMax = std::max(tMax, time);
        runsInfo.at(ExpRun(Exp, Run)) = {tMin, tMax};
      } else {
        runsInfo[ExpRun(Exp, Run)] = {time, time};
      }

    }
    return runsInfo;
  }


  /** Get the exp-run-evt number from the event time [hours]
    @param events: vector of events
    @param tEdge: the event time of the event of interest [hours]
    @return the position of the time point in the exp-run-evt format
   */
  template<typename Evt>
  inline ExpRunEvt getPosition(const std::vector<Evt>& events, double tEdge)
  {
    ExpRunEvt evt(-1, -1, -1);
    double tBreak = -1e10;
    for (auto& e : events) {
      if (e.t < tEdge) {
        if (e.t > tBreak) {
          tBreak = e.t;
          evt =  ExpRunEvt(e.exp, e.run, e.evtNo);
        }
      }
    }
    return evt;
  }

// convert splitPoints [in UTC time] to expRunEvt
  /** Convert splitPoints [hours] to breakPoints in ExpRunEvt
    @param events: vector of events
    @param splitPoints: the vector containing times of the edges of the calibration intervals [hours]
    @return a vector with calibration break-points in the exp-run-evt format
   */
  template<typename Evt>
  std::vector<ExpRunEvt> convertSplitPoints(const std::vector<Evt>& events, std::vector<double> splitPoints)
  {

    std::vector<ExpRunEvt>  breakPos;
    for (auto p : splitPoints) {
      auto pos = getPosition(events, p);
      breakPos.push_back(pos);
    }
    return breakPos;
  }


}
