/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hidekazu Kakuno                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef L3TRIGGERBASE_H
#define L3TRIGGERBASE_H

#include <cstring>
#include "hlt/hlt/dataobjects/L3Tag.h"

namespace Belle2 {

  //! Base class for a Level3 trigger algorithm
  class L3TriggerBase {

  public:
    //! constructor
    L3TriggerBase() { *m_name = '\0'; }

    //! destructor
    virtual ~L3TriggerBase() {}

    //! preselect events before the fast track and cluster finder
    virtual bool preselect() { return false; }

    //! selecct events after the fast track and cluster finder
    virtual bool select(L3Tag* tag) = 0;

    //! get name of the trigger algorithm
    const char* GetName() const { return m_name; }

  protected:
    //! set name of the trigger algorithm
    void SetName(const char* name) { std::strncpy(m_name, name, 32); }


  private:
    //! name of the trigger algorithm
    char m_name[32];
  };
}

#endif
