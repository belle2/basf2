/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <top/reconstruction_cpp/RaytracerBase.h>
#include <TVector3.h>

namespace Belle2 {
  namespace TOP {

    /**
     * State of the Cerenkov photon in the quartz optics.
     */
    class PhotonState {

    public:

      /**
       * enumerator for the type of quartz segments
       */
      enum EType {
        c_Undefined = 0,     /**< undefined */
        c_BarSegment = 1,    /**< bar segment */
        c_MirrorSegment = 2, /**< mirror segment */
        c_Prism = 3          /**< prism */
      };

      /**
       * Constructor
       * @param position initial photon position (must be inside quartz)
       * @param direction initial direction vector (must be unit vector)
       */
      PhotonState(const TVector3& position, const TVector3& direction);

      /**
       * Sets maximal allowed propagation length.
       * @param maxLen maximal allowed propagation length
       */
      static void setMaxPropagationLen(double maxLen) {s_maxLen = maxLen;}

      /**
       * Returns position as 3D vector.
       * @return position
       */
      TVector3 getPosition() const {return TVector3(m_x, m_y, m_z);}

      /**
       * Returns position in x.
       * @return position in x
       */
      double getX() const {return m_x;}

      /**
       * Returns position in y.
       * @return position in y
       */
      double getY() const {return m_y;}

      /**
       * Returns position in z.
       * @return position in z
       */
      double getZ() const {return m_z;}

      /**
       * Unfolds the position in x.
       * @param x position to unfold
       * @return unfolded position
       */
      double getUnfoldedX(double x) const {return unfold(x, m_nx, m_A);}

      /**
       * Unfolds the position in y.
       * @param y position to unfold
       * @return unfolded position
       */
      double getUnfoldedY(double y) const {return (unfold(y - m_y0, m_ny, m_B) + m_y0);}

      /**
       * Returns direction as 3D unit vector.
       * @return direction
       */
      TVector3 getDirection() const {return TVector3(m_kx, m_ky, m_kz);}

      /**
       * Returns direction in x.
       * @return direction in x
       */
      double getKx() const {return m_kx;}

      /**
       * Returns direction in y.
       * @return direction in y
       */
      double getKy() const {return m_ky;}

      /**
       * Returns direction in z.
       * @return direction in z
       */
      double getKz() const {return m_kz;}

      /**
       * Returns total propagation length since initial position.
       * @return propagation length
       */
      double getPropagationLen() const {return m_propLen;}

      /**
       * Returns number of reflections in x at last propagation step.
       * @return signed number of reflections
       */
      int getNx() const {return m_nx;}

      /**
       * Returns number of reflections in y at last propagation step.
       * @return signed number of reflections
       */
      int getNy() const {return m_ny;}

      /**
       * Returns width (dimension in x) of the quartz segment at last propagation step.
       * @return width
       */
      double getA() const {return m_A;}

      /**
       * Returns thickness (dimension in y) of the quartz segment at last propagation step.
       * For prism it is the size of exit window.
       * @return thickness
       */
      double getB() const {return m_B;}

      /**
       * Returns the type of the quartz segment at last propagation.
       * @return type quartz segment type
       */
      EType getSegmentType() const {return m_type;}

      /**
       * Returns propagation status.
       * @return true on success
       */
      bool getPropagationStatus() const {return m_status;}

      /**
       * Returns total internal reflection status. TODO: check also slanted prism side!
       * @param cosTotal cosine of total reflection angle
       * @return true if totally reflected
       */
      bool getTotalReflStatus(double cosTotal) const
      {
        return ((m_nx == 0 or abs(m_kx) < cosTotal) and (m_ny == 0 or abs(m_ky) < cosTotal));
      }

      /**
       * Checks if photon is inside the bar segment (including surface).
       * @param bar bar segment data
       * @return true if inside
       */
      bool isInside(const RaytracerBase::BarSegment& bar) const;

