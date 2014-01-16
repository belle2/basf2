/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef VXD_SENSITIVEDETECTORBASE_H
#define VXD_SENSITIVEDETECTORBASE_H

#include <simulation/kernel/SensitiveDetectorBase.h>
#include <vxd/geometry/SensorInfoBase.h>

namespace Belle2 {
  /** Namespace to provide code needed by both Vertex Detectors, PXD and SVD */
  namespace VXD {
    /** Base class for Sensitive Detector implementation of PXD and SVD.
     * This base class provides common access to sensor information (like transformation)
     */
    class SensitiveDetectorBase: public Simulation::SensitiveDetectorBase {
    public:
      /** Constructor
       * @param info pointer to a valid SensorInfo instance providing
       * information about the sensor this sensitive detector instance will
       * handle. Ownership of the SensorInfo goes to the sensitive detector
       * instance
       */
      SensitiveDetectorBase(SensorInfoBase* info):
        Simulation::SensitiveDetectorBase((info->getType() == SensorInfoBase::PXD ? "PXD " : "SVD ") + (std::string)info->getID(), info->getType() == SensorInfoBase::PXD ? Const::PXD : Const::SVD), m_info(info) {};

      /** Destructor freeing the sensor Info */
      virtual ~SensitiveDetectorBase() {
        if (m_info) delete m_info;
      }
      /** Return a pointer to the SensorInfo associated with this instance */
      SensorInfoBase* getSensorInfo() { return m_info; }
    protected:
      /** Pointer to the SensorInfo associated with this instance */
      SensorInfoBase* m_info;
    };
  }
} //Belle2 namespace
#endif
