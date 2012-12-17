/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/**
 * @file
 * Two enumerations (separate file to avoid header circular dependency).
 */

#ifndef GEOEKLMTYPES_H
#define GEOEKLMTYPES_H

namespace Belle2 {

  namespace EKLM {

    /** Type of sensitive detector corresponding to this volume. */
    enum EKLMSensitiveType {
      /** Strip. */
      EKLM_SENSITIVE_STRIP = 0,
      /** SiPM. */
      EKLM_SENSITIVE_SIPM = 1,
      /** Readout boards. */
      EKLM_SENSITIVE_BOARD = 2,
    };

    /** Mode of detector operation. */
    enum EKLMDetectorMode {
      /** Normal mode. */
      EKLM_DETECTOR_NORMAL = 0,
      /** Background study. */
      EKLM_DETECTOR_BACKGROUND = 1,
      /**
       * Debug mode.
       * When constructing geometry, print masses of all volumes and exit.
       */
      EKLM_DETECTOR_PRINTMASSES = 2,
    };

  }

}

#endif

