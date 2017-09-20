/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef VXD_SENSORINFO_H
#define VXD_SENSORINFO_H

#include <vxd/dataobjects/VxdID.h>
#include <cmath>
#include <float.h>

#include <TGeoMatrix.h>
#include <TVector3.h>

namespace Belle2 {
  /** Namespace to provide code needed by both Vertex Detectors, PXD and SVD */
  namespace VXD {

    /** Base class to provide Sensor Information for PXD and SVD.
     * This class is meant to provide the basic services like length/thickness
     * parameters, pixel/stip ID and position calculation and coordinate transformation.
     */
    class SensorInfoBase {
    public:
      /** Enum specifing the type of sensor the SensorInfo represents */
      enum SensorType {
        PXD = 0,  /**< PXD Sensor */
        SVD = 1,  /**< SVD Sensor */
        TEL = 2,  /**< Testbeam telescope sensor */
        VXD = -1, /**< Any type of VXD Sensor */
      };

      /** Constructor for a SensorInfo instance
       *
       * this class holds all the information to calculate pixel/strip
       * positions and ids. it can accomodate rectangular and trapezoidal
       * shapes and also has the possibility to segment the strip/pixelsize
       * once in v, thus having two different pixel sizes in one sensor: vcells
       * pixels from start to splitlength and vcells2 pixels from splitlength
       * to length.
       *
       * @param type Type of the Sensor, one of PXD or SVD
       * @param id   VxdID of the Sensor
       * @param width Width of the Sensor in default units
       * @param length Length of the Sensor in default units
       * @param thickness Thickness of the Sensor in default units
       * @param uCells Number of Pixels/Strips in u direction (local x)
       * @param vCells Number of Pixels/Strips in v direction (local y)
       * @param width2 Width of trapezoidal sensor on positive side, <=0 means rectangular
       * @param splitLength Length at which the pixel size changes, starting at
       *        0. <=0 means only one pixel/strip size in v
       * @param vCells2 Number of pixels in v direction after split length.
       */
      SensorInfoBase(SensorType type, VxdID id, double width, double length, double thickness,
                     int uCells, int vCells, double width2 = -1, double splitLength = -1, int vCells2 = 0):
        m_type(type), m_id(id), m_width(width), m_length(length), m_thickness(thickness),
        m_deltaWidth(0), m_splitLength(0), m_uCells(uCells), m_vCells(vCells), m_vCells2(vCells2)
      {
        if (width2 > 0) m_deltaWidth = width2 - width;
        if (splitLength > 0) m_splitLength = splitLength / length;
      }
      /** Default constructor to make class polymorph */
      virtual ~SensorInfoBase() {}

      /** Return the Type of the Sensor */
      SensorType getType() const { return m_type; }
      /** Return the ID of the Sensor */
      VxdID getID() const { return m_id; }

      /** Return the width of the sensor
       * @param v v-coordinate where to determine the width, ignored for recangular sensors
       * @return width of the Sensor
       */
      double getWidth(double v = 0) const
      {
        if (m_deltaWidth == 0) return m_width;
        return m_width + (v / m_length + 0.5) * m_deltaWidth;
      }

      /** Convinience Wrapper to return width at backward side.
       * @return width of the sensor at the backward side
       */
      double getBackwardWidth() const
      {
        return getWidth(-0.5 * m_length);
      }

      /** Convinience Wrapper to return width at forward side.
       * @return width of the sensor at the forward side
       */
      double getForwardWidth() const
      {
        return getWidth(0.5 * m_length);
      }

      /** Return the length of the sensor
       * @return length of the sensor
       */
      double getLength() const { return m_length; }

      /** Return the thickness of the sensor
       * @return thickness of the sensor
       */
      double getThickness() const { return m_thickness; }

      /** Return the width of the sensor
       * @param v v-coordinate where to determine the width, ignored for recangular sensors
       * @return width of the Sensor
       */
      double getUSize(double v = 0) const { return getWidth(v); }

      /** Return the length of the sensor
       * @return length of the sensor
       */
      double getVSize() const { return getLength(); }

      /** Return the thickness of the sensor
       * @return thickness of the sensor
       */
      double getWSize() const { return getThickness(); }

      /** Return the pitch of the sensor
       * @param v v-coordinate where to determine the pitch, ignored for rectangular sensors
       * @return Pixel/Strip size in u direction
       */
      double getUPitch(double v = 0) const { return getWidth(v) / m_uCells; }

      /** Return the pitch of the sensor
       * @param v v-coordinate where to determine the pitch, only used for
       * sensors with two different pixel sizes along v
       * @return Pixel/Strip size in v direction
       */
      double getVPitch(double v = 0) const
      {
        if (m_splitLength <= 0) return m_length / m_vCells;
        if (v / m_length + 0.5 >= m_splitLength) return m_length * (1 - m_splitLength) / m_vCells2;
        return m_length * m_splitLength / m_vCells;
      }

