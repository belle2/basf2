/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <Python.h>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <any>
#include <utility>
#include <list>
#include <nlohmann/json.hpp>
#include <TClonesArray.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TTree.h>
#include <framework/database/Database.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/logging/Logger.h>
#include <calibration/Utilities.h>
#include <calibration/dataobjects/RunRange.h>

namespace Belle2 {
  /**
   * Base class for calibration algorithms
   */
  class CalibrationAlgorithm {
  public:
    /// The result of calibration
    enum EResult {
      c_OK,           /**< Finished successfuly             =0 in Python */
      c_Iterate,      /**< Needs iteration                  =1 in Python */
      c_NotEnoughData,/**< Needs more data                  =2 in Python */
      c_Failure,      /**< Failed                           =3 in Python */
      c_Undefined     /**< Not yet known (before execution) =4 in Python */
    };

    /**
     * A class to hold all data that is needed ONLY for the most recent single execution of the algorithm.
     * This is then reset upon calling execute() again. All other data members aren't reset
     * unless you do it explicitly.
     */
    class ExecutionData {
    public:
      ExecutionData() {};
      ~ExecutionData() {};
      /// Resets this class back to what is needed at the beginning of an execution
      void reset()
      {
        B2DEBUG(100, "Resetting ExecutionData of algorithm");
        m_requestedRuns.clear();
        m_iteration = -1;
        m_result = c_Undefined;
        m_payloads.clear();
        m_iov = IntervalOfValidity();
        clearCalibrationData();
      }
      /// Clear calibration data
      void clearCalibrationData()
      {
        m_mapCalibData.clear();
      }
      /// Returns the vector of ExpRuns
      const std::vector<Calibration::ExpRun>& getRequestedRuns() const {return m_requestedRuns;}
      /// Sets the vector of ExpRuns
      void setRequestedRuns(const std::vector<Calibration::ExpRun>& requestedRuns) {m_requestedRuns = requestedRuns;}
      /// Getter for current iteration
      int getIteration() const {return m_iteration;}
      /// Setter for current iteration
      void setIteration(int iteration)
      {
        B2DEBUG(29, "Setting Iteration of Algorithm to " << iteration);
        m_iteration = iteration;
      }
      /// Getter for current result
      EResult getResult() const {return m_result;}
      /// Setter for current iteration
      void setResult(EResult result) {m_result = result;}
      /// Sets the requested IoV for this execution, based on the
      void setRequestedIov(const IntervalOfValidity& iov = IntervalOfValidity(0, 0, -1, -1)) {m_iov = iov;}
      /// Getter for requested IOV
      const IntervalOfValidity& getRequestedIov() const {return m_iov;}
      /// Get constants (in TObjects) for database update from last calibration
      std::list<Database::DBImportQuery>& getPayloads() {return m_payloads;}
      /// Get constants (in TObjects) for database update from last calibration but passed by VALUE
      std::list<Database::DBImportQuery> getPayloadValues() {return m_payloads;}
      /// Get a previously created object in m_mapCalibData if one exists, otherwise return shared_ptr(nullptr)
      std::shared_ptr<TNamed> getCalibObj(const std::string& name, const RunRange& runRange) const
      {
        auto it = m_mapCalibData.find(std::make_pair(name, runRange));
        if (it == m_mapCalibData.end()) {
          return nullptr;
        }
        return it->second;
      }
      /// Insert a newly created object in m_mapCalibData. Overwrites a previous entry if one exists
      void setCalibObj(const std::string& name, const RunRange& runRange, const std::shared_ptr<TNamed>& objectPtr)
      {
        m_mapCalibData[std::make_pair(name, runRange)] = objectPtr;
      }

