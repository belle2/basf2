#include <boost/algorithm/string.hpp>
#include <boost/python/class.hpp>
#include <boost/python/docstring_options.hpp>
#include <calibration/calibration_algorithms/CalibrationAlgorithmNew.h>
#include <framework/core/PyObjConvUtils.h>
#include <framework/utilities/RegisterPythonModule.h>

#include <TClonesArray.h>

using namespace std;
using namespace Belle2;
using namespace RootIOUtilities;

const std::string CalibrationAlgorithmNew::RUN_RANGE_OBJ_NAME = "__ca_data_range";

CalibrationAlgorithmNew::EResult CalibrationAlgorithmNew::execute(vector< Belle2::CalibrationAlgorithmNew::ExpRun > runs,
    int iteration)
{
  // Check if we started a new iteration and clear old data
  if (m_iteration != iteration) {
    m_payloads.clear();
    m_iteration = iteration;
  }

//  // Let's check that we have the data by accessing an object
//  // created by all collector modules by their base class
//  StoreObjPtr<CalibRootObj<RunRange>> storeobj(m_prefix + "_" + RUN_RANGE_OBJ_NAME, DataStore::c_Persistent);
//  if (!storeobj.isValid()) {
//    B2ERROR("Could not access collected data in datastore. " << (m_prefix + "_" + RUN_RANGE_OBJ_NAME) << " object does not exist.");
//    return c_Failure;
//  }
//
//  if (getRunListFromAllData().empty()) {
//    B2ERROR("No collected data.");
//    return c_Failure;
//  }
//
//  // If no runs are provided, just take all collected
//  if (runs.empty())
//    runs = getRunListFromAllData();
//
//  std::sort(runs.begin(), runs.end());
//
//  // After this line, the getObject<...>(...) helpers start to work
//  m_runs = runs;
//
//  IntervalOfValidity caRange(m_runs[0].first, m_runs[0].second, m_runs[m_runs.size() - 1].first, m_runs[m_runs.size() - 1].second);
//
//  if (getRunListFromAllData() == std::vector<ExpRun>({{ -1, -1}})) {
//    // Data collected with granularity=all
//    if (m_runs != std::vector<ExpRun>({{ -1, -1}})) {
//      B2ERROR("The data is collected with granularity=all (exp=-1,run=-1), but you seem to request calibration for specific runs.");
//      // Take the (-1,-1)
//      m_runs = getRunListFromAllData();
//      caRange = getIovFromData();
//    }
//    if (getIovFromData().empty()) {
//      B2ERROR("No collected data.");
//      return c_Failure;
//    }
//  }
//
//  IntervalOfValidity dataRange = getIovFromData();
//  if (dataRange.empty()) {
//    B2ERROR("No data collected for selected runs.");
//    return c_Failure;
//  }
//
//  if (!dataRange.contains(caRange)) {
//    B2ERROR("The requested range for calibration is not contained within range of collected data.");
//    // TODO: remove runs outside collected data range...?
//    B2INFO("If you want to extend the validity range of calibration constants beyond data, you should do it before DB commit manually.");
//
//    // This probably cannot happen until some logic elsewhere is broken - let's have it as a consistency check
//    if (!dataRange.overlaps(caRange)) {
//      B2ERROR("The calibration range does not even overlap with the collected data.");
//      // We should get just c_NotEnoughData or c_Failure all times, so don't start and fail
//      return c_Failure;
//    }
//  }


  return calibrate();
}

/// Set the input file names used for this algorithm and resolve the wildcards
void CalibrationAlgorithmNew::setInputFileNames(boost::python::list inputFileNames)
{
  m_inputFileNames = expandWordExpansions(PyObjConvUtils::convertPythonObject(inputFileNames, std::vector<std::string>()));
}

/// Get the (wildcard resolved) input file names used for this algorithm
std::vector<std::string> CalibrationAlgorithmNew::getInputFileNames()
{
  return m_inputFileNames;
}

/// Get the (wildcard resolved) input file names used for this algorithm
boost::python::list CalibrationAlgorithmNew::getInputFileNames_Python()
{
  return PyObjConvUtils::convertToPythonObject<std::string>(m_inputFileNames);
}

vector< CalibrationAlgorithmNew::ExpRun > CalibrationAlgorithmNew::string2RunList(string list) const
{
  std::vector<ExpRun> result;

  if (list == "")
    return result;

  std::vector<std::string> runs;
  boost::algorithm::split(runs, list, boost::is_any_of(","));

  for (auto exprunstr : runs) {
    std::vector<std::string> exprun;
    boost::algorithm::split(exprun, exprunstr, boost::is_any_of("."));
    if (exprun.size() != 2)
      B2FATAL("Error in parsing object validity");
    result.push_back(std::make_pair(std::stoi(exprun[0]), std::stoi(exprun[1])));
  }
  return result;
}

