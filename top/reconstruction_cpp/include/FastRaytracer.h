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

#include <top/reconstruction_cpp/PhotonState.h>
#include <vector>


namespace Belle2 {
  namespace TOP {

    /**
     * Fast photon propagation in a quartz optics.
     */
    class FastRaytracer {

    public:
      /**
       * Treatement of quartz geometry.
       */
      enum EGeometry {
        c_Unified = 0,   /**< single bar with average width and thickness */
        c_Segmented = 1  /**< segmented bars */
      };

      /**
       * Treatement of spherical mirror optics.
       */
      enum EOptics {
        c_SemiLinear = 0, /**< semi-linear approximation */
        c_Exact = 1       /**< exact optics */
      };

      /**
       * constructor
       * @param moduleID slot ID
       * @param geometry treatement of quartz geometry
       * @param optics treatement of spherical mirror optics
       */
      FastRaytracer(int moduleID, EGeometry geometry = c_Unified, EOptics optics = c_SemiLinear);

      /**
       * Returns slot ID.
       * @return slot ID
       */
      int getModuleID() const {return m_moduleID;}

      /**
       * Returns quartz geometry treatement.
       * @return quartz geometry treatement
       */
      EGeometry getGeometry() const {return m_geometry;}

      /**
       * Returns treatement of spherical mirror optics.
       * @return spherical mirror optics
       */
      EOptics getOptics() const {return m_optics;}

      /**
       * Returns geometry data of bar segments.
       * @return geometry data of bar segments
        */
      const std::vector<PhotonState::BarSegment>& getBars() const {return m_bars;}

      /**
       * Returns geometry data of spherical mirror.
       * @return geometry data of spherical mirror
       */
      const PhotonState::Mirror& getMirror() const {return m_mirror;}

      /**
       * Returns geometry data of prism.
       * @return geometry data of prism
       */
      const PhotonState::Prism& getPrism() const {return m_prism;}

      /**
       * Propagate photon to photo-detector plane.
       * @param photon initial photon state
       */
      void propagate(const PhotonState& photon) const;

      /**
       * Returns photon states (results of propagation).
       * @return photon states
       */
      const std::vector<PhotonState>& getPhotonStates() const {return m_photonStates;}

      /**
       * Returns propagation status.
       * @return true on success
       */
      bool getPropagationStatus() const {return m_status;}

      /**
       * Returns total internal reflection status.
       * Reliable only if propagation status is true.
       * @param cosTotal cosine of total reflection angle
       * @return true if totally reflected
       */
      bool getTotalReflStatus(double cosTotal) const;

      /**
       * Returns unfolded position in x at Detector plane.
       * Reliable only if propagation status is true.
       * For reflected photons unfolding is done up to spherical mirror.
       * @return unfolded position in x at Detector plane
       */
      double getXD() const;

      /**
       * Returns unfolded position in y at Detector plane.
       * Reliable only if propagation status is true.
       * For reflected photons unfolding is done up to spherical mirror.
       * @return unfolded position in y at Detector plane
       */
      double getYD() const;

      /**
       * Returns unfolded position in y at Bar-prism-connection plane.
       * Reliable only if propagation status is true.
       * For reflected photons unfolding is done up to spherical mirror.
       * @return unfolded position in y at Bar-prism-connection plane
       */
      double getYB() const;

      /**
       * Returns signed number of reflections in x.
       * Reliable only if propagation status is true.
       * @return number of reflections in x
       */
      int getNx() const {return (m_Nxm + m_Nxb + m_Nxe);}

      /**
       * Returns signed number of reflections in x before mirror.
       * Reliable only if propagation status is true.
       * @return number of reflections in x
       */
      int getNxm() const {return m_Nxm;}

      /**
       * Returns signed number of reflections in x after mirror and before prism.
       * Reliable only if propagation status is true.
       * @return number of reflections in x
       */
      int getNxb() const {return m_Nxb;}

      /**
       * Returns signed number of reflections in x inside prism.
       * Reliable only if propagation status is true.
       * @return number of reflections in x
       */
      int getNxe() const {return m_Nxe;}

      /**
       * Returns signed number of reflections in y.
       * Reliable only if propagation status is true.
       * @return number of reflections in y
       */
      int getNy() const {return (m_Nym + m_Nyb + m_Nye);}

      /**
       * Returns signed number of reflections in y before mirror.
       * Reliable only if propagation status is true.
       * @return number of reflections in y
       */
      int getNym() const {return m_Nym;}

      /**
       * Returns signed number of reflections in y after mirror and before prism.
       * Reliable only if propagation status is true.
       * @return number of reflections in y
       */
      int getNyb() const {return m_Nyb;}

      /**
       * Returns signed number of reflections in y inside prism.
       * Reliable only if propagation status is true.
       * @return number of reflections in y
       */
      int getNye() const {return m_Nye;}


    private:

      int m_moduleID = 0; /**< slot ID */
      EGeometry m_geometry = c_Unified; /**< quartz geometry */
      EOptics m_optics = c_SemiLinear;  /**< spherical mirror optics */

      std::vector<PhotonState::BarSegment> m_bars; /**< geometry data of bar segments */
      PhotonState::Mirror m_mirror; /**< spherical mirror geometry data */
      PhotonState::Prism m_prism; /**< prism geometry data */

      mutable std::vector<PhotonState> m_photonStates; /**< photon states at propagation steps */
      mutable bool m_status = false; /**< propagation status */
      mutable int m_Nxm = 0; /**< number of reflections in x before mirror */
      mutable int m_Nxb = 0; /**< number of reflections in x after mirror and before prism */
      mutable int m_Nxe = 0; /**< number of reflections in x inside prism */
      mutable int m_Nym = 0; /**< number of reflections in y before mirror */
      mutable int m_Nyb = 0; /**< number of reflections in y after mirror and before prism */
      mutable int m_Nye = 0; /**< number of reflections in y inside prism */

    };

  } // namespace TOP
} // namespace Belle2