    private:
      /// Runs for which the calibration has been last requested, either requested explicitly or generated from the collected data
      std::vector<Calibration::ExpRun> m_requestedRuns{};
      /// current iteration for execution
      int m_iteration{ -1};
      /// Result of execution, default undefined to indicate we haven't run yet
      EResult m_result{c_Undefined};
      /// Current IoV to be executed, default empty. Will be either set by user explicitly or generated from collected/requested runs
      IntervalOfValidity m_iov;
      /// Payloads saved by execution
      std::list<Database::DBImportQuery> m_payloads{};
      /**  Map of shared pointers to merged calibration objects created by getObjectPtr() calls.
        *  Used to lookup previously created objects instead of recreating them needlessly.
        *  Using shared_ptr allows us to return a shared_ptr so the user knows that they don't
        *  own the pointer exclusively, but that they could hold onto a shared_ptr for longer than the
        *  reset of this ExecutionData object if they really wanted to i.e. you could hold onto a previous execution's
        *  created data if you wish (not recommended), but if you don't care then you can be assured that the memory will
        *  clear itself for you upon a new execute().
        */
      std::map<std::pair<std::string, RunRange>, std::shared_ptr<TNamed>> m_mapCalibData;
    };

    /**
     * Constructor - sets the prefix for collected objects (won't be accesses until execute(...))
     * so you can change it via setPrefix(...). The prefix has to be shared among collectors and
     * the algorithm to as it names the TDirectory in the collector output file where the objects are
     * stored. But this can be changed (via parameter for collector modules).
     */
    explicit CalibrationAlgorithm(const std::string& collectorModuleName) : m_prefix(collectorModuleName) {}

    /// Virtual destructor (base class)
    virtual ~CalibrationAlgorithm() {}

    /// Get the prefix used for getting calibration data
    std::string getPrefix() const {return m_prefix;}

    /// Checks that a PyObject can be successfully converted to an ExpRun type
    bool checkPyExpRun(PyObject* pyObj);

    /// Performs the conversion of PyObject to ExpRun
    Calibration::ExpRun convertPyExpRun(PyObject* pyObj);

    /**
     * Alias for prefix. For convenience and less writing, we say developers to
     * set this to default collector module name in constructor of base class.
     * One can however use the dublets of collector+algorithm multiple times
     * with different settings. To bind these together correctly, the prefix has
     * to be set the same for algo and collector. So we call the setter setPrefix
     * rather than setModuleName or whatever. This getter will work out of the
     * box for default cases -> return the name of module you have to add to your
     * path to collect data for this algorihtm.
     */
    std::string getCollectorName() const {return getPrefix();}

    /// Set the prefix used to identify datastore objects
    void setPrefix(const std::string& prefix) {m_prefix = prefix;}

    /// Set the input file names used for this algorithm from a Python list
    void setInputFileNames(PyObject* inputFileNames);

    /// Get the input file names used for this algorithm and pass them out as a Python list of unicode strings
    PyObject* getInputFileNames();

    /// Get the complete list of runs from inspection of collected data
    std::vector<Calibration::ExpRun> getRunListFromAllData() const;

    /// Get the complete RunRange from inspection of collected data
    RunRange getRunRangeFromAllData() const;

    /// Get the complete IoV from inspection of collected data
    IntervalOfValidity getIovFromAllData() const;

    /// Fill the mapping of ExpRun -> Files
    void fillRunToInputFilesMap();

    /// Get the granularity of collected data
    std::string getGranularity() const {return m_granularityOfData;};

    /**
     * Runs calibration over vector of runs for a given iteration. You can also specify the IoV to
     * save the database payload as. By default the Algorithm will create an IoV from your requested
     * ExpRuns, or from the overall ExpRuns of the input data if you haven't specified ExpRuns in this function.
     *
     * No checks are performed to make sure that a IoV you specify matches the data you ran over, it
     * simply labels the IoV to commit to the database later.
     */
    EResult execute(std::vector<Calibration::ExpRun> runs = {}, int iteration = 0, IntervalOfValidity iov = IntervalOfValidity());

    /// Runs calibration over Python list of runs. Converts to C++ and then calls the other execute() function
    EResult execute(PyObject* runs, int iteration = 0, IntervalOfValidity iov = IntervalOfValidity());

    /// Get constants (in TObjects) for database update from last execution
    std::list<Database::DBImportQuery>& getPayloads() {return m_data.getPayloads();}

    /// Get constants (in TObjects) for database update from last execution but passed by VALUE
    std::list<Database::DBImportQuery> getPayloadValues() {return m_data.getPayloadValues();}

    /// Submit constants from last calibration into database
    bool commit();

