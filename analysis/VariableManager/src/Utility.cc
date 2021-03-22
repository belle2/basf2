/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sam Cunliffe                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/VariableManager/Utility.h>

namespace Belle2 {
  namespace Variable {

    bool isCounterVariable(const std::string& variableName)
    {
      if (variableName == "expNum") return true;
      else if (variableName == "runNum") return true;
      else if (variableName == "evtNum") return true;
      else if (variableName == "productionIdentifier") return true;
      return false;
    }
  }
}
