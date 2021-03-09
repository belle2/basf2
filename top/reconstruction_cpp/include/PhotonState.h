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
#include <top/reconstruction_cpp/func.h>
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
       * Default constructor
       */
      PhotonState()
      {}

      /**
       * Constructor with position and direction vectors
       * @param position initial photon position (must be inside quartz)
       * @param direction initial direction vector (must be unit vector)
       */
      PhotonState(const TVector3& position, const TVector3& direction);

      /**
       * Constructor with position vector and direction components
       * @param position initial photon position (must be inside quartz)
       * @param kx initial direction x-component (must be unit vector)
       * @param ky initial direction y-component (must be unit vector)
       * @param kz initial direction z-component (must be unit vector)
       */
      PhotonState(const TVector3& position, double kx, double ky, double kz);

      /**
       * Constructor with position, track direction and cerenkov angles
       * @param position initial photon position (must be inside quartz)
       * @param trackDir track direction vector (must be unit vector)
       * @param thc Cerenkov (polar) angle
       * @param fic Cerenkov azimuthal angle
       */
      PhotonState(const TVector3& position, const TVector3& trackDir, double thc, double fic);

      /**
       * Changes sign of direction y component
       */
      PhotonState& flipKy() {m_ky = -m_ky; return *this;}

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
      double getUnfoldedX(double x) const {return func::unfold(x, m_nx, m_A);}

      /**
       * Unfolds the position in y.
       * @param y position to unfold
       * @return unfolded position
       */
      double getUnfoldedY(double y) const {return (func::unfold(y - m_y0, m_ny, m_B) + m_y0);}

      /**
       * Returns detection position x in unfolded prism
       * @return detection position x in unfolded prism
       */
      double getXD() const {return getUnfoldedX(m_x);}

      /**
       * Returns detection position y in unfolded prism
       * @return detection position y in unfolded prism
       */
      double getYD() const {return m_yD;}

      /**
       * Returns detection position z in unfolded prism
       * @return detection position z in unfolded prism
       */
      double getZD() const {return m_zD;}

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
       * Returns total internal reflection status.
       * @param cosTotal cosine of total reflection angle
       * @return true if totally reflected
       */
      bool getTotalReflStatus(double cosTotal) const
      {
        return ((m_nx == 0 or m_cosx < cosTotal) and (m_ny == 0 or m_cosy < cosTotal));
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

      double m_x = 0; /**< position in x */
      double m_y = 0; /**< position in y */
      double m_z = 0; /**< position in z */
      double m_kx = 0; /**< direction in x */
      double m_ky = 0; /**< direction in y */
      double m_kz = 0; /**< direction in z */
      double m_propLen = 0; /**< propagation length since initial position */
      int m_nx = 0; /**< signed number of reflections in x at last propagation step */
      int m_ny = 0; /**< signed number of reflections in y at last propagation step */
      double m_cosx = 0; /**< maximal cosine of impact angle to surface in x */
      double m_cosy = 0; /**< maximal cosine of impact angle to surface in y */
      double m_A = 0; /**< width of the quartz segment (dimension in x) for unfolding */
      double m_B = 0; /**< thickness of the quartz segment (dimension in y) for unfolding */
      double m_y0 = 0; /**< origin in y for unfolding */
      double m_yD = 0; /**< unfolded prism detection position in y */
      double m_zD = 0; /**< unfolded prism detection position in z */
      EType m_type = c_Undefined; /**< quartz segment type at last propagation step */
      bool m_status = false; /**< propagation status */

      static double s_maxLen; /**< maximal allowed propagation length */

    };

  } // namespace TOP
} // namespace Belle2

