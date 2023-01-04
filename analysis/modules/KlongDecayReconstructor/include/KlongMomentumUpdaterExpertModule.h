/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

// framework - DataStore
#include <framework/datastore/StoreObjPtr.h>

// dataobjects
#include <analysis/dataobjects/ParticleList.h>

#include <string>

namespace Belle2 {

  /**
   * Calculate and update the two body B decay kinematics
   */
  class KlongMomentumUpdaterExpertModule : public Module {

  public:

    /**
     * Constructor
     */
    KlongMomentumUpdaterExpertModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Event processor.
     */
    virtual void event() override;


  private:

    std::string m_listName;  /**< name of the input ParticleList */
    StoreObjPtr<ParticleList> m_plist; /**< the input ParticleList. */

    bool m_writeOut;  /**< toggle output particle list btw. transient/writeOut */
  };

} // Belle2 namespace
