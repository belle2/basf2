/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2018 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
    /** Where to store information about runs without ipprofile information */
    std::string m_missing{"missing.txt"};
    /** Event metadata */
    StoreObjPtr<EventMetaData> m_event;
  };
}
