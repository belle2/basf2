/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
#include <string>
#include <utility>

namespace Belle2 {
  namespace Calibration {
    typedef std::pair<int, int> KeyType;
    typedef std::pair<int, int> ExpRun;
    const std::string RUN_RANGE_OBJ_NAME = "RunRange";
    std::string encodeExpRun(const ExpRun& expRun);
    ExpRun decodeExpRun(const std::string& expRunString);
  }
}
