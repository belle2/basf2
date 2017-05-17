/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Tadeas Bilka                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <string>
#include <vector>
#include <list>
#include <TClonesArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/Database.h>
#include <framework/database/IntervalOfValidity.h>

#include <calibration/dataobjects/RunRange.h>
#include <calibration/dataobjects/CalibRootObj.h>

namespace Belle2 {
  /**
   * Base class for calibration algorithms
   */
  class CalibrationAlgorithm {
  public:
    /// The name of always-created object holding set of all (exp,run) occured in data collection
    static const std::string RUN_RANGE_OBJ_NAME;
    /// Convenient typedef for (exp,run) pairs
    typedef std::pair<int, int> ExpRun;

    /// The result of calibration
    enum EResult {
      c_OK,           /**< Finished successfuly =0 in Python */
      c_Iterate,      /**< Needs iteration      =1 in Python */
      c_NotEnoughData,/**< Needs more data      =2 in Python */
      c_Failure       /**< Failed               =3 in Python */
    };

    /**
     * Constructor - sets the prefix for datastore objects (won't be accesses until execute(...))
     * so you can change it via setPrefix(...). The prefix has to be shared among collectors and
     * the algorithm to share the datastore objects, but can be changed (via parameter for collector modules).
     */
    explicit CalibrationAlgorithm(const std::string& collectorModuleName) : m_prefix(collectorModuleName) {}

    /// Virtual destructor (base class)
    virtual ~CalibrationAlgorithm() {}

    /// Get the prefix used for getting calibration data
    std::string getPrefix() const {return m_prefix;}

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

    /// Get the complete list of runs from inspection of datastore
    std::vector<ExpRun> getRunListFromAllData();

    /// Runs calibration over list of runs
    EResult execute(std::vector<ExpRun> runs = {}, int iteration = 0);

    /// Get constants (in TObjects) for database update from last calibration
    const std::list<Database::DBQuery>& getPayloads() const { return m_payloads; }

    /// Get constants (in TObjects) for database update from last calibration but passed by VALUE
    const std::list<Database::DBQuery> getPayloadValues() const { return m_payloads; }

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
    const std::vector<ExpRun>& getRunList() const {return m_runs;}

    /// Get current iteration
    int getIteration() const { return m_iteration; }

    /// Get calibration data object by name and list of runs
    template<class T>
    T& getObject(const std::string& name, std::vector<CalibrationAlgorithm::ExpRun> runlist) const
    {
      std::string fullName = m_prefix + "_" + name;
      StoreObjPtr<CalibRootObj<T>> storeobj(fullName, DataStore::c_Persistent);

      if (!storeobj.isValid()) {
        B2ERROR("Access to non-existing datastore object with name " << fullName << ". New empty object is created and returned.");
        B2ERROR("Check that the name registered in collector is the same as you request in algorithm: " << name);
        B2ERROR("Check that the algorithm and collector use the same prefix (if you changed collector name): " << m_prefix);
        storeobj.registerInDataStore();
        storeobj.construct();
      }

      std::string strRunList = runList2String(runlist);
      // TODO: Merge only once (now) or each call again?
      if (storeobj->objectExists(strRunList))
        return storeobj->getObject(strRunList);

      // First access creates new object from template
      auto& merged = storeobj->getObject(strRunList);
      merged.Reset(); // To be sure
      TList list;
      list.SetOwner(false);
      for (auto run : runlist) {
        list.Add(&storeobj->getObject(runList2String(run)));
      }
      merged.Merge(&list);

      return merged;
    }

    /// Get calibration data object by name and run
    template<class T>
    T& getObject(std::string name, CalibrationAlgorithm::ExpRun run) const
    {
      std::vector<ExpRun> runlist;
      runlist.push_back(run);
      return getObject<T>(name, runlist);
    }

    /// Get calibration data object (for all runs the calibration is requested)
    template<class T>
    T& getObject(std::string name) const
    {
      return getObject<T>(name, m_runs);
    }

    // Helpers ---------------- Database storage -----

    /// Get the interval of validity from minimum and maximum experiment and run of data in requested calibration range
    IntervalOfValidity getIovFromData();

    /// Store DBArray payload with given name with default IOV
    void saveCalibration(TClonesArray* data, const std::string& name);

    /// Store DBArray with given name and custom IOV
    void saveCalibration(TClonesArray* data, const std::string& name, const IntervalOfValidity& iov);

    /// Store DB payload with given name with default IOV
    void saveCalibration(TObject* data, const std::string& name);

    /// Store DB payload with given name and custom IOV
    void saveCalibration(TObject* data, const std::string& name, const IntervalOfValidity& iov);

    // -----------------------------------------------

    /// Set algorithm description (in constructor)
    void setDescription(std::string description) {m_description = description;}

  private:

    /// Get string repr. of (exp,run) for CalibRootObj
    std::string runList2String(ExpRun run) const ;
    /// Get string repr. of list((exp,run) for CalibRootObj
    std::string runList2String(std::vector<ExpRun>& list) const;
    /// Get list((exp,run)) from string repr. in CalibRootObj
    std::vector<ExpRun> string2RunList(std::string list) const;

    /// Description of the algorithm
    std::string m_description{""};

    /// The prefix of datastore objects
    std::string m_prefix{""};
    /// Runs for which the calibration has been last requested
    std::vector<ExpRun> m_runs{};
    /// Payloads generated by last calibration (needs to be transient as root needs default constructible elements)
    std::list<Database::DBQuery> m_payloads{}; //!transient

    /// current iteration
    int m_iteration{0};

    ClassDef(CalibrationAlgorithm, 1); /**< Abstract base class for calibration algorithms */
  };
} // namespace Belle2


