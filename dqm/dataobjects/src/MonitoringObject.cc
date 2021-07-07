/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