    /// Submit constants from a (potentially previous) set of payloads
    bool commit(std::list<Database::DBImportQuery> payloads);

    /// Get the description of the algoithm (set by developers in constructor)
    const std::string& getDescription() const {return m_description;}

    /// Load the m_inputJson variable from a string (useful from Python interface). The rturn bool indicates success or failure
    //  of JSON object creation. Failure probably means that your JSON string was badly formatted.
    bool loadInputJson(const std::string& jsonString);

    /// Dump the JSON string of the output JSON object.
    const std::string dumpOutputJson() const {return m_jsonExecutionOutput.dump();}

    /// Used to discover the ExpRun boundaries that you want the Python CAF to execute on. This is optional and only used in some
    //  areas of the CAF. Basically you search for features in the data that you want to find and make sure that the CAF knows
    //  there is a boundary where payloads should probably start/end. The output boundaries should be the starting ExpRun
    //  of the new boundary.
    const std::vector<Calibration::ExpRun> findPayloadBoundaries(std::vector<Calibration::ExpRun> runs, int iteration = 0);

  protected:
    // Developers implement this function ------------

    /// Run algo on data - pure virtual: needs to be implemented
    virtual EResult calibrate() = 0;

    /// Given the current collector data, make a decision about whether or not this run should be the start of a payload boundary
    //  Implementing this is optional because most people will never call findPayloadBoundaries in their CAF job.
    //  It returns false by default so that the boundaries vector is empty if you forgot to implement this.
    //
    //  We omit the names of arguments here so that we don't generate lots of compiler warnings in algorithms that don't
    //  implement this function.
    virtual bool isBoundaryRequired(const Calibration::ExpRun& /*currentRun*/)
    {
      B2ERROR("You didn't implement a isBoundaryRequired() member function in your CalibrationAlgorithm but you are calling it!");
      return false;
    }

    /// If you need to make some changes to your algorithm class before 'findPayloadBoundaries' is run, make them in this function
    //  We omit the names of arguments here so that we don't generate lots of compiler warnings in algorithms that don't
    //  implement this function.
    virtual void boundaryFindingSetup(std::vector<Calibration::ExpRun> /*runs*/, int /*iteration = 0*/) {};

    /// Put your algorithm back into a state ready for normal execution if you need to.
    //  This runs right after 'findPayloadBoundaries' and is supposed to  correct any changes you made in 'boundaryFindingSetup'
    //  or 'isBoundaryRequired'.
    virtual void boundaryFindingTearDown() {};

    // When using the boundaries functionality from isBoundaryRequired, this is used to store the boundaries. It is cleared when
    // calling findPayloadBoundaries, before boundaryFindingSetup is called.
    std::vector<Calibration::ExpRun> m_boundaries;

    // Helpers ---------------- Data retrieval -------

    /// Get the list of runs for which calibration is called
    const std::vector<Calibration::ExpRun>& getRunList() const {return m_data.getRequestedRuns();}

    /// Get current iteration
    int getIteration() const { return m_data.getIteration(); }

    /// Set the input file names used for this algorithm
    void setInputFileNames(std::vector<std::string> inputFileNames);

    /// Get the input file names used for this algorithm as a STL vector
    std::vector<std::string> getVecInputFileNames() const {return m_inputFileNames;}

    /// Get calibration data object by name and list of runs, the Merge function will be called to generate the overall object
    template<class T>
    std::shared_ptr<T> getObjectPtr(const std::string& name, const std::vector<Calibration::ExpRun>& requestedRuns);

    /** Get calibration data object (for all runs the calibration is requested for)
     *  This function will only work during or after execute() has been called once.
     */
    template<class T>
    std::shared_ptr<T> getObjectPtr(std::string name)
    {
      if (m_runsToInputFiles.size() == 0)
        fillRunToInputFilesMap();
      return getObjectPtr<T>(name, m_data.getRequestedRuns());
    }

    // Helpers ---------------- Database storage -----

    /// Get the granularity of collected data
    std::string getGranularityFromData() const;

    /// Store DBArray payload with given name with default IOV
    void saveCalibration(TClonesArray* data, const std::string& name);

