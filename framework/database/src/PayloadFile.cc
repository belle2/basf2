/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/database/PayloadFile.h>
#include <framework/logging/Logger.h>
#include <fstream>

using namespace std;
using namespace Belle2;


string PayloadFile::getContent() const
{
  ifstream file(m_fileName.c_str());
  if (!file.is_open()) {
    B2ERROR("Opening of payload file " << m_fileName << " failed.");
    return "";
  }
  stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}
