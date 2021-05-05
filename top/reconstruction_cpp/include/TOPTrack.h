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

#include <TVector3.h>
#include <framework/gearbox/Const.h>
#include <top/reconstruction_cpp/HelixSwimmer.h>
#include <top/reconstruction_cpp/RaytracerBase.h>
#include <framework/database/DBObjPtr.h>
#include <top/dbobjects/TOPCalModuleAlignment.h>
#include <top/dbobjects/TOPFrontEndSetting.h>
#include <mdst/dataobjects/MCParticle.h>

#include <vector>
#include <map>
#include <unordered_map>
#include <string>

namespace Belle2 {

  class Track;
  class ExtHit;
  class TOPBarHit;

  namespace TOP {

    /**
     * Reconstructed track at TOP
     */
    class TOPTrack {

    public:

      /**
       * Sine and cosine of track polar and azimuthal angles at assumed photon emission
       */
      struct TrackAngles {
        double cosTh = 0; /**< cosine of polar angle */
        double sinTh = 0; /**< sine of polar angle */
        double cosFi = 0; /**< cosine of azimuthal angle */
        double sinFi = 0; /**< sine of azimuthal angle */

        /**
         * Default constructor
         */
        TrackAngles()
        {}

        /**
         * Constructor from direction vector
         * @param direction track direction at photon emission (must be unit vector)
         */
        explicit TrackAngles(const TVector3& direction);

        /**
         * Returns direction vector
         * @return direction vector
         */
        TVector3 getDirection() const {return TVector3(cosFi * sinTh, sinFi * sinTh, cosTh);}
      };

      /**
       * assumed photon emission point in local frame
       */
      struct AssumedEmission {
        TVector3 position;  /**< position */
        TrackAngles trackAngles; /**< sine and cosine of track polar and azimuthal angles */
        bool isSet = false; /**< flag */
      };

      /**
       * selected photon hit from TOPDigits
       */
      struct SelectedHit {
        int pixelID = 0;    /**< pixel ID */
        double time = 0;    /**< time */
        double timeErr = 0; /**< time uncertainty */

        /**
         * Constructor
         * @param pix pixel ID
         * @param t time
         * @param terr time uncertainty
         */
        SelectedHit(int pix, double t, double terr):
          pixelID(pix), time(t), timeErr(terr)
        {}
      };

      /**
       * Default constructor
       */
      TOPTrack()
      {}

      /**
       * Constructor from mdst track - isValid() must be checked before using the object
       * @param track mdst track
       * @param digitsName name of TOPDigits collection
       * @param chargedStable hypothesis used in mdst track extrapolation
       */
      explicit TOPTrack(const Track& track, const std::string& digitsName = "",
                        const Const::ChargedStable& chargedStable = Const::pion);

      /**
       * Constructor from extrapolated track hit - isValid() must be checked before using the object
       * @param extHit extrapolated track hit
       * @param digitsName name of TOPDigits collection
       */
      explicit TOPTrack(const ExtHit* extHit, const std::string& digitsName = "");

      /**
       * Overrides transformation from local to nominal frame, which is by default obtained from DB.
       * Needed for module alignment.
       * @param rotation rotation matrix from local to nominal frame (rotation first ...)
       * @param translation translation vector from local to nominal frame (... then translation)
       * @return true if quartz bar intersection still exists
       */
      bool overrideTransformation(const TRotation& rotation, const TVector3& translation)
      {
        m_valid = setHelix(rotation, translation);
        return m_valid;
      }

      /**
       * Checks if track is successfully constructed
       * @return true if successful
       */
      bool isValid() const {return m_valid;}

      /**
       * Returns slot ID
       * @return slot ID
       */
      int getModuleID() const {return m_moduleID;}

      /**
       * Returns momentum magnitude
       * @return momentum magnitude
       */
      double getMomentumMag() const {return m_momentum;}

      /**
       * Returns charge
       * @return charge in units of elementary charge
       */
      double getCharge() const {return m_charge;}

      /**
       * Returns track length from IP to the average position of photon emission within quartz
       * @return track length from IP
       */
      double getTrackLength() const {return m_trackLength;}

      /**
       * Returns track length within quartz
       * @return track length within quartz
       */
      double getLengthInQuartz() const {return m_length;}

      /**
       * Returns particle beta
       * @param particle particle mass hypothesis
       * @return particle beta
       */
      double getBeta(const Const::ChargedStable& particle) const;

      /**
       * Returns assumed photon emission position and track direction
       * @param dL length difference to the average emission position
       * @return assumed photon emission position and track direction in local frame
       */
      const TOPTrack::AssumedEmission& getEmissionPoint(double dL = 0) const;

      /**
       * Returns time-of-flight from IP to photon emission position
       * @param particle particle mass hypothesis
       * @param dL length difference to the average emission position
       * @return time-of-flight
       */
      double getTOF(const Const::ChargedStable& particle, double dL = 0) const;