    /// Store DBArray with given name and custom IOV
    void saveCalibration(TClonesArray* data, const std::string& name, const IntervalOfValidity& iov);

    /// Store DB payload with default name and default IOV
    void saveCalibration(TObject* data);

    /// Store DB payload with default name and custom IOV
    void saveCalibration(TObject* data, const IntervalOfValidity& iov);

    /// Store DB payload with given name with default IOV
    void saveCalibration(TObject* data, const std::string& name);

    /// Store DB payload with given name and custom IOV
    void saveCalibration(TObject* data, const std::string& name, const IntervalOfValidity& iov);

    /// Updates any DBObjPtrs by calling update(event) for DBStore
    void updateDBObjPtrs(const unsigned int event, const int run, const int experiment);

    // -----------------------------------------------

    /// Set algorithm description (in constructor)
    void setDescription(const std::string& description) {m_description = description;}

    /// Clear calibration data
    void clearCalibrationData() {m_data.clearCalibrationData();}

    /// Returns the Exp,Run pair that means 'Everything'. Currently unused.
    Calibration::ExpRun getAllGranularityExpRun() const {return m_allExpRun;}

    /// Clears the m_inputJson member variable.
    void resetInputJson() {m_jsonExecutionInput.clear();}

    /// Clears the m_outputJson member variable.
    void resetOutputJson() {m_jsonExecutionOutput.clear();}

    /// Set a key:value pair for the outputJson object, expected to used interally during calibrate()
    template<class T>
    void setOutputJsonValue(const std::string& key, const T& value) {m_jsonExecutionOutput[key] = value;}

    /// Get a value using a key from the JSON output object, not sure why you would want to do this.
    //  No attempt to catch exceptions is made here.
    template<class T>
    const T getOutputJsonValue(const std::string& key) const
    {
      return m_jsonExecutionOutput.at(key);
    }

    /// Get an input JSON value using a key. The normal exceptions are raised when the key doesn't exist.
    //  No attempt to catch them is made here.
    template<class T>
    const T getInputJsonValue(const std::string& key) const
    {
      return m_jsonExecutionInput.at(key);
    }

    /// Get the entire top level JSON object. We explicitly say this must be of object type so that we might pick
    //  up on weird errors where someone snuck an array into the member variable.
    const nlohmann::json& getInputJsonObject() const {return m_jsonExecutionInput;}

    /// Test for a key in the input JSON object
    bool inputJsonKeyExists(const std::string& key) const {return m_jsonExecutionInput.count(key);}

  private:

    static const Calibration::ExpRun m_allExpRun;

    /// Gets the "exp.run" string repr. of (exp,run)
    std::string getExpRunString(Calibration::ExpRun& expRun) const;

    /// constructs the full TDirectory + Key name of an object in a TFile based on its name and exprun
    std::string getFullObjectPath(const std::string& name, Calibration::ExpRun expRun) const;

    /// List of input files to the Algorithm, will initially be user defined but then gets the wildcards expanded during execute()
    std::vector<std::string> m_inputFileNames;

    /// Map of Runs to input files. Gets filled when you call getRunRangeFromAllData, gets cleared when setting input files again
    std::map<Calibration::ExpRun, std::vector<std::string>> m_runsToInputFiles;

    /// Granularity of input data. This only changes when the input files change so it isn't specific to an execution
    std::string m_granularityOfData;

    /// Data specific to a SINGLE execution of the algorithm. Gets reset at the beginning of execution
    ExecutionData m_data;

    /// Description of the algorithm
    std::string m_description{""};

    /// The name of the TDirectory the collector objects are contained within
    std::string m_prefix{""};

    /// Optional input JSON  object used to make decisions about how to execute the algorithm code.
    //  We initialise to "{}" rather than allowing a JSON array/value as the top level type. This forces the user
    //  to use "key":value for storing data in this object. You should test for empty(), is_null() will always
    //  return false due to the empty top level object.
    //  Functionally similar to simple member variables for configuration of the algorithm.
    //  However these input values are easier to use from Python code without needing to know the details of the algorithm.
    //  These values are intended to be used for a single execution, not reused
    nlohmann::json m_jsonExecutionInput = nlohmann::json::object();

