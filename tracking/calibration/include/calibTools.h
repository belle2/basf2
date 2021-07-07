/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/



#pragma once

#include <tracking/calibration/Splitter.h>
#include <TMatrixDSym.h>
#include <TVector3.h>
#include <functional>
#include <tuple>
#include <map>

#include <framework/database/EventDependency.h>
#include <framework/datastore/StoreArray.h>
#include <calibration/CalibrationAlgorithm.h>

#include <Eigen/Dense>

namespace Belle2 {

  // General functions to perform the calibration
  // Notice that the goal of the calibration is to estimate the parameters
  // of the Gaussian distribution: center + covariance matrix describing the spread.
  // In general it requires more data to determine the spread, so there can be
  // several calib. subintervals with different values of
  // the center position of the Gaussian (mean) but with identical spread parameters.
  // The longer  intervals of constant spread are called "intervals"
  // The shorter intervals of constant mean value are called "subintervals"
  // In general there are several subintervals withing single interval
  // By definition a subinterval always belongs only to single interval.


  /** Function that converts Eigen symmetric matrix to ROOT matrix */
  inline TMatrixDSym toTMatrixDSym(Eigen::MatrixXd mIn)
  {
    TMatrixDSym mOut(mIn.rows());
    for (int i = 0; i < mIn.rows(); ++i)
      for (int j = 0; j < mIn.cols(); ++j)
        mOut(i, j) = (mIn(i, j) + mIn(j, i)) / 2.;
    return mOut;
  }

  /** Function that converts Eigen vector to ROOT vector */
  inline TVector3 toTVector3(Eigen::VectorXd vIn)
  {
    return TVector3(vIn(0), vIn(1), vIn(2));
  }

  /** get id of the time point t */
  inline int getID(const std::vector<double>& breaks, double t)
  {
    for (int i = 0; i < int(breaks.size()) + 1; ++i) {
      double s = (i == 0)             ? 0 : breaks[i - 1];
      double e = (i == int(breaks.size())) ? 1e20 : breaks[i];
      if (s  <= t   &&  t < e)
        return i;
    }
    return -1;
  }

  /** The parameters related to single calibration interval */
  struct calibPars {
    std::vector<Eigen::VectorXd> cnt; ///< vector of means for each calib. subinterval
    std::vector<Eigen::MatrixXd> cntUnc; ///< vector of uncertainties of means for each calib. subinterval
    Eigen::MatrixXd  spreadMat; ///< spread CovMatrix
    int size() const {return cnt.size();} ///< number of the subintervals
  };


  /** Parameters and data relevant for single calibration interval */
  struct CalibrationData {
    /** vector of the start and end times of the calibration subintervals */
    std::vector<std::map<ExpRun, std::pair<double, double>>> subIntervals;

    std::vector<ExpRunEvt> breakPoints; ///< vector with break points positions

    calibPars pars; ///< The parameters of the calibration itself

    bool isCalibrated = false; ///< true if calibration run was successful

  };



  /** Extrapolate calibration to intervals where it failed */
  inline void extrapolateCalibration(std::vector<CalibrationData>&  calVec)
  {
    //put closest neighbor, where the statistic was low or algo failed
    for (unsigned i = 0; i < calVec.size(); ++i) {
      if (calVec[i].pars.cnt.size() != 0) continue;
      const auto& r =  calVec[i].subIntervals;
      double Start, End;
      std::tie(Start, End) = Splitter::getStartEnd(r);

      Eigen::Vector3d ipNow;
      Eigen::MatrixXd ipeNow;
      Eigen::MatrixXd sizeMatNow;

      double distMin = 1e20;
      //Find the closest calibrated interval
      for (unsigned j = 0; j < calVec.size(); ++j) {
        if (calVec[j].isCalibrated == false) continue; //skip not-calibrated intervals
        const auto& rJ = calVec[j].subIntervals;
        for (unsigned jj = 0; jj < rJ.size(); ++jj) { //loop over subintervals
          const auto& rNow = rJ[jj];
          double s = rNow.begin()->second.first;
          double e = rNow.rbegin()->second.second;

          double dist1 = (s - End >= 0) ? (s - End) : 1e20;
          double dist2 = (Start - e >= 0) ? (Start - e) : 1e20;
          double dist = std::min(dist1, dist2);

          if (dist < distMin) {
            ipNow = calVec[j].pars.cnt.at(jj);
            ipeNow = calVec[j].pars.cntUnc.at(jj);
            sizeMatNow = calVec[j].pars.spreadMat;
            distMin = dist;
          }
        }
      }

      //Store it to vectors
      calVec[i].pars.cnt.resize(r.size());
      calVec[i].pars.cntUnc.resize(r.size());
      for (unsigned ii = 0; ii < r.size(); ++ii) {
        calVec[i].pars.cnt.at(ii) = ipNow;
        calVec[i].pars.cntUnc.at(ii) = ipeNow;
      }
      calVec[i].pars.spreadMat = sizeMatNow;
    }

  }