      /** Return the pitch ID of the sensor
       * @param v v-coordinate where to determine the pitchID
       * only used for PXD sensors with two different pixel sizes along v
       * @return Pixel/Strip ID in v direction:
       * 0 for pitch at smaller v, 1 for bigger v
       *     Attention: Pitch ID depend from sensor position
       *       For PXD it swap for sensor=1 vs. seensor=2
       *       Sensor=1: bigger pitch = 0, smaller = 1
       *       Sensor=2: smaller pitch = 0, bigger = 1
       */
      int getVPitchID(double v = 0) const
      {
        if (m_splitLength <= 0) return 0;
        if (v / m_length + 0.5 >= m_splitLength) return 0;
        return 1;
      }

      /** Return the position of a specific strip/pixel in u direction
       * @param uID id of the strip/pixel in u coordinates
       * @param vID id of the strip/pixel in v coordinates, ignored for rectangular sensors
       * @return Pixel/Strip position in u direction
       */
      double getUCellPosition(int uID, int vID = -1) const
      {
        if (m_deltaWidth == 0) return ((uID + 0.5) / m_uCells - 0.5) * m_width;
        double v = 0;
        if (vID >= 0) v = getVCellPosition(vID);
        return ((uID + 0.5) / m_uCells - 0.5) * getWidth(v);
      }

      /** Return the position of a specific strip/pixel in v direction
       * @param vID id of the strip/pixel in v coordinates
       * @return Pixel/Strip position in v direction
       */
      double getVCellPosition(int vID) const
      {
        if (m_splitLength <= 0) return ((vID + 0.5) / m_vCells - 0.5) * m_length;
        if (vID >= m_vCells) return ((vID - m_vCells + 0.5) / m_vCells2 * (1 - m_splitLength) - 0.5 + m_splitLength) * m_length;
        return ((vID + 0.5) / m_vCells * m_splitLength - 0.5) * m_length;
      }

      /** Return the corresponding pixel/strip ID of a given u coordinate
       * @param u u coordinate of the pixel/strip
       * @param v v coordinate of the pixel/strip, ignored for rectangular sensors
       * @return ID of the pixel/strip covering the given coordinate
       */
      int getUCellID(double u, double v = 0, bool clamp = false) const
      {
        if (clamp) return std::min(getUCells() - 1, std::max(0, getUCellID(u, v, false)));
        return static_cast<int>((u / getWidth(v) + 0.5) * m_uCells);
      }

      /** Return the corresponding pixel/strip ID of a given v coordinate
       * @param v v coordinate of the pixel/strip
       * @return ID of the pixel/strip covering the given coordinate
       */
      int getVCellID(double v, bool clamp = false) const
      {
        if (clamp) return std::min(getVCells() - 1, std::max(0, getVCellID(v, false)));
        double nv = v / m_length + 0.5;
        if (m_splitLength <= 0) return static_cast<int>(nv * m_vCells);
        if (nv >= m_splitLength) return static_cast<int>((nv - m_splitLength) / (1 - m_splitLength) * m_vCells2) + m_vCells;
        return static_cast<int>(nv / m_splitLength * m_vCells);
      }

      /** Return number of pixel/strips in u direction */
      int getUCells() const { return m_uCells; }
      /** Return number of pixel/strips in v direction */
      int getVCells() const { return m_vCells + m_vCells2; }
      /** Return number of pixel/strips in v direction up to change pitch */
      int getVCells2() const { return m_vCells2; }

      /** Check wether a given point is inside the active area.
       * Optionally, one can specify a tolerance which should be added to the
       * sensor edges to still be considered inside
       * @param u u coordinate to check, supply 0 if not interested
       * @param v v coordinate to check, supply 0 if not interested
       * @param uTolerance tolerance to be added on each side of the sensor in u direction
       * @param vTolerance tolerance to be added on each side of the sensor in u direction
       * @return true if inside active area, false otherwise
       */
      bool inside(double u, double v, double uTolerance = DBL_EPSILON, double vTolerance = DBL_EPSILON) const
      {
        double nu = u / (getWidth(v) + 2 * uTolerance) + 0.5;
        double nv = v / (getLength() + 2 * vTolerance) + 0.5;
        return 0 <= nu && nu <= 1 && 0 <= nv && nv <= 1;
      }

      /** Check wether a given point is inside the active area
       * @param local point in local coordinates
       * @return true if inside active area, false otherwise
       */
      bool inside(const TVector3& local) const
      {
        double nw = local.z() / getThickness() + 0.5;
        return inside(local.x(), local.y()) && 0 <= nw && nw <= 1;
      }

      /** Force a position to be inside the active area
       * @param u u coordinate to be forced inside
       * @param v v coordinate to be forced inside
       */
      void forceInside(double& u, double& v) const
      {
        double length = getLength() / 2.0;
        v = std::min(length, std::max(-length, v));
        double width = getWidth(v) / 2.0;
        u = std::min(width, std::max(-width, u));
      }

