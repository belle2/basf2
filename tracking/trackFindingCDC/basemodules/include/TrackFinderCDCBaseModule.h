/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>
#include <framework/core/ModuleParamList.h>

#include <vector>
#include <string>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;
  }

  /**
   *  A base class for track finders in the CDC.
   *
   *  This module defines / handles the inputs and outputs to the DataStore and
   *  the respective StoreArray names.
   */
  class TrackFinderCDCBaseModule : public Module {
  public:

    /// Constructor of the module. Setting up parameters and description.
    TrackFinderCDCBaseModule();

    /// Initialize the Module before event processing
    void initialize() override;

    /// Processes the event and generates track candidates
    void event() override;

    /// Generates the tracks into the output argument. To be overriden in a concrete subclass.
    virtual void generate(std::vector<TrackFindingCDC::CDCTrack>& tracks) = 0;

  protected:
    /// Parameter: Name of the output StoreObjPtr of the tracks generated within this module.
    std::string m_param_tracksStoreObjName = "CDCTrackVector";

    /// Parameter: Flag to use the CDCTracks in the given StoreObjPtr as input and output of the module.
    bool m_param_tracksStoreObjNameIsInput = false;
  };
}
