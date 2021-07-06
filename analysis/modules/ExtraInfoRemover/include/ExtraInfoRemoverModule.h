/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <framework/core/Module.h>
#include <string>

#include <analysis/dataobjects/EventExtraInfo.h>
#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {

  /** This module deletes the extrainfo of each particle in the given ParticleLists.
  If specified (removeEventExtraInfo = True) the EventExtraInfo is removed. */

  class ExtraInfoRemoverModule : public Module {
  private:

    /** Name of the lists */
    std::vector<std::string> m_strParticleLists;

    /** Removes or not EventExtraInfo **/
    bool m_removeEventExtraInfo;

    /** event extra info object pointer */
    StoreObjPtr<EventExtraInfo> m_eventExtraInfo;

  public:
    /** Constructor. */
    ExtraInfoRemoverModule();

    /** Initialises the module.
     */
    virtual void initialize() override;
    /** Method called for each event. */
    virtual void event() override;
  };
} // end namespace Belle2


