/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <calibration/CalibrationAlgorithm.h>
#include <vxd/dataobjects/VxdID.h>
#include <map>

namespace Belle2 {
  /**
   * Class implementing PXD hot pixel masking calibration algorithm
   */
  class  PXDHotPixelMaskCalibrationAlgorithm : public CalibrationAlgorithm {
  public:

    /// Constructor set the prefix to PXDHotPixelMaskCalibrationAlgorithm
    PXDHotPixelMaskCalibrationAlgorithm();

    /// Force continue masking in almost empty runs instead of returning c_NotEnoughData
    bool forceContinueMasking;

    /// Minimum number of collected events
    int minEvents;

    /// Minimum median number of hits per pixel needed for dead pixel masking
    int minHits;

    /// The occupancy threshold for masking hot single pixels is the median occupancy x pixelMultiplier
    float pixelMultiplier;

    /// Mask hot drain lines with too high average occupancy after single pixel masking
    bool maskDrains;

    /// The occupancy threshold for masking hot drains is the median occupancy x drainMultiplier
    float drainMultiplier;

    /// Mask hot rows with too high average occupancy after single pixel masking
    bool maskRows;

    /// The occupancy threshold for masking hot rows is the median occupancy x rowMultiplier
    float rowMultiplier;

    /// Payload name for PXDDeadPixelPar used for more defective pixels from damaged gates
    std::string deadPixelPayloadName;

    /// The occupancy threshold for inefficient (or dead) pixels is the median occupancy x inefficientPixelMultiplier
    float inefficientPixelMultiplier;

    /// The minimum number of inefficient (or dead) pixels per row to define an inefficient row
    int minInefficientPixels;

    /// setter for m_debugHisto
    void setDebugHisto(bool debugHisto) {m_debugHisto = debugHisto;}

  protected:

    /// Run algo on data
    virtual EResult calibrate() override;

  private:
    /** Number of vCells of Belle II PXD sensors*/
    const unsigned short c_nVCells = 768;
    /** Number of uCells of Belle II PXD sensors*/
    const unsigned short c_nUCells = 250;
    /** Number of drain lines of Belle II PXD sensors*/
    const unsigned short c_nDrains = 1000;

    /// Set if a debugging histogram should be created in the algorithm output directory
    int m_debugHisto = false;

    /// map of VxdID to median hits of each sensor
    std::map<VxdID, double> m_medianOfHitsMap;

    /// Pointer for TFile
    std::shared_ptr<TFile> m_file;

    /// Perform debug histogram file creation
    void createDebugHistogram();

  };
} // namespace Belle2


