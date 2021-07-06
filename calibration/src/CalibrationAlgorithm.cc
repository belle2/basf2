/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <set>
#include <utility>
#include <boost/algorithm/string.hpp>
#include <boost/python.hpp>
#include <boost/python/list.hpp>
#include <boost/filesystem.hpp>
#include <TChain.h>
#include <calibration/CalibrationAlgorithm.h>
#include <framework/logging/Logger.h>
#include <framework/core/PyObjConvUtils.h>
#include <framework/io/RootIOUtilities.h>

using namespace Belle2;
using namespace std;
using namespace Calibration;
namespace fs = boost::filesystem;

const ExpRun CalibrationAlgorithm::m_allExpRun = make_pair(-1, -1);

/// Checks if the PyObject can be converted to ExpRun
bool CalibrationAlgorithm::checkPyExpRun(PyObject* pyObj)
{
  // Is it a sequence?
  if (PySequence_Check(pyObj)) {
    Py_ssize_t nObj = PySequence_Length(pyObj);
    // Does it have 2 objects in it?
    if (nObj != 2) {
      B2DEBUG(29, "ExpRun was a Python sequence which didn't have exactly 2 entries!");
      return false;
    }
    PyObject* item1, *item2;
    item1 = PySequence_GetItem(pyObj, 0);
    item2 = PySequence_GetItem(pyObj, 1);
    // Did the GetItem work?
    if ((item1 == NULL) || (item2 == NULL)) {
      B2DEBUG(29, "A PyObject pointer was NULL in the sequence");
      return false;
    }
    // Are they longs?
    if (PyLong_Check(item1) && PyLong_Check(item2)) {
      long value1, value2;
      value1 = PyLong_AsLong(item1);
      value2 = PyLong_AsLong(item2);
      if (((value1 == -1) || (value2 == -1)) && PyErr_Occurred()) {
        B2DEBUG(29, "An error occurred while converting the PyLong to long");
        return false;
      }
    } else {
      B2DEBUG(29, "One or more of the PyObjects in the ExpRun wasn't a long");
      return false;
    }
    // Make sure to kill off the reference GetItem gave us responsibility for
    Py_DECREF(item1);
    Py_DECREF(item2);
  } else {
    B2DEBUG(29, "ExpRun was not a Python sequence.");
    return false;
  }
  return true;
}

/// Converts the PyObject to an ExpRun. We've preoviously checked the object so this assumes a lot about the PyObject
ExpRun CalibrationAlgorithm::convertPyExpRun(PyObject* pyObj)
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

CalibrationAlgorithm::EResult CalibrationAlgorithm::execute(PyObject* runs, int iteration, IntervalOfValidity iov)
{
  B2DEBUG(29, "Running execute() using Python Object as input argument");
  // Reset the execution specific data in case the algorithm was previously called
  m_data.reset();
  m_data.setIteration(iteration);
  vector<ExpRun> vecRuns;
  // Is it a list?
  if (PySequence_Check(runs)) {
    boost::python::handle<> handle(boost::python::borrowed(runs));
    boost::python::list listRuns(handle);

    int nList = boost::python::len(listRuns);
    for (int iList = 0; iList < nList; ++iList) {
      boost::python::object pyExpRun(listRuns[iList]);
      if (!checkPyExpRun(pyExpRun.ptr())) {
        B2ERROR("Received Python ExpRuns couldn't be converted to C++");
        m_data.setResult(c_Failure);
        return c_Failure;
      } else {
        vecRuns.push_back(convertPyExpRun(pyExpRun.ptr()));
      }
    }
  } else {
    B2ERROR("Tried to set the input runs but we didn't receive a Python sequence object (list,tuple).");
    m_data.setResult(c_Failure);
    return c_Failure;
  }
  return execute(vecRuns, iteration, iov);
}

