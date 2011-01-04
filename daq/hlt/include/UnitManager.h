/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef UNITMANAGER_H
#define UNITMANAGER_H

#include <iostream>
#include <vector>

#include <framework/logging/Logger.h>

#include <daq/hlt/HLTDefs.h>
#include <daq/hlt/UnitInfo.h>
#include <daq/hlt/NodeInfo.h>
#include <daq/hlt/NodeManager.h>

namespace Belle2 {

  //! UnitManager class
  /*! This class is a manager for units in HLT farm
  */
  class UnitManager {
  public:
    //! Constructor
    UnitManager(UnitInfo& unit);
    //! Destructor
    ~UnitManager(void);

    //! Initializing the unit manager by referring unit information
    EStatus init(UnitInfo& unit);

    void broadCasting(void);

    //! Print information of the current unit (only for internal testing)
    void Print(void);

  protected:
    NodeInfo* buildNodeInfo(const std::string type, const int nodeNo, UnitInfo& unit);
    //! Initializing an event separator
    EStatus initEventSeparator(UnitInfo& unit);
    //! Initializing worker nodes
    EStatus initWorkerNode(UnitInfo& unit);
    //! Initializing an event merger
    EStatus initEventMerger(UnitInfo& unit);

  private:
    int m_unitNo;                               /*!< Unique number of this unit */
    int m_WNs;                                  /*!< Number of worker nodes in this unit */

    NodeManager* m_eventSeparator;              /*!< Pointer to event separator manager */
    NodeManager* m_eventMerger;                 /*!< Pointer to event merger manager */
    std::vector<NodeManager*> m_workerNodes;    /*!< Vector container of pointers to worker node managers */
  };
}


#endif
