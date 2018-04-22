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

#include <math.h>
#include <TVector3.h>
#include <framework/gearbox/Const.h>

namespace Belle2 {

  class Track;
  class ExtHit;
  class MCParticle;
  class TOPBarHit;

  namespace TOP {

    /**
     * Class to hold reconstructed track, interface to fortran.
     * Input to TOPreco.
     */

    class TOPtrack {
    public:
      /**
       * Default constructor
       */
      TOPtrack()
      {}

      /**
       * Constructor with track parameters
       * @param x track spatial position x
       * @param y track spatial position y
       * @param z track spatial position z
       * @param Px track momentum component x
       * @param Py track momentum component y
       * @param Pz track momentum component z
       * @param Tlen track length from IP
       * @param Q track charge
       * @param pdg PDG code (optional)
       */
      TOPtrack(double x, double y, double z, double Px, double Py, double Pz,
               double Tlen, int Q, int pdg = 0);


      /**
       * Constructor from mdst track - isValid() must be checked before using the object
       * @param track mdst track pointer
       * @param chargedStable hypothesis used in mdst track extrapolation (default: pion)
       */
      TOPtrack(const Track* track,
               const Const::ChargedStable& chargedStable = Const::pion);

      /**
       * Constructor from mdst track - isValid() must be checked before using the object
       * @param track mdst track pointer
       * @param moduleID module (=slot) ID
       * @param chargedStable hypothesis used in mdst track extrapolation (default: pion)
       */
      TOPtrack(const Track* track, int moduleID,
               const Const::ChargedStable& chargedStable = Const::pion);

      /**
       * Check if track is properly defined
       * @return true if properly defined
       */
      bool isValid() const {return m_valid;}

      /**
       * Return spatial position
       * @return position
       */
      const TVector3& getPosition() const {return m_position;}

      /**
       * Return momentum vector
       * @return momentum vector
       */
      const TVector3& getMomentum() const {return m_momentum;}

      /**
       * Return position component
       * @return  spatial position x
       */
      double getX() const {return m_position.X();}

      /**
       * Return position component
       * @return  spatial position y
       */
      double getY() const {return m_position.Y();}

      /**
       * Return position component
       * @return  spatial position z
       */
      double getZ() const {return m_position.Z();}

      /**
       * Return momentum component
       * @return  momentum component x
       */
      double getPx() const {return m_momentum.X();}

      /**
       * Return momentum component
       * @return  momentum component y
       */
      double getPy() const {return m_momentum.Y();}

      /**
       * Return momentum component
       * @return  momentum component z
       */
      double getPz() const {return m_momentum.Z();}

      /**
       * Return track length from IP to current position
       * @return track length from IP
       */
      double getTrackLength() const {return m_trackLength;}

      /**
       * Return time-of-flight from IP to current position for given particle mass
       * @param mass particle mass
       * @return time-of-flight
       */
      double getTOF(double mass) const;

      /**
       * Return time-of-flight from IP to current position for given charged stable
       * @param particle charged stable particle
       * @return time-of-flight
       */
      double getTOF(const Const::ChargedStable& particle) const
      {
        return getTOF(particle.getMass());
      }

      /**
       * Set track length from time-of-flight and particle mass
       * @param tof time-of-flight
       * @param mass particle mass
       */
      void setTrackLength(double tof, double mass);

      /**
       * Set track length from time-of-flight and given charged stable
       * @param tof time-of-flight
       * @param particle charged stable particle
       */
      void setTrackLength(double tof, const Const::ChargedStable& particle)
      {
        setTrackLength(tof, particle.getMass());
      }

      /**
       * Return momentum magnitude
       * @return momentum
       */
      double getP() const {return m_momentum.Mag();}

      /**
       * Return momentum polar angle
       * @return momentum polar angle
       */
      double getTheta() const {return m_momentum.Theta();}

      /**
       * Return momentum azimuthal angle
       * @return momentum azimuthal angle
       */
      double getPhi() const {return m_momentum.Phi();}

      /**
       * Return PDG code
       * @return PDG code
       */
      int getPDGcode() const {return m_pdg;}

      /**
       * Return charge
       * @return charge
       */
      int getCharge() const {return m_charge;}

      /**
       * Return internal particle code
       * @return internal particle code: 1=e, 2=mu, 3=pi, 4=K, 5=p, 6=d, 0=unknown
       */
      int getHypID() const;

      /**
       * Return module ID
       * @return module ID, if track hits the bar, otherwise 0
       */
      int getModuleID() const {return m_moduleID;}

      /**
       * Check if toTop() was already called
       * @return true or false
       */
      bool atTop() const {return m_atTop;}

      /**
       * Return mdst track if this track is constructed from it
       * @return mdst track pointer or NULL
       */
      const Track* getTrack() const {return m_track;}

      /**
       * Return extrapolated hit (track entrance to the bar)
       * if this track is constructed from mdst track
       * @return extrapolated hit pointer or NULL
       */
      const ExtHit* getExtHit() const {return m_extHit;}

      /**
       * Return MC particle assigned to this track (if any)
       * @return MC particle pointer or NULL
       */
      const MCParticle* getMCParticle() const {return m_mcParticle;}

      /**
       * Return bar hit of MC particle assigned to this track (if any)
       * @return bar hit pointer or NULL
       */
      const TOPBarHit* getBarHit() const {return m_barHit;}

      /**
       * Propagate track to TOP counter (assuming uniform B field along z)
       * @return module ID, if track hits the bar, otherwise 0
       */
      int toTop();

      /**
       * Smear track
       * @param sig_x sigma in x
       * @param sig_z sigma in z
       * @param sig_theta sigma in theta
       * @param sig_phi sigma in phi
       */
      void smear(double sig_x, double sig_z, double sig_theta, double sig_phi);

      /**
       * Print track parameters to std output
       */
      void dump() const;

    private:

      /**
       * Finds moduleID the track is crossing
       * @return module ID, if track hits the bar, otherwise 0
       */
      int findModule();

      bool m_valid = false;       /**< true for properly defined track */
      TVector3 m_position;        /**< position vector*/
      TVector3 m_momentum;        /**< momentum vector */
      double m_trackLength = 0;   /**< track length from IP to point */
      int m_charge = 0;           /**< charge */
      int m_pdg = 0;              /**< PDG code (optional) */
      bool m_atTop = false;       /**< true, if toTop() called */
      int m_moduleID = 0;         /**< module ID or 0 */
      const Track* m_track = 0;            /**< pointer to mdst track or NULL */
      const ExtHit* m_extHit = 0;          /**< pointer to extrapolated hit or NULL */
      const MCParticle* m_mcParticle = 0;  /**< pointer to MC particle or NULL */
      const TOPBarHit* m_barHit = 0;       /**< pointer to bar hit or NULL */
    };

  } // end TOP namespace
} // end Belle2 namespace


