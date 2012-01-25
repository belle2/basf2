#ifndef HLTFRAMEWORK_H
#define HLTFRAMEWORK_H

#include <iostream>

#include <framework/logging/Logger.h>
#include <daq/hlt/HLTDefs.h>

#include <daq/hlt/HLTManager.h>
#include <daq/hlt/HLTProcess.h>
#include <daq/hlt/XMLParser.h>

namespace Belle2 {
  class HLTFramework {
  public:
    HLTFramework(int type);
    ~HLTFramework();

    EHLTStatus init(char* xmlFileName);

  private:
    EHLTNodeType m_type;

    HLTManager* m_hltManager;
    HLTProcess* m_hltProcess;
  };
}

#endif