  /** Extrapolate calibration to the very short runs which were filtered before */
  inline void addShortRun(std::vector<CalibrationData>& calVec,  std::pair<ExpRun, std::pair<double, double>> shortRun)
  {
    double shortStart = shortRun.second.first;
    double shortEnd   = shortRun.second.second;

    double distMin = 1e20;
    int iMin = -1, jMin = -1;

    for (unsigned i = 0; i < calVec.size(); ++i) {
      if (calVec[i].isCalibrated == false)
        continue;
      for (unsigned j = 0; j < calVec[i].subIntervals.size(); ++j) {
        for (auto I : calVec[i].subIntervals[j]) {
          double s = I.second.first;
          double e = I.second.second;

          double dist1 = (s - shortEnd >= 0)   ? (s - shortEnd) : 1e20;
          double dist2 = (shortStart - e >= 0) ? (shortStart - e) : 1e20;
          double dist = std::min(dist1, dist2);

          if (dist < distMin) {
            distMin = dist;
            iMin = i;
            jMin = j;
          }
        }
      }
    }

    B2ASSERT("Must be found", iMin != -1 && jMin != -1);
    calVec[iMin].subIntervals[jMin].insert(shortRun);
  }

  /** Encode integer num into double val such that val is nearly not changed (maximally by a relative shift 1e-6).
    * It is use to store time information to the payloads   */
  inline double encodeNumber(double val, unsigned num)
  {
    double factor = pow(FLT_RADIX, DBL_MANT_DIG);
    static const long long fEnc   = pow(2, 32); //32 binary digits  for encoded number

    int e; //exponent of the number
    double mantisa = std::frexp(val, &e);
    long long mantisaI = mantisa * factor; //mantissa as integer

    if (val != 0)
      mantisaI = (mantisaI / fEnc) * fEnc  + num; //adding encoded number to last digits of mantissa
    else {
      mantisaI = factor / 2 + num;
      e = -100; //if the val is zero, ensure very small number by the exponent
    }

    double newVal = ldexp(mantisaI / factor, e);

    return newVal;
  }

  /** Decode the integer number encoded in val */
  inline unsigned decodeNumber(double val)
  {
    double factor = pow(FLT_RADIX, DBL_MANT_DIG);
    static const long long fEnc   = pow(2, 32); //32 binary digits  for encoded number

    int e;
    double mantisa = std::frexp(val, &e);
    long long mantisaI = mantisa * factor;

    return (mantisaI % fEnc);
  }




