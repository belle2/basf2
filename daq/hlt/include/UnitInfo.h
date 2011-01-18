/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef UNITINFO_H
#define UNITINFO_H

#include <string>
#include <vector>

#include <framework/logging/Logger.h>

namespace Belle2 {

  class UnitInfo {
  public:
    UnitInfo(void);
    UnitInfo(const int unitNo);
    virtual ~UnitInfo(void);

    void eventSeparator(const std::string ES);
    void workerNodes(const std::string WN);
    void workerNodes(const std::vector<std::string> WNs);
    void eventMerger(const std::string EM);
    void manager(const std::string MAN);

    std::string eventSeparator(void);
    std::vector<std::string> workerNodes(void);
    std::string eventMerger(void);
    std::string manager(void);

    int unitNo(void);
    char* steering(void);

    void Print(void);

  private:
    int m_unitNo;
    std::string m_manager;
    std::string m_eventSeparator;
    std::vector<std::string> m_workerNodes;
    std::string m_eventMerger;
    char* m_steeringName;
  };
}

#endif
