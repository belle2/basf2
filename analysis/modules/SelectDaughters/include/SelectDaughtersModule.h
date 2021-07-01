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
#include <analysis/DecayDescriptor/DecayDescriptor.h>

namespace Belle2 {
  /**
   * Redefine (select) the daughters of a particle
   *
   * useful for recoil analysis   *
   */
  class SelectDaughtersModule : public Module {

  public:
    /** constructor */
    SelectDaughtersModule();
    /** destructor */
    ~SelectDaughtersModule() {}
    /** set up datastore */
    virtual void initialize() override;
    /** process the event */
    virtual void event() override;
  private:
    std::string m_listName;  /**< name of particle list */
    std::string m_decayString;    /**< daughter particles selection */
    DecayDescriptor m_decaydescriptor; /**< Decay descriptor of decays to look for. */
  };
}
