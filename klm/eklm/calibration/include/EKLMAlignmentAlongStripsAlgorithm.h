/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* Belle 2 headers. */
#include <calibration/CalibrationAlgorithm.h>

namespace Belle2 {

  /**
   * EKLM time calibration algorithm.
   */
  class EKLMAlignmentAlongStripsAlgorithm : public CalibrationAlgorithm {

  public:

    /**
     * Event: time, distance from hit to SiPM.
     */
    struct Event {
      int section;       /**< Section number. */
      int layer;         /**< Layer number. */
      int sector;        /**< Sector number. */
      int plane;         /**< Plane number. */
      int strip;         /**< Strip number. */
      int stripGlobal;   /**< Strip global number. */
      int segmentGlobal; /**< Segment global number. */
      float x;           /**< Hit X coordinate. */
      float y;           /**< Hit Y coordinate. */
      float z;           /**< Hit Z coordinate. */
      float distSiPM;    /**< Distance from hit to the SiPM. */
      float distFarEnd;  /**< Distance from hit to the far end of the strip. */
    };

    /**
     * Constructor.
     */
    EKLMAlignmentAlongStripsAlgorithm();

    /**
     * Destructor.
     */
    ~EKLMAlignmentAlongStripsAlgorithm();

    /**
     * Calibration.
     */
    CalibrationAlgorithm::EResult calibrate() override;

    /**
     * Set output file name.
     */
    void setOutputFile(const char* outputFile);

  private:

    /**
     * Get plane number for average values (0-based).
     * @param[in] sector Sector number.
     * @param[in] plane  Plane number.
     */
    int getAveragedPlane(int sector, int plane) const;

    /** Output file name. */
    std::string m_OutputFile;

  };

}
