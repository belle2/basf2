/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TVector3.h>
#include <TVector2.h>

namespace Belle2 {
  namespace CDCTB {

    /**
     * Helper class for straight line in 3D.
     */
    class StraightLine {

    public:

      /**
       * Constructor
       * @param pos position (3D point)
       * @param dir direction (unit vector, but not necessary)
       */
      StraightLine(const TVector3& pos, const TVector3& dir):
        m_point(pos), m_dir(dir.Unit()), m_path(0)
      {}

      /**
       * Destructor
       */
      ~StraightLine()
      {}

      /**
       * Set path lenght
       * @param path path lenght
       */
      void setPath(double path) {m_path = path;}

      /**
       * Set new reference point according to internally stored path
       */
      void setReferencePoint()
      {
        m_point += m_path * m_dir;
        m_path = 0;
      }

      /**
       * Find cross section with a plane perpendicular to x-axis.
       * Result stored internally: use getPosition() to obtain the position
       * @param x plane at this x
       * @return true on success
       */
      bool crossPlaneX(double x = 0)
      {
        double path = (x - m_point.X()) / m_dir.X();
        if (!std::isfinite(path)) return false;
        m_path = path;
        return true;
      }

      /**
       * Find cross section with a plane perpendicular to y-axis
       * Result stored internally: use getPosition() to obtain the position
       * @param y plane at this y
       * @return true on success
       */
      bool crossPlaneY(double y = 0)
      {
        double path = (y - m_point.Y()) / m_dir.Y();
        if (!std::isfinite(path)) return false;
        m_path = path;
        return true;
      }

      /**
       * Find cross section with a plane perpendicular to z-axis
       * Result stored internally: use getPosition() to obtain the position
       * @param z plane at this z
       * @return true on success
       */
      bool crossPlaneZ(double z = 0)
      {
        double path = (z - m_point.Z()) / m_dir.Z();
        if (!std::isfinite(path)) return false;
        m_path = path;
        return true;
      }

      /**
       * Find cross section with a plane given by a point and its normal
       * Result stored internally: use getPosition() to obtain the position
       * @param point point on the plane
       * @param normal plane normal vector (unit vector, but not required to be)
       * @return true on success
       */
      bool crossPlane(const TVector3& point, const TVector3& normal);

      /**
       * Find cross section with a plane parallel to z at angle alpha
       * Result stored internally: use getPosition() to obtain the position
       * @param alpha plane angle (rotation around z)
       * @param rho plane at this radius
       * @return true on success
       */
      bool crossPlaneAlpha(double alpha, double rho = 0)
      {
        TVector3 normal(-sin(alpha), cos(alpha), 0);
        return crossPlane(rho * normal, normal);
      }

      /**
       * Find closest approach of two lines.
       * Result stored internally in both objects: use getPosition() to obtain the points.
       * @param line straight line
       * @return true on success
       */
      bool closestApproach(StraightLine& line);

      /**
       * Returns the position of a reference point
       * @return reference position
       */
      const TVector3& getReferencePoint() const {return m_point;}

      /**
       * Returns direction
       * @return direction unit vector
       */
      const TVector3& getDirection() const {return m_dir;}

      /**
       * Returns path length
       * @return path
       */
      double getPath() const {return m_path;}

      /**
       * Returns the position of a point given by internally stored path
       * @return position
       */
      TVector3 getPosition() const {return m_point + m_path * m_dir;}

      /**
       * Returns the position of a point at path. Internally stored path is not changed.
       * @return position
       */
      TVector3 getPosition(double path) const {return m_point + path * m_dir;}

      /**
       * Returns the path lenght in a cell (prism along z with trapezoidal cross section)
       * @param r1 inner cell radius
       * @param r2 outer cell radius
       * @param phi1 minimal angle phi
       * @param phi2 maximal angle phi
       * @return path length
       */
      double getLengthInCell(double r1, double r2, double phi1, double phi2) const;

    private:

      /**
       * Calculates and appends crossing point of two 2D lines
       * defined by (r1, r2) and (r0, dir) if the point lies between r1 and r2
       * @param crossPoints a vector of crossing points
       * @param r1 first point defining first line
       * @param r2 second point defining first line
       * @param r0 point defining second line
       * @param dir direction of the second line
       * @return size of crossPoint vector
       */
      int appendCrossPoint(std::vector<TVector2>& crossPoints,
                           const TVector2& r1, const TVector2& r2,
                           const TVector2& r0, const TVector2& dir) const;

      TVector3 m_point; /**< reference point defining the line */
      TVector3 m_dir;   /**< direction of line (unit vector) */
      double m_path;    /**< path lenght to some other point on the line */
    };

  } // CDCTB
} // Belle2


