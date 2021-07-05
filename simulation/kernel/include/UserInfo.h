/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef USERINFO_H_
#define USERINFO_H_

#include <framework/core/FrameworkExceptions.h>
#include <mdst/dataobjects/MCParticleGraph.h>
#include <simulation/dataobjects/MCParticleTrajectory.h>

#include "G4VUserTrackInformation.hh"
#include "G4VUserPrimaryParticleInformation.hh"


namespace Belle2 {

  namespace Simulation {

    //Define exceptions
    /** Exception is thrown if no user information could be found. */
    BELLE2_DEFINE_EXCEPTION(CouldNotFindUserInfo, "No User Information was found !");

    /**
     * UserInfo class which is used to attach additional information to Geant4 particles and tracks.
     * This class is designed as a template based payload class.
     */
    template<class Info, class Payload> class UserInfo: public Info {

    public:

      /**
       * The constructor of the user info class.
       * @param data The data which should be payloaded to the class.
       */
      explicit UserInfo(Payload data) : Info(), m_data(data), m_status(0), m_fraction(1.0), m_trajectory(nullptr) {}

      /**
       * Returns the payloaded data.
       * @return The data added to the class.
       */
      Payload getData() { return m_data; }

      /**
       * Get status of optical photon (used for performance speed-ups)
       * 0 initial
       * 1 prescaled in StackingAction
       * 2 quantum efficiency has been applied
       * @return status
       */
      int getStatus() {return m_status;}

      /**
       * Get optical photon propagation fraction (used for performance speed-ups)
       * status=0: fraction=1
       * status=1: fraction=PhotonFraction as set in FullSim
       * status=2: fraction=quantum efficiency
       * @return fraction
       */
      double getFraction() {return m_fraction;}

      /** Get optional particle trajectory */
      MCParticleTrajectory* getTrajectory() { return m_trajectory; }

      /**
       * Set status of optical photon (used for performance speed-ups)
       * @param status status
       */
      void setStatus(int status) { m_status = status;}

      /**
       * Store optical photon propagation fraction (used for performance speed-ups)
       * @param fraction fraction
       */
      void setFraction(double fraction) { m_fraction = fraction;}

      /** Set the optional particle trajectory */
      void setTrajectory(MCParticleTrajectory* trajectory) { m_trajectory = trajectory; }

      /**
       * Prints information, not implemented in this class.
       */
      virtual void Print() const {;}

      /**
       * Static function to just return UserInformation attached to the obj of type Carrier.
       * Throws an exception of type CouldNotFindUserInfo if no information was found.
       *
       * @param obj Reference to the carrier.
       * @return The data attached to the carrier.
       */
      template<class Carrier> static Payload getInfo(Carrier& obj)
      {
        UserInfo<Info, Payload>* userinfo = dynamic_cast<UserInfo<Info, Payload>*>(obj.GetUserInformation());
        if (!userinfo) throw(CouldNotFindUserInfo());
        return userinfo->getData();
      }

    protected:

      Payload m_data; /**< The data which is payloaded to the specified class.*/

      int m_status;      /**< optical photon: status */
      double m_fraction; /**< optical photon: propagation fraction */
      MCParticleTrajectory* m_trajectory; /**< Particle trajectory, if any */

    };

    typedef UserInfo< G4VUserTrackInformation, MCParticleGraph::GraphParticle& > TrackInfo;
    typedef UserInfo< G4VUserPrimaryParticleInformation, MCParticleGraph::GraphParticle& > ParticleInfo;

  } // end namespace Simulation
} // end namespace Belle2

#endif /* USERINFO_H_ */