string CalibrationAlgorithmNew::runList2String(vector< CalibrationAlgorithmNew::ExpRun >& list) const
{
  std::string str("");
  for (auto run : list) {
    if (str != "")
      str = str + ",";

    str = str + std::to_string(run.first) + "." + std::to_string(run.second);
  }
  return str;
}

string CalibrationAlgorithmNew::runList2String(CalibrationAlgorithmNew::ExpRun run) const
{
  std::vector<ExpRun> runlist;
  runlist.push_back(run);

  return runList2String(runlist);
}

//IntervalOfValidity CalibrationAlgorithmNew::getIovFromData()
//{
//  auto& range = getObject<RunRange>(RUN_RANGE_OBJ_NAME);
//  return range.getIntervalOfValidity();
//}

void CalibrationAlgorithmNew::saveCalibration(TObject* data, const string& name, const IntervalOfValidity& iov)
{
  m_payloads.emplace_back(name, data, iov);
}

void CalibrationAlgorithmNew::saveCalibration(TClonesArray* data, const string& name, const IntervalOfValidity& iov)
{
  m_payloads.emplace_back(name, data, iov);
}

void CalibrationAlgorithmNew::saveCalibration(TObject* data, const string& name)
{
  if (m_runs.empty())
    return;

  IntervalOfValidity iov;
  IntervalOfValidity caRange(m_runs[0].first, m_runs[0].second, m_runs[m_runs.size() - 1].first, m_runs[m_runs.size() - 1].second);

//  if (getRunListFromAllData() == std::vector<ExpRun>({{ -1, -1}})) {
//    // For granularity=all, automatic IOV is defined by range of collected data
//    iov = getIovFromData();
//  } else {
//    int expMin = m_runs[0].first;
//    int runMin = m_runs[0].second;
//    int expMax = m_runs[m_runs.size() - 1].first;
//    int runMax = m_runs[m_runs.size() - 1].second;
//    iov = IntervalOfValidity(expMin, runMin, expMax, runMax);
//  }

  saveCalibration(data, name, iov);
}

void CalibrationAlgorithmNew::saveCalibration(TClonesArray* data, const string& name)
{
  if (m_runs.empty())
    return;

  IntervalOfValidity iov;
  IntervalOfValidity caRange(m_runs[0].first, m_runs[0].second, m_runs[m_runs.size() - 1].first, m_runs[m_runs.size() - 1].second);

//  if (getRunListFromAllData() == std::vector<ExpRun>({{ -1, -1}})) {
//    // For granularity=all, automatic IOV is defined by range of collected data
//    iov = getIovFromData();
//  } else {
//    int expMin = m_runs[0].first;
//    int runMin = m_runs[0].second;
//    int expMax = m_runs[m_runs.size() - 1].first;
//    int runMax = m_runs[m_runs.size() - 1].second;
//    iov = IntervalOfValidity(expMin, runMin, expMax, runMax);
//  }

  saveCalibration(data, name, iov);
}

bool CalibrationAlgorithmNew::commit()
{
  if (m_payloads.empty())
    return false;
  std::list<Database::DBQuery> payloads = getPayloads();
  B2INFO("Committing " << payloads.size()  << " payloads to database.");
  return Database::Instance().storeData(payloads);
}

bool CalibrationAlgorithmNew::commit(std::list<Database::DBQuery> payloads)
{
  if (payloads.empty())
    return false;
  return Database::Instance().storeData(payloads);
}

//vector< CalibrationAlgorithmNew::ExpRun > CalibrationAlgorithmNew::getRunListFromAllData()
//{
//  string fullName(m_prefix + "_" + RUN_RANGE_OBJ_NAME);
//  StoreObjPtr<CalibRootObj<RunRange>> storeobj(fullName, DataStore::c_Persistent);
//
//  std::vector<ExpRun> list;
//  if (!storeobj.isValid())
//    return list;
//
//  for (auto objAndIov : storeobj->getObjects()) {
//    std::vector<ExpRun> sublist = string2RunList(objAndIov.second);
//    for (auto exprun : sublist)
//      list.push_back(exprun);
//  }
//  std::sort(list.begin(), list.end());
//  list.erase(std::unique(list.begin(), list.end()), list.end());
//  return list;
//}

void CalibrationAlgorithmNew::exposePythonAPI()
{
  using namespace boost::python;
  docstring_options options(true, true, false); //userdef, py sigs, c++ sigs

  class_<CalibrationAlgorithmNew>("CalibrationAlgorithmNew", init<std::string>())
  .def("setInputFileNames", &CalibrationAlgorithmNew::setInputFileNames)
  .def("getInputFileNames", &CalibrationAlgorithmNew::getInputFileNames_Python);
}

//-----------------------------------
//   Define the pybasf2 python module
//-----------------------------------
BOOST_PYTHON_MODULE(calibration_algorithms)
{
  CalibrationAlgorithmNew::exposePythonAPI();
}

REGISTER_PYTHON_MODULE(calibration_algorithms)
