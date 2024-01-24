/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {
  /**
   * Convert the IpProfile and Benergy to BeamParameters, BeamSpot,
   * CollisionInvariantMass and CollisionBoostVector.
   *
   * This module is supposed to run with one event per run in
   */
  class B2BIIConvertBeamParamsModule : public Module {

  public:
    /** Create parameters */
    B2BIIConvertBeamParamsModule();

    /** Initialize phanter banks */
    void initialize() override;

    /** Set run info in panther and load IPProfile/Benergy and convert the
     * values to payloads */
    void beginRun() override;

  private:

    /** mcFlag to use when getting belle database content */
    int m_mcFlag{1};

    /** Where to store information about runs without beam energy information */
    std::string m_missingBenergy{"missing_benergy.txt"};

    /** Where to store information about runs without IP profile information */
    std::string m_missingIp{"missing_ip.txt"};

    /** Smear energy when generating initial events. */
    bool m_SmearEnergy;

    /** Smear beam direction when generating initial events. */
    bool m_SmearDirection;

    /** Smear vertex position when generating initial events. */
    bool m_SmearVertex;

    /** Generate events in CMS, not lab system. */
    bool m_GenerateCMS = false;

    /** Store the BeamParameters payloads in the local database. */
    bool m_storeBeamParameters = true;

    /** Store the CollisionAxisCMS payloads in the local database. */
    bool m_storeCollisionAxisCMS = true;

    /** Store the CollisionInvariantMass payloads in the local database. */
    bool m_storeCollisionInvariantMass = true;

    /** Store the CollisionBoostVector payloads in the local database. */
    bool m_storeCollisionBoostVector = true;

    /** Store the BeamSpot payloads in the local database. */
    bool m_storeBeamSpot = true;

    /** Event metadata */
    StoreObjPtr<EventMetaData> m_event;

  };
}