    /// Optional output JSON object that can be set during the execution by the underlying algorithm code.
    //  As for input we initialise to an empty "{}" JSON object. Testing for empty() returns true, but is_null() does not.
    //  Nothing is done with these by default, however a calling process may decide to capture these values and use them
    //  as input to a following execution.
    nlohmann::json m_jsonExecutionOutput = nlohmann::json::object();

  };  // End of CalibrationAlgorithm definition


  /**************************************
   *                                    *
   * Implementation of larger templates *
   *                                    *
   **************************************/
  template<class T>
  std::shared_ptr<T> CalibrationAlgorithm::getObjectPtr(const std::string& name,
                                                        const std::vector<Calibration::ExpRun>& requestedRuns)
  {
    // Check if this object already exists
    RunRange runRangeRequested(requestedRuns);
    std::shared_ptr<T> objOutputPtr = std::dynamic_pointer_cast<T>(m_data.getCalibObj(name, runRangeRequested));
    if (objOutputPtr)
      return objOutputPtr;

    std::shared_ptr<T> mergedObjPtr(nullptr);
    bool mergedEmpty = true;
    TDirectory* dir = gDirectory;

    // Technically we could grab all the objects from all files, add to list and then merge at the end.
    // But I prefer the (maybe) more memory efficient way of merging with all objects
    // in a file before moving on to the next one, just in case TDirectory stuff screws us.
    TList list;
    list.SetOwner(false);

    // Construct the TDirectory names where we expect our objects to be
    std::string runRangeObjName(getPrefix() + "/" + Calibration::RUN_RANGE_OBJ_NAME);

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
              std::string keyName = key->GetName();
              B2DEBUG(100, "Adding found object " << keyName << " in the directory " << objDir->GetPath());
              T* objOther = (T*)objDir->Get(keyName.c_str());
              if (objOther) {
                if (mergedEmpty) {
                  mergedObjPtr = std::shared_ptr<T>(dynamic_cast<T*>(objOther->Clone(name.c_str())));
                  mergedObjPtr->SetDirectory(0);
                  mergedEmpty = false;
                } else {
                  list.Add(objOther);
                }
              }
            }
            if (!mergedEmpty)
              mergedObjPtr->Merge(&list);
            list.Clear();
          }
        }
      }
    } else {
      for (auto fileName : m_inputFileNames) {
        //Open TFile to get the objects
        std::unique_ptr<TFile> f;
        f.reset(TFile::Open(fileName.c_str(), "READ"));
        Calibration::ExpRun allGranExpRun = getAllGranularityExpRun();
        std::string objDirName = getFullObjectPath(name, allGranExpRun);
        std::string objPath = objDirName + "/" + name + "_1";
        T* objOther = (T*)f->Get(objPath.c_str()); // Only one index for granularity == all
        B2DEBUG(100, "Adding " << objPath);
        if (objOther) {
          if (mergedEmpty) {
            mergedObjPtr = std::shared_ptr<T>(dynamic_cast<T*>(objOther->Clone(name.c_str())));
            mergedObjPtr->SetDirectory(0);
            mergedEmpty = false;
          } else {
            list.Add(objOther);
          }
        }
        if (!mergedEmpty)
          mergedObjPtr->Merge(&list);
        list.Clear();
      }
    }
    dir->cd();
    objOutputPtr = mergedObjPtr;
    if (!objOutputPtr) {
      B2ERROR("No data found for object " << name);
      return nullptr;
    }
    objOutputPtr->SetDirectory(0);
    // make a TNamed version to input to the map of previous calib objects
    std::shared_ptr<TNamed> storedObjPtr = std::static_pointer_cast<TNamed>(objOutputPtr);
    m_data.setCalibObj(name, runRangeRequested, storedObjPtr);
    B2DEBUG(100, "Passing back merged data " << name);
    return objOutputPtr;
  }

  /**
   * Specialization of getObjectPtr<TTree>.
   */
  template<> std::shared_ptr<TTree>
  CalibrationAlgorithm::getObjectPtr(
    const std::string& name,
    const std::vector<Calibration::ExpRun>& requestedRuns);

} // namespace Belle2
