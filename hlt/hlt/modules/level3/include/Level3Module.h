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
#include "framework/datastore/StoreArray.h"
#include "hlt/hlt/dataobjects/L3Track.h"
#include "hlt/hlt/dataobjects/L3Cluster.h"
#include "hlt/hlt/modules/level3/L3TrackTrigger.h"
#include "hlt/hlt/modules/level3/L3TriggerManager.h"
#include "hlt/hlt/modules/level3/FTList.h"

namespace Belle2 {

  class FTFinder;
  class FCFinder;

  /* The Level-3 software trigger */
  class Level3Module : public L3TriggerManager, public Module {

  public:

    /** Constructor, for setting module description and parameters. */
    Level3Module();

    /** Use to clean up anything you created in the constructor. */
    virtual ~Level3Module();

    /** initialize routine **/
    virtual void initialize();

    /** begin run routine **/
    virtual void beginRun();

    /** initialize routine **/
    virtual void event();

    /** end run routine **/
    virtual void endRun() {}

    /** terminate routine **/
    virtual void terminate();

  private:
    StoreArray<L3Track> m_l3Track; /** Array for L3Track */
    StoreArray<L3Cluster> m_l3Cluster; /** Array for L3Cluster */

    /** L3 reconstruction routine **/
    void reconstruct();

    //! pointer to the fast track finder
    FTFinder& m_tFinder;

    //! pointer to the fast cluster finder
    FCFinder& m_cFinder;

    //! cut parameter of seed energy for FCFinder
    double m_EsCut;

    //! cut parameter of cluster energy for FCFinder
    double m_EcCut;

    //! =1: perform vertex finding at FTFinder
    int m_findEvtVtx;
  };
}

#endif
