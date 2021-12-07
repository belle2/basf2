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
  /** Internal module used by Path.add_independent_merge_path(). Don't use it directly. */
  class SteerRootInputModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the description, the properties and the parameters of the module.
     */
    SteerRootInputModule();

    /** setter for Path. */
    void init(bool eventMixing);

    ~SteerRootInputModule();

    virtual void initialize() override;
    virtual void event() override;

  private:

    /** do event mixing (merge each event of main path with each event of independent path) */
    bool m_eventMixing;

    /** these events should be processed next {main path, independent path} */
    std::pair<long, long> m_nextEntries = {0, 0};

  };
}
