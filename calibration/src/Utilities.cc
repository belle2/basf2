/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <calibration/Utilities.h>

#include <vector>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

using namespace std;

namespace Belle2 {
  namespace Calibration {
    string encodeExpRun(const ExpRun& expRun)
    {
      return to_string(expRun.first) + "." + to_string(expRun.second);
    }

    ExpRun decodeExpRun(const string& expRunString)
    {
      vector<string> strs;
      boost::split(strs, expRunString, boost::is_any_of("."));
      return make_pair(stoi(strs[0]), stoi(strs[1]));
    }
  }
}
