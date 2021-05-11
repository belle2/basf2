/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Roca                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <analysis/dataobjects/FlavorTaggerInfo.h>
#include <analysis/dataobjects/RestOfEvent.h>
#include <analysis/dataobjects/FlavorTaggerInfoMap.h>

#include <framework/datastore/StoreArray.h>

namespace Belle2 {

  /**
   * Creates for each Particle and RestOfEvent object in the DataStore a
   * FlavorTaggerInfo dataobject and makes BASF2 relation between them.
   * NOTE: this module DOES NOT fill the FlavorTaggerInfo dataobject, but only
   * initializes it so can be filled during the run of FlavorTagger.py
   */
  class FlavorTaggerInfoBuilderModule : public Module {

  public:

    /** constructor */
    FlavorTaggerInfoBuilderModule();
    /** initialize the module (setup the data store) */
    virtual void initialize() override;
    /** process event */
    virtual void event() override;

  private:

    StoreArray<RestOfEvent> m_roes; /**< StoreArray of ROEs */
    StoreArray<FlavorTaggerInfo> m_flavorTaggerInfos; /**< StoreArray of FlavorTaggerInfos */
    StoreArray<FlavorTaggerInfoMap> m_flavorTaggerInfoMaps; /**< StoreArray of FlavorTaggerInfoMaps */
  };
}

