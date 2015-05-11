/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Madlener                                          *
 *                                                                        *
 **************************************************************************/

#pragma once

// framework
#include <framework/core/Module.h>
#include <string>

namespace Belle2 {

  /**
   * Module for generating training samples for supervised training of a MLP
   * Combines the spatial coordinates of two Segments (consisting of 2 SpacePoints each)
   * where both Segments share one hit. Thus a training sample consists of three SpacePoint
   * coordinates and a boolean value that flags if this combination of SpacePoints is part
   * of a track or not.
   *
   * NOTE: this is still in a developement stage!
   */
  class ThreeHitSamplesGeneratorModule : public Module {

  public:
    ThreeHitSamplesGeneratorModule(); /**< Constructor */

    virtual void initialize(); /**< initialize */
    virtual void event(); /**< event */
    virtual void terminate(); /**< terminate */

  protected:

    std::string m_PARAMcontainerName; /**< name of SpacePoint container in StoreArray */

  };
}
