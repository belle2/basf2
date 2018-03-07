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
