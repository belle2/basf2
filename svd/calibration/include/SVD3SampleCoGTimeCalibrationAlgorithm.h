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

    void setTwoLineSelectionParameters(bool linearCutsOnCoG3, float interceptUpperLine, float angularCoefficientUpperLine,
                                       float interceptLowerLine,
                                       float angularCoefficientLowerLine)
    {
      m_applyLinearCutsToRemoveBkg = linearCutsOnCoG3;
      m_interceptUpperLine =  interceptUpperLine;
      m_angularCoefficientUpperLine =  angularCoefficientUpperLine;
      m_interceptLowerLine =  interceptLowerLine;
      m_angularCoefficientLowerLine =  angularCoefficientLowerLine;
    }

    std::vector<float> getTwoLineSelectionParameters()
    {
      std::vector<float> twoLineSelectionParameters;
      twoLineSelectionParameters.push_back(m_interceptUpperLine);
      twoLineSelectionParameters.push_back(m_angularCoefficientUpperLine);
      twoLineSelectionParameters.push_back(m_interceptLowerLine);
      twoLineSelectionParameters.push_back(m_angularCoefficientLowerLine);

      return twoLineSelectionParameters;
    }

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

    // straight lines used to reject off-time clusters from 2D-distribution
    bool m_applyLinearCutsToRemoveBkg = false; /**< if true turns on the selection to remove background clusters from CoG3 calibration*/
    float m_interceptUpperLine =
      -94.0; /**< Intercept of one of the two lines that define the signal region used in the CoG3 calibration (to remove background clusters) */
    float m_angularCoefficientUpperLine =
      1.264; /**< Angular coefficient of one of the two lines that define the signal region used in the CoG3 calibration (to remove background clusters) */
    float m_interceptLowerLine =
      -134.0; /**< Intercept of one of the two lines that define the signal region used in the CoG3 calibration (to remove background clusters) */
    float m_angularCoefficientLowerLine =
      1.264; /**< Angulat coefficienct of one of the two lines that define the signal region used in the CoG3 calibration (to remove bakground clusters) */
  };
} // namespace Belle2
