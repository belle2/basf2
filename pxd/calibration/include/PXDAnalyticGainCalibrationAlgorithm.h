/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Qingyuan Liu                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <calibration/CalibrationAlgorithm.h>
#include <vxd/dataobjects/VxdID.h>
#include <vector>

// ROOT
#include <TH1.h>


namespace Belle2 {
  /**
   * Class implementing the PXD gain calibration algorithm
   */
  class  PXDAnalyticGainCalibrationAlgorithm : public CalibrationAlgorithm {
  public:

    /// Constructor set the prefix to PXDAnalyticGainCalibrationAlgorithm
    PXDAnalyticGainCalibrationAlgorithm();

    /// Destructor
    virtual ~PXDAnalyticGainCalibrationAlgorithm() {}

    /// Minimum number of collected clusters for estimating gains
    int minClusters;

    /// Safety factor for determining whether the collected number of clusters is enough
    float safetyFactor;

    /// Force continue in low statistics runs instead of returning c_NotEnoughData
    bool forceContinue;

    /// strategy to used for gain calibration, 0 for medians, 1 for landau fit
    int strategy;

    /// Flag to use histogram of charge ratio (relative to expected MPV)
    bool useChargeRatioHistogram;

    /// Setter for boundaries
    void setBoundaries(const std::vector<Calibration::ExpRun>& boundaries) {m_requestedBoundaries = boundaries;}

    /// getter for current boundaries
    std::vector<Calibration::ExpRun> getBoundaries() const {return m_requestedBoundaries;}

  protected:

    /// Run algo on data
    virtual EResult calibrate() override;

    /// Decide if a run should be a payload boundary. Only used in certain Python Algorithm Starategies.
    virtual bool isBoundaryRequired(const Calibration::ExpRun& /*currentRun*/) override;

    /// What we do before isBoundaryRequired gets called.
    //  We reset the temporary boundaries list to be the same as the requested one.
    virtual void boundaryFindingSetup(std::vector<Calibration::ExpRun> /*runs*/, int /*iteration = 0*/) override
    {
      m_boundaries = m_requestedBoundaries;
    };

  private:

    /// Estimate gain as ratio of medians from MC and data for a part of PXD
    double EstimateGain(VxdID sensorID, unsigned short uBin, unsigned short vBin, TH1* hist = nullptr);

    ///// Calculate a median from unsorted signal vector. The input vector gets sorted.
    //double CalculateMedian(std::vector<double>& signals);

    ///// Calculate a median from a histogram.
    //double CalculateMedian(TH1* hist);

    ///// Calculate MPV from signal vector using a landau fit.
    //double FitLandau(std::vector<double>& signals);

    ///// Calculate MPV from a histogram.
    //double FitLandau(TH1* hist);

    /// Retrive current gain value from pulled in data base payload
    double GetCurrentGainFromDB(VxdID sensorID, unsigned short uBin, unsigned short vBin);

    /// During findPayloadBoundaries and isBoundaryRequired this is used to define the boundaries we want.
    std::vector<Calibration::ExpRun> m_requestedBoundaries;

  };
} // namespace Belle2