CalibrationAlgorithm::EResult CalibrationAlgorithm::execute(vector<ExpRun> runs, int iteration, IntervalOfValidity iov)
{
  // Check if we are calling this function directly and need to reset, or through Python where it was already done.
  if (m_data.getResult() != c_Undefined) {
    m_data.reset();
    m_data.setIteration(iteration);
  }

  if (m_inputFileNames.empty()) {
    B2ERROR("There aren't any input files set. Please use CalibrationAlgorithm::setInputFiles()");
    m_data.setResult(c_Failure);
    return c_Failure;
  }

  // Did we receive runs to execute over explicitly?
  if (!(runs.empty())) {
    for (auto expRun : runs) {
      B2DEBUG(29, "ExpRun requested = (" << expRun.first << ", " << expRun.second << ")");
    }
    // We've asked explicitly for certain runs, but we should check if the data granularity is 'run'
    if (strcmp(getGranularity().c_str(), "all") == 0) {
      B2ERROR(("The data is collected with granularity=all (exp=-1,run=-1), but you seem to request calibration for specific runs."
               " We'll continue but using ALL the input data given instead of the specific runs requested."));
    }
  } else {
    // If no runs are provided, infer the runs from all collected data
    runs = getRunListFromAllData();
    // Let's check that we have some now
    if (runs.empty()) {
      B2ERROR("No collected data in input files.");
      m_data.setResult(c_Failure);
      return c_Failure;
    }
    for (auto expRun : runs) {
      B2DEBUG(29, "ExpRun requested = (" << expRun.first << ", " << expRun.second << ")");
    }
  }

  m_data.setRequestedRuns(runs);
  if (iov.empty()) {
    // If no user specified IoV we use the IoV from the executed run list
    iov = IntervalOfValidity(runs[0].first, runs[0].second, runs[runs.size() - 1].first, runs[runs.size() - 1].second);
  }
  m_data.setRequestedIov(iov);
  // After here, the getObject<...>(...) helpers start to work

  CalibrationAlgorithm::EResult result = calibrate();
  m_data.setResult(result);
  return result;
}

/// Set the input file names used for this algorithm and resolve the wildcards
void CalibrationAlgorithm::setInputFileNames(PyObject* inputFileNames)
{
  // The reasoning for this very 'manual' approach to extending the Python interface
  // (instead of using boost::python) is down to my fear of putting off final users with
  // complexity on their side.
  //
  // I didn't want users that inherit from this class to be forced to use boost and
  // to have to define a new python module just to use the CAF. A derived class from
  // from a boost exposed class would need to have its own boost python module definition
  // to allow access from a steering file and to the base class functions (I think).
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
void CalibrationAlgorithm::setInputFileNames(vector<string> inputFileNames)
{
  // A lot of code below is tweaked from RootInputModule::initialize,
  // since we're basically copying the functionality anyway.
  if (inputFileNames.empty()) {
    B2WARNING("You have called setInputFileNames() with an empty list. Did you mean to do that?");
    return;
  }
  auto tmpInputFileNames = RootIOUtilities::expandWordExpansions(inputFileNames);

  // We'll use a set to enforce sorted unique file paths as we check them
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
  } else {
    // Reset the run -> files map as our files are likely different
    m_runsToInputFiles.clear();
  }

  // Open TFile to check they can be accessed by ROOT
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
  m_granularityOfData = getGranularityFromData();
}

PyObject* CalibrationAlgorithm::getInputFileNames()
{
  PyObject* objInputFileNames = PyList_New(m_inputFileNames.size());
  for (size_t i = 0; i < m_inputFileNames.size(); ++i) {
    PyList_SetItem(objInputFileNames, i, Py_BuildValue("s", m_inputFileNames[i].c_str()));
  }
  return objInputFileNames;
}

string CalibrationAlgorithm::getExpRunString(ExpRun& expRun) const
{
  string expRunString;
  expRunString += to_string(expRun.first);
  expRunString += ".";
  expRunString += to_string(expRun.second);
  return expRunString;
}

string CalibrationAlgorithm::getFullObjectPath(const string& name, ExpRun expRun) const
{
  string dirName = getPrefix() + "/" + name;
  string objName = name + "_" + getExpRunString(expRun);
  return dirName + "/" + objName;
}

