/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
