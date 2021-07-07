/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationAlgorithm.h>

#include <optional>

namespace Belle2 {
  /**
   * Class implementing SVD3SampleCoGTimeCalibration calibration algorithm
   */
  class SVD3SampleCoGTimeCalibrationAlgorithm : public CalibrationAlgorithm {
  public:

    /** Constructor set the prefix to SVDTimeCalibrationCollector */
    explicit SVD3SampleCoGTimeCalibrationAlgorithm(const std::string& str);

    /** Destructor */
    virtual ~SVD3SampleCoGTimeCalibrationAlgorithm() {}

    /** Setter for m_allowedTimeShift */
    void setAllowedTimeShift(float value) {m_allowedTimeShift = value;}

    /** Getter for m_allowedTimeShift */
    float getAllowedTimeShift() {return m_allowedTimeShift;}

    /** Set the minimum entries required in the histograms */
    void setMinEntries(int minEntries) {m_minEntries = minEntries;}

    /** Get the minimum entries required in the histograms */
    int getMinEntries() {return m_minEntries;}

  protected:

    /** Run algo on data */
    virtual EResult calibrate() override;

    /** If the event T0 changes significantly return true. This is run inside the findPayloadBoundaries member function
    in the base class. */
    virtual bool isBoundaryRequired(const Calibration::ExpRun& currentRun) override;

    /** setup the boundary finding*/
    virtual void boundaryFindingSetup(std::vector<Calibration::ExpRun> /*runs*/, int /*iteration = 0*/) override
    {
      m_previousRawTimeMeanL3V.reset();
    }


  private:

    std::string m_id; /**< Parameter given to set the UniqueID of the payload*/
    std::optional<float> m_previousRawTimeMeanL3V; /**< Raw CoG of the previous run*/
    float m_allowedTimeShift = 2.; /**< Allowed Raw CoGshift*/
    float m_minEntries = 10000; /**< Set the minimun number of entries required in the histograms of layer 3*/
  };
} // namespace Belle2
