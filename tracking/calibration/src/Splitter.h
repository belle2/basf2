/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Radek Zlebcik
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <map>
#include <vector>
#include <utility>

namespace Belle2 {

// Struct containing exp number and run number
  struct ExpRun {
    int exp, run;
    ExpRun(int Exp, int Run) : exp(Exp), run(Run) {}
  };

  /** Not equal for ExpRun */
  inline bool operator!=(ExpRun a, ExpRun b) { return (a.exp != b.exp || a.run != b.run); }

  /** less than for ExpRun */
  inline bool operator<(ExpRun a, ExpRun b) {return (1e6 * a.exp + a.run) < (1e6 * b.exp + b.run);}

  /** filter events to remove runs shorter than cut, it stores removed runs in runsRemoved */
  std::map<ExpRun, std::pair<double, double>> filter(const std::map<ExpRun, std::pair<double, double>>& runs, double cut,
                                                     std::map<ExpRun, std::pair<double, double>>& runsRemoved);


  /** Class that allows to split runs into the intervals of intended lenght
   *  in the same time it avoids of having large time gaps within calibration intervals
   *  The algorithm is based on minimized total loss function
   *  Notice, that the calibration interval is defined as a map,
   *  where for each run the included period is defined by the start and end time
   */
  class Splitter {
  public:


    /** Function to merge/divide runs into the calibration intervals of given
      * characteristic length.
      * @param runs: A map with key containing expNum+runNum and value start+end time in hours of the run
      * @param tBestSize: An intended length of the calibration intervals for BS-size parameters [hours]
      * @param tBestVtx: An intended length of the calibration intervals PS-position parameters [hours]
      * @output: Vector of BS-size calib. intervals always containing vector of BS-position calib. intervals.
      *          The BS-position calib. interval is a map spanning in general over several runs
      **/
    std::vector<std::vector<std::map<ExpRun, std::pair<double, double>>>>  getIntervals(
      const std::map<ExpRun, std::pair<double, double>>& runs,
      double tBestSize, double tBestVtx, double GapPenalty);

    /** Get the start/end time of the BS-size calibration interval (vector of BS-position calib intervals).
      * @param res: The BS-size calibration interval (for example from getIntervals function)
      * @output: Vector of size-calibration intervals always containing vector of position calib. intervals.
      **/
    static std::pair<double, double> getStartEnd(std::vector<std::map<ExpRun, std::pair<double, double>>> res)
    {
      return {res.front().begin()->second.first,
              res.back().rbegin()->second.second};
    }

    /** Get vector with breaks of the BS-size calib. interval
      * @param res: The BS-size calibration interval
      * @output: Vector of times [hours] of the break points, where BS-position calibration changes
      **/
    static std::vector<double> getBreaks(std::vector<std::map<ExpRun, std::pair<double, double>>> res)
    {
      std::vector<double> breaks;
      for (int k = 0; k < int(res.size()) - 1; ++k) {
        double e = res.at(k).rbegin()->second.second; //end of the previous interval
        double s = res.at(k + 1).begin()->second.first; //start of the next interval
        breaks.push_back((e + s) / 2.);  //in break time is in between
      }
      return breaks;
    }

    /** Merge two BS-position intervals into one
      * @param I1: First interval to merge
      * @param I2: Second interval to merge
      * @output: The resulting interval
      **/
    static std::map<ExpRun, std::pair<double, double>> mergeIntervals(std::map<ExpRun, std::pair<double, double>> I1,
                                                    std::map<ExpRun, std::pair<double, double>> I2);


  private:

    /** Split the runs into small calibration intervals (atoms) of a specified size.
      * But definition each of these intervals spans only over single run.
      * These will be clustered into larger intervals in the next steps
      * @param runs: Runs to split into the atoms
      * @param intSize: Intended size of the small intervals
      * @output: A vector with resulting time boundaries of the atoms
      **/
    static std::vector<std::pair<double, double>> splitToSmall(std::map<ExpRun, std::pair<double, double>> runs,
                                                               double intSize = 1. / 60);


    /** Get optimal break points using algorithm based on dynamic programing
      * @param runs: Vector of atoms, where each atom is an intervals in time
      * @output: Optimal indexes of the break points
     **/
    std::vector<int> dynamicBreaks(std::vector<std::pair<double, double>>  runs);



    /** Recursive function to evaluate minimal sum of the lossFuctions for the optimal clustering.
      * It return the minimum of the lossFunction and optimal break points giving such value.
      * It acts only on atoms with indexes between 0 and e
      * @param vec: Vector of atoms, where each atom is an intervals in time
      * @param e: the index of the last atom included in the optimisation problem
      * @param[out] breaks: Output vector with indexes of optimal break points
      * @output: Minimal value of the summed loss function
     **/
    double getMinLoss(const std::vector<std::pair<double, double>>&  vec,   int e, std::vector<int>& breaks);


    /** lossFunction of the calibration interval consisting of several "atoms" stored in vector vec
      * The atoms included in calibration interval have indices between s and e
      * @param vec: Vector of atoms, where each atom is an intervals in time
      * @param s: First index of the calib. interval
      * @param e: Last index of the calib. interval
      * @output: A value of the loss function
      **/
    double lossFunction(const std::vector<std::pair<double, double>>&  vec, int s, int e);




    double tBest;      /** target calib. interval time in hours */
    double gapPenalty; /** in relative units (higher -> less gaps) */

    std::vector<std::pair<double, std::vector<int>>> cache; /** cache used by the clustering algorithm */


  };

}