      /**
       * Checks if photon is inside the mirror segment (including surface).
       * @param bar bar segment data
       * @param mirror spherical mirror data
       * @return true if inside
       */
      bool isInside(const RaytracerBase::BarSegment& bar, const RaytracerBase::Mirror& mirror) const;

      /**
       * Checks if photon is inside the prism (including surface).
       * @param prism prism data
       * @return true if inside
       */
      bool isInside(const RaytracerBase::Prism& prism) const;

      /**
       * Propagate photon to the exit of bar segment.
       * @param bar bar segment data
       * @return true on success
       */
      void propagate(const RaytracerBase::BarSegment& bar);

      /**
       * Propagate photon to the mirror and reflect it using semi-linear mirror optics.
       * Semi-linear: mirror surface approximated with a cylinder along y-axis
       * and using linear optics approximation in y for the photon reflection.
       * Useful to get rid of some of the discontinuities in the TOP image at given wavelength,
       * which are anyway smeared-out by the dispersion and other processes.
       * @param bar mirror segment data
       * @param mirror spherical mirror data
       * @return true on success
       */
      void propagateSemiLinear(const RaytracerBase::BarSegment& bar, const RaytracerBase::Mirror& mirror);

      /**
       * Propagate photon to the mirror and reflect it using exact mirror optics.
       * @param bar mirror segment data
       * @param mirror spherical mirror data
       * @return true on success
       */
      void propagateExact(const RaytracerBase::BarSegment& bar, const RaytracerBase::Mirror& mirror);

      /**
       * Propagate photon in the prism to the detector plane.
       * @param prism prism data
       * @return true on success
       */
      void propagate(const RaytracerBase::Prism& prism);

    private:

      /**
       * unfold a coordinate.
       * @param x true position
       * @param nx signed number of reflections
       * @param A size for unfolding
       * @return unfolded coordinate (position of image)
       */
      double unfold(double x, int nx, double A) const;

      /**
       * unfold a direction.
       * @param kx true direction component
       * @param nx signed number of reflections
       * @return unfolded direction component
       */
      double unfold(double kx, int nx) const;

      /**
       * fold a coordinate (inverse of unfold).
       * @param xu unfolded coordinate (position of image)
       * @param A size for folding
       * @param x true position [out]
       * @param kx true direction component [out]
       * @param nx signed number of reflections [out]
       */
      void fold(double xu, double A, double& x, double& kx, int& nx) const;

      double m_x = 0; /**< position in x */
      double m_y = 0; /**< position in y */
      double m_z = 0; /**< position in z */
      double m_kx = 0; /**< direction in x */
      double m_ky = 0; /**< direction in y */
      double m_kz = 0; /**< direction in z */
      double m_propLen = 0; /**< propagation length since initial position */
      int m_nx = 0; /**< signed number of reflections in x at last propagation step */
      int m_ny = 0; /**< signed number of reflections in y at last propagation step */
      double m_A = 0; /**< width of the quartz segment (dimension in x) for unfolding */
      double m_B = 0; /**< thickness of the quartz segment (dimension in y) for unfolding */
      double m_y0 = 0; /**< origin in y for unfolding */
      EType m_type = c_Undefined; /**< quartz segment type at last propagation step */
      bool m_status = false; /**< propagation status */

      static double s_maxLen; /**< maximal allowed propagation length */

    };


    inline double PhotonState::unfold(double x, int nx, double A) const
    {
      if (nx % 2 == 0) return (nx * A + x);
      else return (nx * A - x);
    }


    inline double PhotonState::unfold(double kx, int nx) const
    {
      if (nx % 2 == 0) return kx;
      else return -kx;
    }


    inline void PhotonState::fold(double xu, double A, double& x, double& kx, int& nx) const
    {
      nx = lround(xu / A);
      x = xu - nx * A;
      if (nx % 2 != 0) {
        x = -x;
        kx = -kx;
      }
    }


  } // namespace TOP
} // namespace Belle2

