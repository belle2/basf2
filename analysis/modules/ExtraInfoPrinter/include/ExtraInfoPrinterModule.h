/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2018 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Sam Cunliffe                                             *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once
#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>
#include <analysis/dataobjects/EventExtraInfo.h>

namespace Belle2 {
  /**
   * Prints the names of the ExtraInfos for a ParticleList or for the Event
  */
  class ExtraInfoPrinterModule : public Module {
  private:
    bool m_printOnce = true;          /**< print for he first event or for all events? */
    bool m_hasPrinted = false;        /**< internal check if I've already printed */
    std::string m_listName;           /**< name of the ParticleList */
    StoreObjPtr<ParticleList> m_list; /**< the ParticleList itself */
    StoreObjPtr<ParticleExtraInfoMap> m_peem; /**< the map of particles to extra info */
    StoreObjPtr<EventExtraInfo> m_eee; /**< the EventExtraInfo */
  public:
    /** Constructor */
    ExtraInfoPrinterModule();
    /** Initialises module */
    virtual void initialize() override;
    /** Called for each event */
    virtual void event() override;
  };
} // end namespace Belle2