  /** Store payloads to files */
  template<typename Evt>
  inline void storePayloads(const std::vector<Evt>& evts, std::vector<CalibrationData>&  calVec, std::string objName,
                            std::function<TObject*(Eigen::VectorXd, Eigen::MatrixXd, Eigen::MatrixXd)  > getCalibObj)
  {

    // Loop to store payloads
    ExpRun exprunLast(-1, -1); //last exprun
    EventDependency* intraRun = nullptr;

    // Loop over calibration intervals
    for (unsigned i = 0; i < calVec.size(); ++i) {
      const auto& r = calVec[i].subIntervals; //   splits[i];
      // Loop over calibration subintervals
      for (int k = 0; k < int(r.size()); ++k) {

        for (auto I : r[k]) { //interval required to be within single run
          ExpRun exprun = I.first;

          //Encode Start+End time in seconds of the payload
          if (calVec[i].pars.cntUnc.at(k).rows() == 3) {
            calVec[i].pars.cntUnc.at(k)(0, 1) = calVec[i].pars.cntUnc.at(k)(1, 0) = encodeNumber(calVec[i].pars.cntUnc.at(k)(0, 1),
                                                round(I.second.first  * 3600));
            calVec[i].pars.cntUnc.at(k)(0, 2) = calVec[i].pars.cntUnc.at(k)(2, 0) = encodeNumber(calVec[i].pars.cntUnc.at(k)(0, 2),
                                                round(I.second.second * 3600));
          } else {
            calVec[i].pars.cntUnc.at(k)(0, 0)    = encodeNumber(calVec[i].pars.cntUnc.at(k)(0, 0),    round(I.second.first  * 3600));
            calVec[i].pars.spreadMat(0, 0) = encodeNumber(calVec[i].pars.spreadMat(0, 0), round(I.second.first  * 3600));
          }

          TObject* obj = getCalibObj(calVec[i].pars.cnt.at(k), calVec[i].pars.cntUnc.at(k), calVec[i].pars.spreadMat);
          if (exprun != exprunLast) { //if new run
            if (intraRun) { //if not first -> store
              auto m_iov = IntervalOfValidity(exprunLast.exp, exprunLast.run, exprunLast.exp, exprunLast.run);
              Database::Instance().storeData(objName, intraRun, m_iov);
            }

            intraRun = new EventDependency(obj);
          } else {
            int breakPoint;
            if (k - 1 >= 0) {
              breakPoint = calVec[i].breakPoints.at(k - 1).evt;
              B2ASSERT("Payload saving consistency", calVec[i].breakPoints.at(k - 1).run == exprun.run);
            } else {
              B2ASSERT("Payload saving consistency", i != 0);
              double rStart, rEnd;
              std::tie(rStart, rEnd) = Splitter::getStartEnd(r);
              auto pos = getPosition(evts, rStart);
              breakPoint = pos.evt;
              B2ASSERT("Payload saving consistency", pos.run == exprun.run);
            }
            intraRun->add(breakPoint, obj);
          }
          exprunLast = exprun;
        }
      } //end loop over calibration subintervals

    } //end loop over calibration intervals

    //Store the last entry
    auto m_iov = IntervalOfValidity(exprunLast.exp, exprunLast.run, exprunLast.exp, exprunLast.run);
    Database::Instance().storeData(objName, intraRun, m_iov);
  }

  /** run calibration algorithm for single calibration interval */
  template<typename Evt, typename Fun>
  inline CalibrationData runAlgorithm(const std::vector<Evt>& evts, std::vector<std::map<ExpRun, std::pair<double, double>>> range,
                                      Fun runCalibAnalysis
                                     )
  {
    CalibrationData calD;
    auto& r = range;
    double rStart, rEnd;
    std::tie(rStart, rEnd) = Splitter::getStartEnd(r);
    B2INFO("Start of loop startTime endTime : " << rStart << " " << rEnd);

    auto breaks =  Splitter::getBreaks(r);

    std::vector<Evt> evtsNow;

    std::vector<int> Counts(breaks.size() + 1, 0);
    // Select events belonging to the interval
    for (const auto& ev : evts) {
      if (rStart <= ev.t && ev.t < rEnd) {
        evtsNow.push_back(ev);
        ++Counts.at(getID(breaks, ev.t));
      }
    }

    B2ASSERT("Number of intervals vs number of breakPoints", r.size()  == breaks.size() + 1);

    //Merge smallest interval if with low stat (try it 10times)
    for (int k = 0; k < 10; ++k)  {
      int iMin = min_element(Counts.begin(), Counts.end()) - Counts.begin();
      if (Counts.size() >= 2 &&  Counts[iMin] < 50) { //merge with neighbor if possible
        auto iM = -1;
        if (iMin == 0)
          iM = iMin + 1;
        else if (iMin == int(Counts.size()) - 1)
          iM = iMin - 1;
        else {
          if (Counts[iMin + 1] < Counts[iMin - 1])
            iM = iMin + 1;
          else
            iM = iMin - 1;
        }
        B2ASSERT("Number of intervals equal to size of counters", r.size() == Counts.size());

        r.at(iM) = Splitter::mergeIntervals(r[iM], r[iMin]);
        r.erase(r.begin() + iMin);
        breaks =  Splitter::getBreaks(r);
        Counts[iM] += Counts[iMin];
        Counts.erase(Counts.begin() + iMin);
      }
    }

    B2INFO("#events "  << " : " << evtsNow.size());
    B2INFO("Breaks size " << " : " << breaks.size());

    calD.breakPoints = convertSplitPoints(evtsNow, breaks);

    calD.subIntervals = r;

    if (breaks.size() > 0)
      B2INFO("StartOfCalibInterval (run,evtNo,vtxIntervalsSize) " <<   calD.breakPoints.at(0).run << " " <<
             calD.breakPoints.at(0).evt << " " << calD.breakPoints.size());


    //If too few events, let have the output empty
    //Will be filled with the closest neighbor at the next stage
    if (evtsNow.size() < 50) {
      return calD;
    }

    // Run the calibration
    B2INFO("Start of running calibration over calibration interval");
    tie(calD.pars.cnt, calD.pars.cntUnc, calD.pars.spreadMat) = runCalibAnalysis(evtsNow, breaks);
    B2INFO("End of running analysis - SpreadMatX : " << sqrt(abs(calD.pars.spreadMat(0, 0))));
    B2ASSERT("All subintervals have calibration of the mean value", calD.pars.cnt.size() == r.size());
    B2ASSERT("All subintervals have calibration of the unc. of mean", calD.pars.cntUnc.size() == r.size());

    calD.isCalibrated = true;

    return calD;
  }


