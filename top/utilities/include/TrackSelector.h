/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <top/reconstruction/TOPtrack.h> // old version (TODO: to be removed)
#include <top/reconstruction_cpp/TOPTrack.h>
#include <framework/gearbox/Const.h>
#include <string>
#include <TVector3.h>

namespace Belle2 {
  namespace TOP {

    /**
     * Utility for the track selection - used in various calibration modules
     */
    class TrackSelector {

    public:

      /**
       * enumerators for possible data sample types (skims)
       */
      enum EType {
        c_undefined = 0,
        c_dimuon = 1,
        c_bhabha = 2,
        c_cosmics = 3
      };

      /**
       * Default constructor
       */
      TrackSelector()
      {}

      /**
       * Useful constructor
       * @param sampleName data sample name, one of dimuon, bhabha or cosmics
       */
      explicit TrackSelector(const std::string& sampleName);

      /**
       * Sets momentum cut (used for "cosmics" only)
       * @param minMomentum minimal track momentum
       */
      void setMinMomentum(double minMomentum) {m_minMomentum = minMomentum;}

      /**
       * Sets cut on c.m.s. energy (used for "dimuon" and "bhabha")
       * @param deltaEcms size of energy window
       */
      void setDeltaEcms(double deltaEcms) {m_deltaEcms = deltaEcms;}

      /**
       * Sets cut on point of closest approach to (0, 0, 0)
       * @param dr maximal radius in x-y plane
       * @param dz cut in z coordinate (|z| < dz/2)
       */
      void setCutOnPOCA(double dr, double dz)
      {
        m_dr = dr;
        m_dz = dz;
      }

      /**
       * Sets cut on local z coordinate (module frame) of the track extrapolated to TOP
       * @param minZ minimal z
       * @param maxZ maximal z
       */
      void setCutOnLocalZ(double minZ, double maxZ)
      {
        m_minZ = minZ;
        m_maxZ = maxZ;
      }

      /**
       * Returns sample name
       * @return sample name
       */
      const std::string& getSampleName() const {return m_sampleName;}

      /**
       * Returns momentum cut
       * @return minimal momentum
       */
      double getMinMomentum() const {return m_minMomentum;}

      /**
       * Returns c.m.s. energy cut
       * @return size of the energy window
       */
      double getDeltaEcms() const {return m_deltaEcms;}

      /**
       * Returns POCA cut in r
       * @return maximal radius in x-y plane
       */
      double getDr() const {return m_dr;}

      /**
       * Returns POCA cut in z
       * @return cut in z coordinate (|z| < dz/2)
       */
      double getDz() const {return m_dz;}

      /**
       * Returns cut on local z
       * @return minimal local z of extrapolated hit
       */
      double getMinZ() const {return m_minZ;}

      /**
       * Returns cut on local z
       * @return maximal local z of extrapolated hit
       */
      double getMaxZ() const {return m_maxZ;}

      /**
       * Returns track hypothesis
       */
      const Const::ChargedStable& getChargedStable() const {return m_chargedStable;}

      /**
       * Returns position of POCA of the track in last isSelected call
       * @return POCA if track is selected, otherwise undefined
       */
      const TVector3& getPOCAPosition() const {return m_pocaPosition;}

      /**
       * Returns momentum vector at POCA of the track in last isSelected call
       * @return momentum vector if track is selected, otherwise undefined
       */
      const TVector3& getPOCAMomentum() const {return m_pocaMomentum;}

      /**
       * Returns c.m.s energy of the track in last isSelected call
       * @return c.m.s. energy if track is selected, otherwise undefined
       */
      double getCMSEnergy() const {return m_cmsEnergy;}

      /**
       * Returns position at TOP in local frame of the track in last isSelected call
       * @return position in local frame if track is selected, otherwise undefined
       */
      const TVector3& getLocalPosition() const {return m_localPosition;}

      /**
       * Returns momentum at TOP in local frame of the track in last isSelected call
       * @return momentum vector in local frame if track is selected, otherwise undefined
       */
      const TVector3& getLocalMomentum() const {return m_localMomentum;}

      /**
       * Returns selection status - old version (TODO: to be removed)
       * @param track track extrapolated to TOP
       * @return true if track passes selection criteria
       */
      bool isSelected(const TOPtrack& track) const;

      /**
       * Returns selection status
       * @param track track extrapolated to TOP
       * @return true if track passes selection criteria
       */
      bool isSelected(const TOPTrack& track) const;

    private:

      EType m_sampleType = c_undefined; /**< data sample (skim) type */
      std::string m_sampleName; /**< data sample (skim) name */
      double m_minMomentum = 0; /**< minimal track momentum if sample is "cosmics" */
      double m_deltaEcms = 0; /**< c.m.s energy window for "dimuon" or "bhabha" */
      double m_dr = 0; /**< cut on POCA in r */
      double m_dz = 0; /**< cut on POCA in z */
      double m_minZ = 0; /**< minimal local z of extrapolated hit */
      double m_maxZ = 0;  /**< maximal local z of extrapolated hit */
      Const::ChargedStable m_chargedStable = Const::muon; /**< track hypothesis */

      /* cache for track parameters */
      mutable TVector3 m_pocaPosition; /**< position of POCA */
      mutable TVector3 m_pocaMomentum; /**< momentum at POCA */
      mutable double m_cmsEnergy = 0;  /**< c.m.s. energy of track */
      mutable TVector3 m_localPosition; /**< position at TOP in local (module) frame */
      mutable TVector3 m_localMomentum; /**< momentum at TOP in local (module) frame */

    };

  } // namespace TOP
} // namespace Belle2
