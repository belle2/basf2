/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <tracking/spacePointCreation/SpacePoint.h>

namespace Belle2 {
  /**
   * Imports Clusters of the silicon detectors and converts them to spacePoints.
   *
   */
  class SpacePointCreatorModule : public Module {

  public:

    /** Constructor */
    SpacePointCreatorModule();

    /** Init the module */
    virtual void initialize();
    /** Show progress */
    virtual void event();
    /** Don't break the terminal */
    virtual void terminate();

  protected:

  };
} // end namespace Belle2
