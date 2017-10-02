#include <set>
#include <boost/algorithm/string.hpp>
#include <boost/python/class.hpp>
#include <boost/python/docstring_options.hpp>
#include <calibration/CalibrationAlgorithmNew.h>
#include <framework/core/PyObjConvUtils.h>
#include <framework/utilities/RegisterPythonModule.h>

#include <TClonesArray.h>

using namespace std;
using namespace Belle2;
using namespace RootIOUtilities;
namespace fs = boost::filesystem;

const string CalibrationAlgorithmNew::RUN_RANGE_OBJ_NAME = "__ca_data_range";

/// Checks if the PyObject can be converted to ExpRun
bool CalibrationAlgorithmNew::checkPyExpRun(PyObject* pyObj)
{
  // Is it a sequence?
  if (PySequence_Check(pyObj)) {
    Py_ssize_t nObj = PySequence_Length(pyObj);
    // Does it have 2 objects in it?
    if (nObj != 2) {
      B2DEBUG(100, "ExpRun was a Python sequence which didn't have exactly 2 entries!");
      return false;
    }
    long value1, value2;
    PyObject* item1, *item2;
    item1 = PySequence_GetItem(pyObj, 0);
    item2 = PySequence_GetItem(pyObj, 1);
    // Did the GetItem work?
    if ((item1 == NULL) || (item2 == NULL)) {
      B2DEBUG(100, "A PyObject pointer was NULL in the sequence");
      return false;
    }
    // Are they longs?
    if (PyLong_Check(item1) && PyLong_Check(item2)) {
      value1 = PyLong_AsLong(item1);
      value2 = PyLong_AsLong(item2);
      if (((value1 == -1) || (value2 == -1)) && PyErr_Occurred()) {
        B2DEBUG(100, "An error occurred while converting the PyLong to long");
        return false;
      }
    } else {
      B2DEBUG(100, "One or more of the PyObjects in the ExpRun wasn't a long");
      return false;
    }
    // Make sure to kill off the reference GetItem gave us responsibility for
    Py_DECREF(item1);
    Py_DECREF(item2);
  } else {
    B2DEBUG(100, "ExpRun was not a Python sequence.");
    return false;
  }
  return true;
}

/// Converts the PyObject to an ExpRun. We've preoviously checked the object so this assumes a lot about the PyObject
CalibrationAlgorithmNew::ExpRun CalibrationAlgorithmNew::convertPyExpRun(PyObject* pyObj)
{
  ExpRun expRun;
  PyObject* itemExp, *itemRun;
  itemExp = PySequence_GetItem(pyObj, 0);
  itemRun = PySequence_GetItem(pyObj, 1);
  expRun.first = PyLong_AsLong(itemExp);
  Py_DECREF(itemExp);
  expRun.second = PyLong_AsLong(itemRun);
  Py_DECREF(itemRun);
  return expRun;
}

CalibrationAlgorithmNew::EResult CalibrationAlgorithmNew::execute(PyObject* runs, int iteration)
{
  B2DEBUG(100, "Running execute() using Python Object as input argument");
  vector<ExpRun> vecRuns;
  // Is it a list?
  if (PySequence_Check(runs)) {
    boost::python::handle<> handle(boost::python::borrowed(runs));
    const boost::python::list listRuns(handle);

    int nList = boost::python::len(listRuns);
    for (int iList = 0; iList < nList; ++iList) {
      boost::python::object pyExpRun(listRuns[iList]);
      if (!checkPyExpRun(pyExpRun.ptr())) {
        B2ERROR("Received Python ExpRuns couldn't be converted to C++");
        return c_Failure;
      } else {
        vecRuns.push_back(convertPyExpRun(pyExpRun.ptr()));
      }
    }
  } else {
    B2ERROR("Tried to set the input runs but we didn't receive a Python sequence object (list,tuple).");
    return c_Failure;
  }
  return execute(vecRuns, iteration);
}

