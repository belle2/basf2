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