void CalibrationAlgorithm::saveCalibration(TObject* data, const string& name, const IntervalOfValidity& iov)
{
  B2DEBUG(29, "Saving calibration TObject = '" <<  name << "' to payloads list.");
  getPayloads().emplace_back(name, data, iov);
}

void CalibrationAlgorithm::saveCalibration(TClonesArray* data, const string& name, const IntervalOfValidity& iov)
{
  B2DEBUG(29, "Saving calibration TClonesArray '" <<  name << "' to payloads list.");
  getPayloads().emplace_back(name, data, iov);
}

void CalibrationAlgorithm::saveCalibration(TObject* data, const IntervalOfValidity& iov)
{
  saveCalibration(data, DataStore::objectName(data->IsA(), ""), iov);
}

void CalibrationAlgorithm::saveCalibration(TObject* data)
{
  saveCalibration(data, DataStore::objectName(data->IsA(), ""));
}

void CalibrationAlgorithm::saveCalibration(TObject* data, const string& name)
{
  saveCalibration(data, name, m_data.getRequestedIov());
}

void CalibrationAlgorithm::saveCalibration(TClonesArray* data, const string& name)
{
  saveCalibration(data, name, m_data.getRequestedIov());
}

bool CalibrationAlgorithm::commit()
{
  if (getPayloads().empty())
    return false;
  list<Database::DBImportQuery> payloads = getPayloads();
  B2INFO("Committing " << payloads.size()  << " payloads to database.");
  return Database::Instance().storeData(payloads);
}

bool CalibrationAlgorithm::commit(list<Database::DBImportQuery> payloads)
{
  if (payloads.empty())
    return false;
  return Database::Instance().storeData(payloads);
}

vector<ExpRun> CalibrationAlgorithm::getRunListFromAllData() const
{
  RunRange runRange = getRunRangeFromAllData();
  set<ExpRun> expRunSet = runRange.getExpRunSet();
  return vector<ExpRun>(expRunSet.begin(), expRunSet.end());
}

IntervalOfValidity CalibrationAlgorithm::getIovFromAllData() const
{
  return getRunRangeFromAllData().getIntervalOfValidity();
}

void CalibrationAlgorithm::fillRunToInputFilesMap()
{
  m_runsToInputFiles.clear();
  // Save TDirectory to change back at the end
  TDirectory* dir = gDirectory;
  RunRange* runRange;
  // Construct the TDirectory name where we expect our objects to be
  string runRangeObjName(getPrefix() + "/" + RUN_RANGE_OBJ_NAME);
  for (const auto& fileName : m_inputFileNames) {
    //Open TFile to get the objects
    unique_ptr<TFile> f;
    f.reset(TFile::Open(fileName.c_str(), "READ"));
    runRange = dynamic_cast<RunRange*>(f->Get(runRangeObjName.c_str()));
    if (runRange) {
      // Insert or extend the run -> file mapping for this ExpRun
      auto expRuns = runRange->getExpRunSet();
      for (const auto& expRun : expRuns) {
        auto runFiles = m_runsToInputFiles.find(expRun);
        if (runFiles != m_runsToInputFiles.end()) {
          (runFiles->second).push_back(fileName);
        } else {
          m_runsToInputFiles.insert(std::make_pair(expRun, std::vector<std::string> {fileName}));
        }
      }
    } else {
      B2WARNING("Missing a RunRange object for file: " << fileName);
    }
  }
  dir->cd();
}

RunRange CalibrationAlgorithm::getRunRangeFromAllData() const
{
  // Save TDirectory to change back at the end
  TDirectory* dir = gDirectory;
  RunRange runRange;
  // Construct the TDirectory name where we expect our objects to be
  string runRangeObjName(getPrefix() + "/" + RUN_RANGE_OBJ_NAME);
  for (const auto& fileName : m_inputFileNames) {
    //Open TFile to get the objects
    unique_ptr<TFile> f;
    f.reset(TFile::Open(fileName.c_str(), "READ"));
    RunRange* runRangeOther = dynamic_cast<RunRange*>(f->Get(runRangeObjName.c_str()));
    if (runRangeOther) {
      runRange.merge(runRangeOther);
    } else {
      B2WARNING("Missing a RunRange object for file: " << fileName);
    }
  }
  dir->cd();
  return runRange;
}

