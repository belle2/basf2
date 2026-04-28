/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationAlgorithm.h>

namespace Belle2 {
  /**
   * Class implementing SVDHotStripsCalibrations calibration algorithm
   */
  class SVDHotStripsCalibrationsAlgorithm : public CalibrationAlgorithm {
  public:

    /** Constructor set the prefix to SVDHotStripsCalibrationsCollector */
    explicit SVDHotStripsCalibrationsAlgorithm(const std::string& str);

    /** Destructor */
    virtual ~SVDHotStripsCalibrationsAlgorithm() {}

    /** Setter of m_computeAverageOccupancyPerChip */
    void computeAverageOccupancyPerChip(bool value) {m_computeAverageOccupancyPerChip = value;}

    /** Setter of m_relativeOccupancyThreshold */
    void setRelativeOccupancyThreshold(float value) {m_relativeOccupancyThreshold = value;}

    /** Setter of m_absoluteOccupancyThreshold */
    void setAbsoluteOccupancyThreshold(float value) {m_absoluteOccupancyThreshold = value;}

    /** Setter of m_computeAverageOccupancyPerChip */
    bool getComputeAverageOccupancyPerChip() {return m_computeAverageOccupancyPerChip;}

    /** Setter of m_relativeOccupancyThreshold */
    float getRelativeOccupancyThreshold() {return m_relativeOccupancyThreshold;}

    /** Setter of m_relativeOccupancyThreshold */
    float getAbsoluteOccupancyThreshold() {return m_absoluteOccupancyThreshold;}

  protected:

    /** Run algo on data */
    virtual EResult calibrate() override;

  private:

    std::string m_id; /**< Parameter given to set the UniqueID of the payload*/
    bool theHSFinder(double* stripOccAfterAbsCut, bool* hsflag, int nstrips); /**< returns true if the strip is hot*/
    bool m_computeAverageOccupancyPerChip = false; /**< granularity used to estimate average occupancy.
                  True: the avg. occupancy per chip is considered.
                  False: the avg. occupancy per sensor/side is considered.*/
    float m_relativeOccupancyThreshold = 5.0; /**< occupancy relative to the average sensor occupancy used
               to define a strip as hot.*/
    float m_absoluteOccupancyThreshold = 0.2; /**< absolute occupancy thresold to define a strip as hot.*/
  };
} // namespace Belle2


