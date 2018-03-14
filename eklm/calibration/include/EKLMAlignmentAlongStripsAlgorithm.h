/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMALIGNMENTALONGSTRIPSALGORITHM_H
#define EKLMALIGNMENTALONGSTRIPSALGORITHM_H

/* Belle2 headers. */
#include <calibration/CalibrationAlgorithm.h>
#include <eklm/geometry/GeometryData.h>

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
      int endcap;        /**< Endcap number. */
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
    CalibrationAlgorithm::EResult calibrate();

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

#endif

