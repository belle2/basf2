/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hidekazu Kakuno                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef LEVEL3MODULE_H
#define LEVEL3MODULE_H

#include <framework/core/Module.h>
#include "skim/hlt/modules/level3/L3TrackTrigger.h"
#include "skim/hlt/modules/level3/L3TriggerManager.h"
#include "skim/hlt/modules/level3/FTList.h"

namespace Belle2 {

  class FTFinder;
  class FCFinder;
  class Level3Module : public L3TriggerManager, public Module {

  public:
    /** Constructor, for setting module description and parameters. */
    Level3Module();

    /** Use to clean up anything you created in the constructor. */
    virtual ~Level3Module();

    virtual void initialize();

    virtual void beginRun();

    virtual void event();

    virtual void endRun() {}

    virtual void terminate();

  private:
    //bool preselect();
    void reconstruct();
    //bool select();
    //void append(L3TriggerBase* t){ m_triggers.append(t); }

    //define your own data members here
    FTFinder& m_tFinder;
    FCFinder& m_cFinder;
    //double m_drCut;
    //double m_dzCut;
    //double m_ptCut;
    double m_EsCut;
    double m_EcCut;
    //int m_minNGoodTrks;
    //double m_minEnergy;
    int m_findEvtVtx;
    //FTList<L3TriggerBase*> m_triggers;
  };
}

#endif
