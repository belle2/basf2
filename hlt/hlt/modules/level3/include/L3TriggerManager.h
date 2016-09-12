/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hidekazu Kakuno                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef L3TRIGGERMANAGER_H
#define L3TRIGGERMANAGER_H

#include "skim/hlt/modules/level3/FTList.h"

namespace Belle2 {
  class L3TriggerBase;

  //! Manage Level3 trigger algorithms
  class L3TriggerManager {

  public:
    L3TriggerManager() : m_saveData(0) {}
    virtual ~L3TriggerManager() { m_triggers.deleteAll(); }

  protected:
    //! add trigger
    void addTrigger(L3TriggerBase* t) { m_triggers.append(t); }

    //! function to be called in Module::initialize()
    void initializeTriggerManager();

    //! preselect events
    bool preselect();

    //! select events
    bool select();

    //! print information
    void printInformation();

    //! data members
    int m_saveData;
  private:

    //! data members
    FTList<L3TriggerBase*> m_triggers;
  };
}

#endif
