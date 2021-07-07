/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef VXD_SENSORPLANE_H
#define VXD_SENSORPLANE_H

#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <framework/logging/Logger.h>
//ROOT CINT has problems with the boost classes used by the GeoCache but it
//does not need to see them anyway
#ifndef __CINT__
#include <vxd/geometry/GeoCache.h>
#endif

#include <genfit/AbsFinitePlane.h>
#include <cmath>

namespace Belle2 {
  namespace VXD {
    /**
     * A Finite plane of one VXD Sensor.
     * This class takes the SensorID of the sensor and gets the dimensions from the SensorInfo of that Sensor.
     */
    class SensorPlane: public genfit::AbsFinitePlane {
    public:
      /**
       * Constructs the plane for a given VXD Sensor
       * @param sensorID SensorID of the Sensor for which this plane should be build
       * @param uTolerance tolerance in the u direction
       * @param vTolerance tolerance in the v direction
       */
      SensorPlane(VxdID sensorID = 0, double uTolerance = 0, double vTolerance = 0):
        m_sensorID(sensorID), m_uTolerance(uTolerance), m_vTolerance(vTolerance), m_cosPhi(1.), m_sinPhi(0.), m_sensorInfo(0) {}

      /** Set plane rotation angle.
       * This angle will be used to rotate coordinate parameters in inActive() method.
       * @param phi The angle by which the plane was rotated.
       */
      void setRotation(double phi)
      {
        m_cosPhi = cos(phi);
        m_sinPhi = sin(phi);
      }
      /** Destructor. */
      virtual ~SensorPlane() {}

      /** Return whether the given coordinates are inside the finite region.
       * @param u u-coordinate of the point.
       * @param v v-coordinate of the point.
       * @return true if (u,v) is within the sensor plane, otherwise false.
       */
      bool isInActive(double u, double v) const override
      {
#ifndef __CINT__
        //If running in ROOT CINT we do not know about GeoCache so we cannot get
        //the SensorInfo
        if (!m_sensorInfo) {
          m_sensorInfo = &VXD::GeoCache::get(m_sensorID);
        }
#endif
        //No sensorInfo set so we have to bail
        if (!m_sensorInfo) {
          B2FATAL("Could not find sensorInfo for VXD Sensor " << VxdID(m_sensorID));
        }
        double uRot = m_cosPhi * u - m_sinPhi * v;
        double vRot = m_sinPhi * u + m_cosPhi * v;
        return m_sensorInfo->inside(uRot, vRot, m_uTolerance, m_vTolerance);
      }

      /** Prints object data. */
      void Print(const Option_t* option = "") const override;

      /**
       * Deep copy of the object.
       * @return Pointer to a deep copy of the object.
       */
      virtual genfit::AbsFinitePlane* clone() const override
      {
        return new SensorPlane(*this);
      }

    private:
      /** Sensor ID of the sensor plane */
      unsigned short m_sensorID;
      /** Tolerance to add to the sensor dimensions in u direction */
      double m_uTolerance;
      /** Tolerance to add to the sensor dimensions in v direction */
      double m_vTolerance;
      /** Cosine term of plane rotation, used to align SVD trapezoidal sensors */
      double m_cosPhi;
      /** Sine term of plane rotation, used to align SVD trapezoidal sensors */
      double m_sinPhi;
      /** Pointer to the SensorInfo which contains the geometry information for the given sensor plane */
      mutable const SensorInfoBase* m_sensorInfo; //! transient member

      ClassDefOverride(SensorPlane, 2)
    };
  } // vxd namespace
} // Belle2 namespace

#endif /* VXD_SENSORPLANE_H */