      /**
       * Returns helix
       * Helix is given in nominal slot frame and with reference position at average photon emission
       * @return helix
       */
      const TOP::HelixSwimmer& getHelix() const {return m_helix;}

      /**
       * Returns mdst track
       * @return mdst track
       */
      const Track* getTrack() const {return m_track;}

      /**
       * Returns extrapolated hit (track entrance to the bar)
       * @return extrapolated hit
       */
      const ExtHit* getExtHit() const {return m_extHit;}

      /**
       * Returns MC particle assigned to this track (if any)
       * @return MC particle or NULL pointer
       */
      const MCParticle* getMCParticle() const {return m_mcParticle;}

      /**
       * Returns PDG code of associated MCParticle (returns 0 if none)
       * @return PDG code or 0
       */
      int getPDGCode() const
      {
        if (m_mcParticle) return m_mcParticle->getPDG();
        return 0;
      }

      /**
       * Returns bar hit of MC particle assigned to this track (if any)
       * @return bar hit or NULL pointer
       */
      const TOPBarHit* getBarHit() const {return m_barHit;}

      /**
       * Returns selected photon hits from TOPDigits belonging to the slot ID
       * @return selected photon hits
       */
      const std::vector<SelectedHit>& getSelectedHits() const {return m_selectedHits;}

      /**
       * Returns estimated background hit rate
       * @return background hit rate per module
       */
      double getBkgRate() const {return m_bkgRate;}

      /**
       * Checks if scan method of YScanner is needed to construct PDF for a given pixel column.
       * @param col pixel column (0-based)
       * @param time PDF peak time
       * @param wid PDF peak width squared
       * @return true if at least one of the detected photons is within the PDF peak convoluted with TTS.
       */
      bool isScanRequired(unsigned col, double time, double wid) const;

    private:

      /**
       * Sets the object (called by constructors)
       * @param track mdst track
       * @param digitsName name of TOPDigits collection
       * @param chargedStable hypothesis used in mdst track extrapolation
       */
      void set(const Track& track, const std::string& digitsName, const Const::ChargedStable& chargedStable);

      /**
       * Sets helix (helix is given in nominal frame)
       * @param rotation rotation matrix from local to nominal frame (rotation first ...)
       * @param translation translation vector from local to nominal frame (... then translation)
       * @return true if quartz bar intersection exists
       */
      bool setHelix(const TRotation& rotation, const TVector3& translation);

      /**
       * Calculates intersection of trajectory with prism
       * @param lengths trajectory lengths relative to extHit position of intersection points [in/out]
       * @param positions positions of intersection points in module local frame [in/out]
       * @param prism prism geometry data
       * @param rotation rotation matrix from local to nominal frame (rotation first ...)
       * @param translation translation vector from local to nominal frame (... then translation)
       * @return true if intersection exists
       */
      bool xsecPrism(std::vector<double>& lengths, std::vector<TVector3>& positions,
                     const RaytracerBase::Prism& prism, const TRotation& rotation, const TVector3& translation);

      int m_moduleID = 0;  /**< slot ID */
      double m_momentum = 0; /**< track momentum magnitude */
      double m_charge = 0;  /**< track charge in units of elementary charge */
      double m_TOFLength = 0; /**< trajectory length corresponding to TOF of extrapolated hit */
      double m_trackLength = 0;  /**< trajectory length from IP to average photon emission point */
      double m_length = 0; /**< trajectory length within quartz */
      TOP::HelixSwimmer m_helix; /**< trajectory helix in nominal slot frame */
      DBObjPtr<TOPCalModuleAlignment> m_alignment;   /**< module alignment constants */
      DBObjPtr<TOPFrontEndSetting> m_feSetting;  /**< front-end settings */

      const Track* m_track = 0;  /**< mdst track */
      const ExtHit* m_extHit = 0;  /**< extrapolated hit */
      const MCParticle* m_mcParticle = 0;  /**< MC particle */
      const TOPBarHit* m_barHit = 0;  /**< bar hit */
      bool m_valid = false;  /**< true for properly defined track */

      std::vector<SelectedHit> m_selectedHits; /**< selected photon hits from TOPDigits belonging to this slot ID */
      double m_bkgRate = 0; /**< estimated background hit rate */
      std::unordered_multimap<unsigned, const SelectedHit*> m_columnHits; /**< selected hits mapped to pixel columns */

      /** assumed emission points in module local frame */
      mutable std::map<double, TOPTrack::AssumedEmission> m_emissionPoints;

    };


    inline double TOPTrack::getBeta(const Const::ChargedStable& chargedStable) const
    {
      double mass = chargedStable.getMass();
      return m_momentum / sqrt(m_momentum * m_momentum + mass * mass);
    }

    inline double TOPTrack::getTOF(const Const::ChargedStable& chargedStable, double dL) const
    {
      return (m_trackLength + dL) / getBeta(chargedStable) / Const::speedOfLight;
    }


  } // end TOP namespace
} // end Belle2 namespace