  /** Run the the calibration over the whole event sample
    @param tracks: TTree object with mu-mu events
    @param calibName: name of the calibration payload
    @param GetEvents: function that transforms TTree to std::vector
    @param calibAnalysis: function that performs the calibration on a single calibration interval
    @param calibObjCreator: function that stores results to the payload class which inherits from TObject
    @param m_lossFunctionOuter: Lost function for the calibration intervals of the spread parameters
    @param m_lossFunctionInner: Lost function for the calibration subintervals (for the mean value parameters)
    @return State of the calibration run, i.e. EResult::c_OK if everything OK
  */
  template<typename Fun1, typename Fun2>
  CalibrationAlgorithm::EResult runCalibration(TTree* tracks,  const std::string& calibName,  Fun1 GetEvents,  Fun2 calibAnalysis,
                                               std::function<TObject*(Eigen::VectorXd, Eigen::MatrixXd, Eigen::MatrixXd)> calibObjCreator,
                                               TString m_lossFunctionOuter, TString m_lossFunctionInner)
  {
    // Check that there are at least some data
    if (!tracks || tracks->GetEntries() < 15) {
      if (tracks)
        B2WARNING("Too few data : " << tracks->GetEntries());
      return CalibrationAlgorithm::EResult::c_NotEnoughData;
    }
    B2INFO("Number of tracks: " << tracks->GetEntries());

    // Tree to vector of Events
    auto evts = GetEvents(tracks);

    //Time range for each ExpRun
    std::map<ExpRun, std::pair<double, double>> runsInfoOrg = getRunInfo(evts);
    std::map<ExpRun, std::pair<double, double>> runsRemoved; //map with time intervals of very short runs
    auto runsInfo = filter(runsInfoOrg, 2. / 60, runsRemoved); //include only runs longer than 2mins

    // If nothing remains
    if (runsInfo.size() == 0) {
      B2WARNING("Too short run");
      return  CalibrationAlgorithm::EResult::c_NotEnoughData;
    }

    // Get intervals based on the input loss functions
    Splitter splt;
    auto splits = splt.getIntervals(runsInfo, evts, m_lossFunctionOuter, m_lossFunctionInner);

    //Loop over all calibration intervals
    std::vector<CalibrationData> calVec;
    for (auto s : splits) {
      CalibrationData calD = runAlgorithm(evts, s, calibAnalysis); // run the calibration over the interval s
      calVec.push_back(calD);
    }

    // extrapolate results to the low-stat intervals
    extrapolateCalibration(calVec);

    // Include removed short runs
    for (auto shortRun : runsRemoved) {
      addShortRun(calVec,  shortRun);
    }

    // Store Payloads to files
    storePayloads(evts, calVec, calibName, calibObjCreator);

    return CalibrationAlgorithm::EResult::c_OK;
  }


}
