/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Tadeas Bilka, David Dossett                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <Python.h>
#include <memory>
#include <string>
#include <vector>
#include <list>
#include <TClonesArray.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TChain.h>
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
      void reset()
      {
        B2DEBUG(100, "Resetting ExecutionData of algorithm");
        m_requestedRuns.clear();
        m_iteration = -1;
        m_result = c_Undefined;
        m_payloads.clear();
        m_iov = IntervalOfValidity();
      }
      /// Returns the vector of ExpRuns
      const std::vector<Calibration::ExpRun>& getRequestedRuns() const {return m_requestedRuns;}
      /// Sets the vector of ExpRuns
      void setRequestedRuns(std::vector<Calibration::ExpRun> requestedRuns) {m_requestedRuns = requestedRuns;}
      /// Getter for current iteration
      int getIteration() const {return m_iteration;}
      /// Setter for current iteration
      void setIteration(int iteration) {m_iteration = iteration;}
      /// Getter for current result
      EResult getResult() const {return m_result;}
      /// Setter for current iteration
      void setResult(EResult result) {m_result = result;}
      /// Sets the requested IoV for this execution, based on the
      void setRequestedIov(IntervalOfValidity iov = IntervalOfValidity(0, 0, -1, -1)) {m_iov = iov;}
      /// Getter for requested IOV
      const IntervalOfValidity& getRequestedIov() const {return m_iov;}
      /// Get constants (in TObjects) for database update from last calibration
      std::list<Database::DBQuery>& getPayloads() {return m_payloads;}
      /// Get constants (in TObjects) for database update from last calibration but passed by VALUE
      std::list<Database::DBQuery> getPayloadValues() {return m_payloads;}

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
      std::list<Database::DBQuery> m_payloads{};
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
    void setPrefix(std::string prefix) {m_prefix = prefix;}

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
    std::list<Database::DBQuery>& getPayloads() {return m_data.getPayloads();}

    /// Get constants (in TObjects) for database update from last execution but passed by VALUE
    std::list<Database::DBQuery> getPayloadValues() {return m_data.getPayloadValues();}

    /// Submit constants from last calibration into database
    bool commit();

    /// Submit constants from a (potentially previous) set of payloads
    bool commit(std::list<Database::DBQuery> payloads);

    /// Get the description of the algoithm (set by developers in constructor)
    const std::string& getDescription() const {return m_description;}

  protected:
    // Developers implement this function ------------

    /// Run algo on data - pure virtual: needs to be implemented
    virtual EResult calibrate() = 0;

    // Helpers ---------------- Data retrieval -------

    /// Get the list of runs for which calibration is called
    const std::vector<Calibration::ExpRun>& getRunList() const {return m_data.getRequestedRuns();}

    /// Get current iteration
    int getIteration() const { return m_data.getIteration(); }

    /// Set the input file names used for this algorithm
    void setInputFileNames(std::vector<std::string> inputFileNames);

    /// Get the input file names used for this algorithm as a STL vector
    std::vector<std::string> getVecInputFileNames() const {return m_inputFileNames;}

    /// Get TTree calibration data object by name and list of runs, use TChain to avoid huge memory usage and merging
    std::unique_ptr<TTree> getTreeObjectPtr(const std::string& name, const std::vector<Calibration::ExpRun>& requestedRuns) const;

    /** Get TTree calibration data object by name, use TChain to avoid huge memory usage and merging
     *  This will only work properly after or during an execute()->calibrate() call.
     *  We pass out a unique_ptr to make it obvious that the caller owns the object.
     */
    std::unique_ptr<TTree> getTreeObjectPtr(const std::string& name) const
    {
      return std::move(getTreeObjectPtr(name, m_data.getRequestedRuns()));
    }

    /// Get calibration data object by name and list of runs, the Merge function will be called to generate the overall object
    template<class T>
    std::unique_ptr<T> getObjectPtr(const std::string& name, const std::vector<Calibration::ExpRun>& requestedRuns) const
    {
      T* mergedObjPtr = new T();
      B2DEBUG(100, "Getting " << mergedObjPtr->ClassName() << " calibration object: " << name);
      bool mergedEmpty = true;
      mergedObjPtr->SetName(name.c_str());
      mergedObjPtr->SetDirectory(0);
      TDirectory* dir = gDirectory;

      // Technically we could grab all the objects from all files, add to list and then merge at the end.
      // But I prefer the more memory efficient way of merging with all objects
      // in a file before moving on to the next one, just in case TDirectory stuff screws us.
      TList list;
      list.SetOwner(false);

      // Construct the TDirectory names where we expect our objects to be
      std::string runRangeObjName(getPrefix() + "/" + Calibration::RUN_RANGE_OBJ_NAME);
      RunRange runRangeRequested(requestedRuns);
      RunRange* runRangeData;
      for (const auto& fileName : m_inputFileNames) {
        //Open TFile to get the objects
        std::unique_ptr<TFile> f;
        f.reset(TFile::Open(fileName.c_str(), "READ"));
        runRangeData = dynamic_cast<RunRange*>(f->Get(runRangeObjName.c_str()));

        if (strcmp(getGranularity().c_str(), "run") == 0) {
          if (runRangeData->getIntervalOfValidity().overlaps(runRangeRequested.getIntervalOfValidity())) {
            B2DEBUG(100, "Found requested data in file: " << fileName);
            // Loop over runs in data and check if they exist in our requested ones, then add if they do
            for (auto expRunData : runRangeData->getExpRunSet()) {
              for (auto expRunRequested : requestedRuns) {
                if (expRunData == expRunRequested) {
                  std::string objName = getFullObjectPath(name, expRunData);
                  B2DEBUG(100, "Adding " << objName << " from file " << fileName);
                  TObject* objOther = f->Get(objName.c_str());
                  if (mergedEmpty) {
                    objOther->Copy(*mergedObjPtr);
                    mergedObjPtr->SetDirectory(0);
                    mergedEmpty = false;
                  } else {
                    list.Add(objOther);
                  }
                }
              }
            }
          } else {
            B2DEBUG(100, "No overlapping data found in file: " << fileName);
            continue;
          }
        } else {
          Calibration::ExpRun allGranExpRun = getAllGranularityExpRun();
          std::string objName = getFullObjectPath(name, allGranExpRun);
          B2DEBUG(100, "Adding " << objName << " from file " << fileName);
          TObject* objOther = f->Get(objName.c_str());
          if (mergedEmpty) {
            objOther->Copy(*mergedObjPtr);
            mergedObjPtr->SetDirectory(0);
            mergedEmpty = false;
          } else {
            list.Add(objOther);
          }
        }
        mergedObjPtr->Merge(&list);
        list.Clear();
      }
      dir->cd();
      std::unique_ptr<T> objOutput(mergedObjPtr);
      return std::move(objOutput);
    }

    /** Get calibration data object (for all runs the calibration is requested for)
     *  This function will only work during or after execute() has been called once.
     */
    template<class T>
    std::unique_ptr<T> getObjectPtr(std::string name) const
    {
      return std::move(getObjectPtr<T>(name, m_data.getRequestedRuns()));
    }

