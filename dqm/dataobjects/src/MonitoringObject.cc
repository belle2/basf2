/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2015 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <dqm/dataobjects/MonitoringObject.h>

#include <framework/utilities/KeyValuePrinter.h>

#include <iostream>
#include <sstream>

using namespace std;
using namespace Belle2;

void MonitoringObject::print() const
{

  KeyValuePrinter printVar(false);
  printVar.put("Name", GetName());
  for (const auto& var : m_vars) {
    printVar.put(var.first, var.second);
  }
  for (const auto& var : m_strVars) {
    printVar.put(var.first, var.second);
  }

  KeyValuePrinter printCanv(false);
  for (const auto& canv : m_Canvases) {
    printCanv.put(canv->GetName(), canv->GetTitle());
  }

  std::cout << "=== MonitoringObject ===\n";
  std::cout << printVar.string();
  std::cout << "\n List of canvases: \n";
  std::cout << printCanv.string();
  std::cout << "========================\n";
}