      /** Force a given point inside the active area
       * @param local point in local coordinates, will be modified to lie
       * inside or at the border of the sensor
       */
      void forceInside(TVector3& local) const;

      /** Convert a point from local to global coordinates
       * @param local point in local coordinates
       * @param reco Use sensor position in reconstruction (true) or in nominal geometry (false)
       * @return point in global coordinates
       */
      TVector3 pointToGlobal(const TVector3& local, bool reco = false) const;

      /** Convert a vector from local to global coordinates
       * @param local vector in local coordinates
       * @param reco Use sensor position in reconstruction (true) or in nominal geometry (false)
       * @return vector in global coordinates
       */
      TVector3 vectorToGlobal(const TVector3& local, bool reco = false) const;

      /** Convert a point from global to local coordinates
       * @param global point in global coordinates
       * @param reco Use sensor position in reconstruction (true) or in nominal geometry (false)
       * @return point in local coordinates
       */
      TVector3 pointToLocal(const TVector3& global, bool reco = false) const;

      /** Convert a vector from global to local coordinates
       * @param global vector in global coordinates
       * @param reco Use sensor position in reconstruction (true) or in nominal geometry (false)
       * @return vector in local coordinates
       */
      TVector3 vectorToLocal(const TVector3& global, bool reco = false) const;

      /** Set the transformation matrix of the Sensor
       * @param transform Transformation matrix of the Sensor
       * @param reco Set transformation for reconstruction (true) or nominal (false)
       */
      void setTransformation(const TGeoHMatrix& transform, bool reco = false)
      {
        if (reco) m_recoTransform = transform;
        else m_transform = transform;
      }

      /** Return the transformation matrix of the Sensor
       * @return Transformation matrix of the Sensor
       * @param reco Get transformation for reconstruction (true) or nominal (false)
       */
      const TGeoHMatrix& getTransformation(bool reco = false) const
      {
        if (reco) return m_recoTransform;
        else return m_transform;
      }

    protected:
      /** Type of the Sensor */
      SensorType m_type;
      /** ID of the Sensor */
      unsigned short m_id;
      /** Width of the sensor */
      double m_width;
      /** Length of the Sensor */
      double m_length;
      /** Thickness of the Sensor */
      double m_thickness;
      /** Difference between backward and forward width, 0 for rectangular sensors */
      double m_deltaWidth;
      /** Relative length at which second pixel size starts, 0 for only one pixel size */
      double m_splitLength;
      /** Number of strips/pixels in u direction */
      int m_uCells;
      /** Number of strips/pixels in v direction (up to splitLength for two pixel sizes) */
      int m_vCells;
      /** Number of strips/pixels in v direction after splitLength, 0 for only one pixel size */
      int m_vCells2;
      /** Nominal transformation matrix of the Sensor */
      TGeoHMatrix m_transform;
      /** Alignment-corrected transformation matrix of the Sensor for use in reconstruction */
      TGeoHMatrix m_recoTransform;
    };

    inline void SensorInfoBase::forceInside(TVector3& local) const
    {
      double u = local.x();
      double v = local.y();
      double thickness = getThickness() / 2.0;
      forceInside(u, v);
      local.SetX(u);
      local.SetY(v);
      local.SetZ(std::min(thickness, std::max(-thickness, local.z())));
    }

    inline TVector3 SensorInfoBase::pointToGlobal(const TVector3& local, bool reco) const
    {
      double clocal[3];
      double cmaster[3];
      local.GetXYZ(clocal);
      if (reco) m_recoTransform.LocalToMaster(clocal, cmaster);
      else m_transform.LocalToMaster(clocal, cmaster);
      return TVector3(cmaster);
    }

    inline TVector3 SensorInfoBase::vectorToGlobal(const TVector3& local, bool reco) const
    {
      double clocal[3];
      double cmaster[3];
      local.GetXYZ(clocal);
      if (reco) m_recoTransform.LocalToMasterVect(clocal, cmaster);
      else m_transform.LocalToMasterVect(clocal, cmaster);
      return TVector3(cmaster);
    }

    inline TVector3 SensorInfoBase::pointToLocal(const TVector3& global, bool reco) const
    {
      double clocal[3];
      double cmaster[3];
      global.GetXYZ(cmaster);
      if (reco) m_recoTransform.MasterToLocal(cmaster, clocal);
      else m_transform.MasterToLocal(cmaster, clocal);
      return TVector3(clocal);
    }

    inline TVector3 SensorInfoBase::vectorToLocal(const TVector3& global, bool reco) const
    {
      double clocal[3];
      double cmaster[3];
      global.GetXYZ(cmaster);
      if (reco) m_recoTransform.MasterToLocalVect(cmaster, clocal);
      else m_transform.MasterToLocalVect(cmaster, clocal);
      return TVector3(clocal);
    }
  }
} //Belle2 namespace
#endif
