/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EVENTPROCESS_H
#define EVENTPROCESS_H

#include <string>
#include <vector>

namespace Belle2 {

  class EventProcess {
  public:
    EventProcess(void);
    EventProcess(std::vector<std::string> inBuffer, std::vector<std::string> outBuffer);
    ~EventProcess(void);

  private:
    std::vector<std::string> m_inBuffer;
    std::vector<std::string> m_outBuffer;
  };
}

#endif