CalibrationAlgorithmNew::EResult CalibrationAlgorithmNew::execute(vector<ExpRun> runs, int iteration)
{
  // Check if we started a new iteration and clear old data
  if (m_iteration != iteration) {
    m_payloads.clear();
    m_iteration = iteration;
  }
  // Did we receive runs to execute over explicitly?
  if (!(runs.empty())) {
    for (auto expRun : runs) {
      B2DEBUG(100, "(" << expRun.first << ", " << expRun.second << ")");
    }
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
void CalibrationAlgorithmNew::setInputFileNames(PyObject* inputFileNames)
{
  // The reasoning for this very 'manual' approach to extending the Python interface
  // (instead of using boost::python) is down to my fear of putting off final users.
  // I didn't want users that inherit from this class to be forced to use boost and
  // to have to define a new python module just to use the CAF. A derived class from
  // from a boost exposed class would need to have its own boost python module definition
  // to allow access from a steering file and to the base class functions.
  // I also couldn't be bothered to write a full framework to get around the issue in a similar
  // way to Module()...maybe there's an easy way.
  //
  // But this way we can allow people to continue using their ROOT implemented classes and inherit
  // easily from this one. But add in a few helper functions that work with Python objects
  // created in their steering file i.e. instead of being forced to use STL objects as input
  // to the algorithm.
  if (PyList_Check(inputFileNames)) {
    boost::python::handle<> handle(boost::python::borrowed(inputFileNames));
    boost::python::list listInputFileNames(handle);
    auto vecInputFileNames = PyObjConvUtils::convertPythonObject(listInputFileNames, vector<string>());
    setInputFileNames(vecInputFileNames);
  } else {
    B2ERROR("Tried to set the input files but we didn't receive a Python list.");
  }
}

/// Set the input file names used for this algorithm and resolve the wildcards
void CalibrationAlgorithmNew::setInputFileNames(vector<string> inputFileNames)
{
  // A lot of code below is tweaked from RootInputModule::initialize,
  // since we're basically copying the functionality anyway.
  if (inputFileNames.empty()) {
    B2WARNING("You have called setInputFileNames() with an empty list. Did you mean to do that?");
    return;
  }
  auto tmpInputFileNames = expandWordExpansions(inputFileNames);

  // We'll use a set to enforce unique file paths as we check them
  set<string> setInputFileNames;
  // Check that files exist and convert to absolute paths
  for (auto path : tmpInputFileNames) {
    string fullPath = fs::absolute(path).string();
    if (fs::exists(fullPath)) {
      setInputFileNames.insert(fs::canonical(fullPath).string());
    } else {
      B2WARNING("Couldn't find the file " << path);
    }
  }

  if (setInputFileNames.empty()) {
    B2WARNING("No valid files specified!");
    return;
  }

  //Open TFile to check they can be accessed by ROOT
  TDirectory* dir = gDirectory;
  for (const string& fileName : setInputFileNames) {
    unique_ptr<TFile> f;
    try {
      f.reset(TFile::Open(fileName.c_str(), "READ"));
    } catch (logic_error&) {
      //this might happen for ~invaliduser/foo.root
      //actually undefined behaviour per standard, reported as ROOT-8490 in JIRA
    }
    if (!f || !f->IsOpen()) {
      B2FATAL("Couldn't open input file " + fileName);
    }
  }
  dir->cd();

  // Copy the entries of the set to a vector
  m_inputFileNames = vector<string>(setInputFileNames.begin(), setInputFileNames.end());
}

PyObject* CalibrationAlgorithmNew::getInputFileNames()
{
  PyObject* objInputFileNames = PyList_New(m_inputFileNames.size());
  for (size_t i = 0; i < m_inputFileNames.size(); ++i) {
    PyList_SetItem(objInputFileNames, i, Py_BuildValue("s", m_inputFileNames[i].c_str()));
  }
  return objInputFileNames;
}

vector<CalibrationAlgorithmNew::ExpRun> CalibrationAlgorithmNew::string2RunList(string list) const
{
  vector<ExpRun> result;

  if (list == "")
    return result;

  vector<string> runs;
  boost::algorithm::split(runs, list, boost::is_any_of(","));

  for (auto exprunstr : runs) {
    vector<string> exprun;
    boost::algorithm::split(exprun, exprunstr, boost::is_any_of("."));
    if (exprun.size() != 2)
      B2FATAL("Error in parsing object validity");
    result.push_back(make_pair(stoi(exprun[0]), stoi(exprun[1])));
  }
  return result;
}

string CalibrationAlgorithmNew::runList2String(vector<ExpRun>& list) const
{
  string str("");
  for (auto run : list) {
    if (str != "")
      str = str + ",";

    str = str + to_string(run.first) + "." + to_string(run.second);
  }
  return str;
}

string CalibrationAlgorithmNew::runList2String(ExpRun run) const
{
  vector<ExpRun> runlist;
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
  list<Database::DBQuery> payloads = getPayloads();
  B2INFO("Committing " << payloads.size()  << " payloads to database.");
  return Database::Instance().storeData(payloads);
}

bool CalibrationAlgorithmNew::commit(list<Database::DBQuery> payloads)
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
