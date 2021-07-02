/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
