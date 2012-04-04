/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka, Martin Ritter                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef VXD_SENSORPLANE_H
#define VXD_SENSORPLANE_H

#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/SensorInfoBase.h>
#ifndef __CINT__
#include <vxd/geometry/GeoCache.h>
#endif

#include <GFAbsFinitePlane.h>
#include <cmath>

namespace Belle2 {
  namespace VXD {
    /**
     * A Finite plane of one VXD Sensor.
     * This class takes the SensorID of the sensor and gets the dimensions from the SensorInfo of that Sensor.
     */
    class SensorPlane: public GFAbsFinitePlane {
    public:
      /**
       * Constructs the plane for a given VXD Sensor
       * @param sensorID SensorID of the Sensor for which this plane should be build
       */
      SensorPlane(VxdID sensorID = 0, double uTolerance = 0, double vTolerance = 0):
        m_sensorID(sensorID), m_uTolerance(uTolerance), m_vTolerance(vTolerance), m_sensorInfo(0) {}

      /** Destructor. */
      virtual ~SensorPlane() {}

      /** Return wether the given coordinates are inside the finite region.
       * @param u u-coordinate of the point.
       * @param v v-coordinate of the point.
       * @return true if (u,v) is within the sensor plane, otherwise false.
       */
      bool inActive(const double& u, const double& v) const {
#ifndef __CINT__
        if (!m_sensorInfo) {
          m_sensorInfo = &VXD::GeoCache::get(m_sensorID);
        }
#endif
        return m_sensorInfo->inside(u, v, m_uTolerance, m_vTolerance);
      }

      /** Prints object data. */
      void Print(const Option_t* option = "") const;

      /**
       * Deep copy of the object.
       * @return Pointer to a deep copy of the object.
       */
      virtual GFAbsFinitePlane* clone() const {
        return new SensorPlane(*this);
      }

    private:
      /** Sensor ID of the the sensor plane */
      unsigned short m_sensorID;
      /** Tolerance to add to the sensor dimensions in u direction */
      double m_uTolerance;
      /** Tolerance to add to the sensor dimensions in v direction */
      double m_vTolerance;
      /** Pointer to the SensorInfo which contains the geometry information for the given sensor plane */
      mutable const SensorInfoBase* m_sensorInfo; //! transient member

      ClassDef(SensorPlane, 1)
    };
  } // vxd namespace
} // Belle2 namespace

#endif /* VXD_SENSORPLANE_H */