//    // Helpers ---------------- Database storage -----
//
    /// Get the granularity of collected data
    std::string getGranularityFromData() const;

    /// Store DBArray payload with given name with default IOV
    void saveCalibration(TClonesArray* data, const std::string& name);

    /// Store DB payload with given name with default IOV
    void saveCalibration(TObject* data, const std::string& name);

    /// Store DBArray with given name and custom IOV
    void saveCalibration(TClonesArray* data, const std::string& name, const IntervalOfValidity& iov);

    /// Store DB payload with given name and custom IOV
    void saveCalibration(TObject* data, const std::string& name, const IntervalOfValidity& iov);

    // -----------------------------------------------

    /// Set algorithm description (in constructor)
    void setDescription(std::string description) {m_description = description;}

    Calibration::ExpRun getAllGranularityExpRun() const {return m_allExpRun;}

  private:

    static const Calibration::ExpRun m_allExpRun;

    /// Gets the "exp.run" string repr. of (exp,run)
    std::string getExpRunString(Calibration::ExpRun& expRun) const;

    /// constructs the full TDirectory + Key name of an object in a TFile based on its name and exprun
    std::string getFullObjectPath(std::string name, Calibration::ExpRun expRun) const;

    /// List of input files to the Algorithm, will initially be user defined but then gets the wildcards expanded during execute()
    std::vector<std::string> m_inputFileNames;

    /// Granularity of input data. This only changes when the input files change so it isn't specific to an execution
    std::string m_granularityOfData;

    /// Data specific to a SINGLE execution of the algorithm. Gets reset at the beginning of execution
    ExecutionData m_data;

    /// Description of the algorithm
    std::string m_description{""};

    /// The name of the TDirectory the collector objects are contained within
    std::string m_prefix{""};

    ClassDef(CalibrationAlgorithm, 2); /**< Abstract base class for calibration algorithms */
  };
} // namespace Belle2