string CalibrationAlgorithm::getGranularityFromData() const
{
  // Save TDirectory to change back at the end
  TDirectory* dir = gDirectory;
  RunRange* runRange;
  string runRangeObjName(getPrefix() + "/" + RUN_RANGE_OBJ_NAME);
  // We only check the first file
  string fileName = m_inputFileNames[0];
  unique_ptr<TFile> f;
  f.reset(TFile::Open(fileName.c_str(), "READ"));
  runRange = dynamic_cast<RunRange*>(f->Get(runRangeObjName.c_str()));
  if (!runRange) {
    B2FATAL("The input file " << fileName << " does not contain a RunRange object at "
            << runRangeObjName << ". Please set your input files to exclude it.");
    return "";
  }
  string granularity = runRange->getGranularity();
  dir->cd();
  return granularity;
}

void CalibrationAlgorithm::updateDBObjPtrs(const unsigned int event = 1, const int run = 0, const int experiment = 0)
{
  // Construct an EventMetaData object but NOT in the Datastore
  EventMetaData emd(event, run, experiment);
  // Explicitly update while avoiding registering a Datastore object
  DBStore::Instance().update(emd);
  // Also update the intra-run objects to the event at the same time (maybe unnessary...)
  DBStore::Instance().updateEvent(event);
}

// Have to put the explicit template specialization in the enclosing namespace
namespace Belle2 {
  /** We cheekily cast the TChain to TTree for the returned pointer so that the user never knows
    * Hopefully this doesn't cause issues if people do low level stuff to the tree...
    */
  template<>
  shared_ptr<TTree> CalibrationAlgorithm::getObjectPtr<TTree>(const string& name,
                                                              const vector<ExpRun>& requestedRuns)
  {
    // Check if this object already exists
    RunRange runRangeRequested(requestedRuns);
    std::shared_ptr<TTree> objOutputPtr = std::dynamic_pointer_cast<TTree>(m_data.getCalibObj(name, runRangeRequested));
    if (objOutputPtr)
      return objOutputPtr;

    // If not we best make a new one
    shared_ptr<TChain> chain = make_shared<TChain>(name.c_str());
    chain->SetDirectory(0);
    // Construct the TDirectory names where we expect our objects to be
    string runRangeObjName(getPrefix() + "/" + RUN_RANGE_OBJ_NAME);

    if (strcmp(getGranularity().c_str(), "run") == 0) {
      // Loop over our runs requested for the right files
      for (auto expRunRequested : requestedRuns) {
        // Find the relevant files for this ExpRun
        auto searchFiles = m_runsToInputFiles.find(expRunRequested);
        if (searchFiles == m_runsToInputFiles.end()) {
          B2WARNING("No input file found with data collected from run "
                    "(" << expRunRequested.first << "," << expRunRequested.second << ")");
          continue;
        } else {
          auto files = searchFiles->second;
          for (auto fileName : files) {
            RunRange* runRangeData;
            //Open TFile to get the objects
            std::unique_ptr<TFile> f;
            f.reset(TFile::Open(fileName.c_str(), "READ"));
            runRangeData = dynamic_cast<RunRange*>(f->Get(runRangeObjName.c_str()));
            // Check that nothing went wrong in the mapping and that this file definitely contains this run's data
            auto runSet = runRangeData->getExpRunSet();
            if (runSet.find(expRunRequested) == runSet.end()) {
              B2WARNING("Something went wrong with the mapping of ExpRun -> Input Files. "
                        "(" << expRunRequested.first << "," << expRunRequested.second << ") not in " << fileName);
            }
            // Get the path/directory of the Exp,Run TDirectory that holds the object(s)
            std::string objDirName = getFullObjectPath(name, expRunRequested);
            TDirectory* objDir = f->GetDirectory(objDirName.c_str());
            if (!objDir) {
              B2ERROR("Directory for requested object " << name << " not found: " << objDirName);
              return nullptr;
            }
            // Find all the objects inside, there may be more than one
            for (auto key : * (objDir->GetListOfKeys())) {
              string keyName = key->GetName();
              string objectPath = fileName + "/" + objDirName + "/" + keyName;
              B2DEBUG(29, "Adding TTree " << objectPath);
              chain->Add(objectPath.c_str());
            }
          }
        }
      }
    } else {
      ExpRun allGranExpRun = getAllGranularityExpRun();
      string objDirName = getFullObjectPath(name, allGranExpRun);
      for (const auto& fileName : m_inputFileNames) {
        string objectPath = fileName + "/" + objDirName + "/" + name + "_1";  // Only one index for this granularity
        B2DEBUG(29, "Adding TTree " << objectPath);
        chain->Add(objectPath.c_str());
      }
    }
    if (!chain->GetListOfFiles()->GetEntries()) {
      B2ERROR("No data found for object " << name);
      return nullptr;
    }
    objOutputPtr = static_pointer_cast<TTree>(chain);
    // make a TNamed version to input to the map of previous calib objects
    shared_ptr<TNamed> storedObjPtr = static_pointer_cast<TNamed>(objOutputPtr);
    m_data.setCalibObj(name, runRangeRequested, storedObjPtr);
    B2DEBUG(29, "Passing back merged data " << name);
    return objOutputPtr;
  }
}

