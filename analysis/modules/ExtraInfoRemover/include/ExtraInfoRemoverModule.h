/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Fernando Abudinen                                        *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once
#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <analysis/dataobjects/Particle.h>
#include <string>


namespace Belle2 {

  /** This module deletes the extrainfo of each particle in the given ParticleLists.
  If specified (removeEventExtraInfo = True) the EventExtraInfo is removed. */

  class ExtraInfoRemoverModule : public Module {
  private:

    /** Name of the lists */
    std::vector<std::string> m_strParticleLists;

    /** Removes or not EventExtraInfo **/
    bool m_removeEventExtraInfo;

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


