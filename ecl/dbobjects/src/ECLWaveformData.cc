/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <ecl/dbobjects/ECLWaveformData.h>

/* Basf2 headers. */
#include <framework/logging/Logger.h>

/* C++ headers. */
#include <iostream>
#include <string>

using namespace std;

void Belle2::ECLWaveformData::print() const
{
  string line;
  for (size_t i = 0; i < c_nElements; ++ i) {
    line += to_string(m_matrixElement[i]);
    line += ' ';
  }
  B2INFO(line);
}
