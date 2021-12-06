/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

namespace Belle2 {
  /** Internal module used by Path.add_independent_path(). Don't use it directly. */
  class MergeDataStoreModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the description, the properties and the parameters of the module.
     */
    MergeDataStoreModule();
    /** setter for Path. */
    void init(const std::string& to, bool doCopy, const std::vector<std::string>& mergeBack,  bool eventMixing);

    ~MergeDataStoreModule();

    virtual void initialize() override;
    virtual void beginRun() override;
    virtual void endRun() override;
    virtual void event() override;
    virtual void terminate() override;

  private:
    std::string m_from; /**< active DataStore ID before this module. */
    std::string m_to; /**< active DataStore ID after this module. */
    bool m_createNew; /**< create new DataStore */
    bool m_eventMixing; /**< do event mixing (merge each event with each one) */

    /** list of obj/arrays (of event durability) that should be merged with m_to. */
    std::vector<std::string> m_mergeBack;
  };
}
