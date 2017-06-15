#include <calibration/CalibrationAlgorithm.h>
#include <boost/algorithm/string.hpp>

#include <TClonesArray.h>

using namespace std;
using namespace Belle2;

const std::string CalibrationAlgorithm::RUN_RANGE_OBJ_NAME = "__ca_data_range";

CalibrationAlgorithm::EResult CalibrationAlgorithm::execute(vector< Belle2::CalibrationAlgorithm::ExpRun > runs, int iteration)
{
  // Check if we started a new iteration and clear old data
  if (m_iteration != iteration) {
    m_payloads.clear();
    m_iteration = iteration;
  }

  // Let's check that we have the data by accessing an object
  // created by all collector modules by their base class
  StoreObjPtr<CalibRootObj<RunRange>> storeobj(m_prefix + "_" + RUN_RANGE_OBJ_NAME, DataStore::c_Persistent);
  if (!storeobj.isValid()) {
    B2ERROR("Could not access collected data in datastore. " << (m_prefix + "_" + RUN_RANGE_OBJ_NAME) << " object does not exist.");
    return c_Failure;
  }

  if (getRunListFromAllData().empty()) {
    B2ERROR("No collected data.");
    return c_Failure;
  }

  // If no runs are provided, just take all collected
  if (runs.empty())
    runs = getRunListFromAllData();

  std::sort(runs.begin(), runs.end());

  // After this line, the getObject<...>(...) helpers start to work
  m_runs = runs;

  IntervalOfValidity caRange(m_runs[0].first, m_runs[0].second, m_runs[m_runs.size() - 1].first, m_runs[m_runs.size() - 1].second);

  if (getRunListFromAllData() == std::vector<ExpRun>({{ -1, -1}})) {
    // Data collected with granularity=all
    if (m_runs != std::vector<ExpRun>({{ -1, -1}})) {
      B2ERROR("The data is collected with granularity=all (exp=-1,run=-1), but you seem to request calibration for specific runs.");
      // Take the (-1,-1)
      m_runs = getRunListFromAllData();
      caRange = getIovFromData();
    }
    if (getIovFromData().empty()) {
      B2ERROR("No collected data.");
      return c_Failure;
    }
  }

  IntervalOfValidity dataRange = getIovFromData();
  if (dataRange.empty()) {
    B2ERROR("No data collected for selected runs.");
    return c_Failure;
  }

  if (!dataRange.contains(caRange)) {
    B2ERROR("The requested range for calibration is not contained within range of collected data.");
    // TODO: remove runs outside collected data range...?
    B2INFO("If you want to extend the validity range of calibration constants beyond data, you should do it before DB commit manually.");

    // This probably cannot happen until some logic elsewhere is broken - let's have it as a consistency check
    if (!dataRange.overlaps(caRange)) {
      B2ERROR("The calibration range does not even overlap with the collected data.");
      // We should get just c_NotEnoughData or c_Failure all times, so don't start and fail
      return c_Failure;
    }
  }


  return calibrate();
}

vector< CalibrationAlgorithm::ExpRun > CalibrationAlgorithm::string2RunList(string list) const
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

string CalibrationAlgorithm::runList2String(vector< CalibrationAlgorithm::ExpRun >& list) const
{
  std::string str("");
  for (auto run : list) {
    if (str != "")
      str = str + ",";

    str = str + std::to_string(run.first) + "." + std::to_string(run.second);
  }
  return str;
}

string CalibrationAlgorithm::runList2String(CalibrationAlgorithm::ExpRun run) const
{
  std::vector<ExpRun> runlist;
  runlist.push_back(run);

  return runList2String(runlist);
}

IntervalOfValidity CalibrationAlgorithm::getIovFromData()
{
  auto& range = getObject<RunRange>(RUN_RANGE_OBJ_NAME);
  return range.getIntervalOfValidity();
}

void CalibrationAlgorithm::saveCalibration(TObject* data, const string& name, const IntervalOfValidity& iov)
{
  m_payloads.emplace_back(name, data, iov);
}

void CalibrationAlgorithm::saveCalibration(TClonesArray* data, const string& name, const IntervalOfValidity& iov)
{
  m_payloads.emplace_back(name, data, iov);
}

void CalibrationAlgorithm::saveCalibration(TObject* data, const string& name)
{
  if (m_runs.empty())
    return;

  IntervalOfValidity iov;
  IntervalOfValidity caRange(m_runs[0].first, m_runs[0].second, m_runs[m_runs.size() - 1].first, m_runs[m_runs.size() - 1].second);

  if (getRunListFromAllData() == std::vector<ExpRun>({{ -1, -1}})) {
    // For granularity=all, automatic IOV is defined by range of collected data
    iov = getIovFromData();
  } else {
    int expMin = m_runs[0].first;
    int runMin = m_runs[0].second;
    int expMax = m_runs[m_runs.size() - 1].first;
    int runMax = m_runs[m_runs.size() - 1].second;
    iov = IntervalOfValidity(expMin, runMin, expMax, runMax);
  }

  saveCalibration(data, name, iov);
}

void CalibrationAlgorithm::saveCalibration(TClonesArray* data, const string& name)
{
  if (m_runs.empty())
    return;

  IntervalOfValidity iov;
  IntervalOfValidity caRange(m_runs[0].first, m_runs[0].second, m_runs[m_runs.size() - 1].first, m_runs[m_runs.size() - 1].second);

  if (getRunListFromAllData() == std::vector<ExpRun>({{ -1, -1}})) {
    // For granularity=all, automatic IOV is defined by range of collected data
    iov = getIovFromData();
  } else {
    int expMin = m_runs[0].first;
    int runMin = m_runs[0].second;
    int expMax = m_runs[m_runs.size() - 1].first;
    int runMax = m_runs[m_runs.size() - 1].second;
    iov = IntervalOfValidity(expMin, runMin, expMax, runMax);
  }

  saveCalibration(data, name, iov);
}

bool CalibrationAlgorithm::commit()
{
  if (m_payloads.empty())
    return false;
  std::list<Database::DBQuery> payloads = getPayloads();
  B2INFO("Committing " << payloads.size()  << " payloads to database.");
  return Database::Instance().storeData(payloads);
}

bool CalibrationAlgorithm::commit(std::list<Database::DBQuery> payloads)
{
  if (payloads.empty())
    return false;
  return Database::Instance().storeData(payloads);
}

vector< CalibrationAlgorithm::ExpRun > CalibrationAlgorithm::getRunListFromAllData()
{
  string fullName(m_prefix + "_" + RUN_RANGE_OBJ_NAME);
  StoreObjPtr<CalibRootObj<RunRange>> storeobj(fullName, DataStore::c_Persistent);

  std::vector<ExpRun> list;
  if (!storeobj.isValid())
    return list;

  for (auto objAndIov : storeobj->getObjects()) {
    std::vector<ExpRun> sublist = string2RunList(objAndIov.second);
    for (auto exprun : sublist)
      list.push_back(exprun);
  }
  std::sort(list.begin(), list.end());
  list.erase(std::unique(list.begin(), list.end()), list.end());
  return list;
}