bool CalibrationAlgorithm::loadInputJson(const std::string& jsonString)
{
  try {
    auto jsonInput = nlohmann::json::parse(jsonString);
    // Input string has an object (dict) as the top level object?
    if (jsonInput.is_object()) {
      m_jsonExecutionInput = jsonInput;
      return true;
    } else {
      B2ERROR("JSON input string isn't an object type i.e. not a '{}' at the top level.");
      return false;
    }
  } catch (nlohmann::json::parse_error&) {
    B2ERROR("Parsing of JSON input string failed");
    return false;
  }
}

const std::vector<ExpRun> CalibrationAlgorithm::findPayloadBoundaries(std::vector<ExpRun> runs, int iteration)
{
  m_boundaries.clear();
  if (m_inputFileNames.empty()) {
    B2ERROR("There aren't any input files set. Please use CalibrationAlgorithm::setInputFiles()");
    return m_boundaries;
  }
  // Reset the internal execution data just in case something is hanging around
  m_data.reset();
  if (runs.empty()) {
    // Want to loop over all runs we could possibly know about
    runs = getRunListFromAllData();
  }
  // Let's check that we have some now
  if (runs.empty()) {
    B2ERROR("No collected data in input files.");
    return m_boundaries;
  }
  // In order to find run boundaries we must have collected with data granularity == 'run'
  if (strcmp(getGranularity().c_str(), "all") == 0) {
    B2ERROR("The data is collected with granularity='all' (exp=-1,run=-1), and we can't use that to find run boundaries.");
    return m_boundaries;
  }
  m_data.setIteration(iteration);
  // User defined setup function
  boundaryFindingSetup(runs, iteration);
  std::vector<ExpRun> runList;
  // Loop over run list and call derived class "isBoundaryRequired" member function
  for (auto currentRun : runs) {
    runList.push_back(currentRun);
    m_data.setRequestedRuns(runList);
    // After here, the getObject<...>(...) helpers start to work
    if (isBoundaryRequired(currentRun)) {
      m_boundaries.push_back(currentRun);
    }
    // Only want run-by-run
    runList.clear();
    // Don't want memory hanging around
    m_data.clearCalibrationData();
  }
  m_data.reset();
  boundaryFindingTearDown();
  return m_boundaries;
}
