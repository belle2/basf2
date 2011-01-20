/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Andreas Moll                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef USERINFO_H_
#define USERINFO_H_

#include <framework/core/FrameworkExceptions.h>
#include <generators/dataobjects/MCParticleGraph.h>

#include "G4VUserTrackInformation.hh"
#include "G4VUserPrimaryParticleInformation.hh"


namespace Belle2 {

  namespace Simulation {

    /**
     * UserInfo class which is used to attach additional information to Geant4 particles and tracks.
     * This class is designed as a template based payload class.
     */
    template<class Info, class Payload> class UserInfo: public Info {

    public:

      //Define exceptions
      /** Exception is thrown if no user information could be found. */
      BELLE2_DEFINE_EXCEPTION(CouldNotFindUserInfo, "No User Information was found !");

      /**
       * The constructor of the user info class.
       * @param data The data which should be payloaded to the class.
       */
      UserInfo(Payload data) : Info(), m_data(data) {}

      /**
       * Returns the payloaded data.
       * @return The data added to the class.
       */
      Payload getData() { return m_data; }

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
      template<class Carrier> static Payload getInfo(Carrier& obj) {
        UserInfo<Info, Payload> *userinfo = dynamic_cast<UserInfo<Info, Payload>*>(obj.GetUserInformation());
        if (!userinfo) throw(CouldNotFindUserInfo());
        return userinfo->getData();
      }

    protected:

      Payload m_data; /**< The data which is payloaded to the specified class.*/
    };

    typedef UserInfo< G4VUserTrackInformation, MCParticleGraph::GraphParticle& > TrackInfo;
    typedef UserInfo< G4VUserPrimaryParticleInformation, MCParticleGraph::GraphParticle& > ParticleInfo;

  } // end namespace Simulation
} // end namespace Belle2

#endif /* USERINFO_H_ */
