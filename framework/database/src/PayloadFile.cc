/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
