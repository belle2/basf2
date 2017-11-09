/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>

#include <memory>
#include <framework/logging/Logger.h>
#include <framework/database/Database.h>

#include <framework/database/DBObjPtr.h>
#include <alignment/GlobalLabel.h>
#include <framework/dbobjects/BeamParameters.h>

namespace Belle2 {
  namespace alignment {
    class GlobalParamVector;
    class GlobalDerivativesHierarchy;

    /// Object representing no parameters
    class EmptyGlobalParamSet : public TObject {
    public:
      // ------------- Interface to global Millepede calibration ----------------
      /// Get global unique id = 0 (empty/none)
      static unsigned short getGlobalUniqueID() {return 0;}
      /// Get global parameter - return zero
      double getGlobalParam(unsigned short, unsigned short) {return 0.;}
      /// Set global parameter - do nothing
      void setGlobalParam(double, unsigned short, unsigned short) {}
      /// list stored global parameters - empty list, no parameters
      std::vector<std::pair<unsigned short, unsigned short>> listGlobalParams() {return {};}
      // ------------------------------------------------------------------------
    };


    /// Some tentative base class to allow to add functionality to the default
    /// behavior, like manipulate constants in DB objects after clibration is finished
    /// All methods to override have default implementaion which does nothing
    class IGlobalParamInterface {
    public:
      /// Implement this to be called after Millepede finishes and all global parameters are filled
      /// in DB objects (which allow this) in GlobalParam
      /// @param std::vector<std::tuple<unsigned short, unsigned short, unsigned short, double>>& tuple with result <unique id, element, param, value>
      /// @param GlobalParamVector& the GlobalVector which you have to update, extend or just read jsu after it is filled
      /// automatically (for DB objects which support it) and before its DBObjects are stored back into DB.
      virtual void readFromResult(std::vector<std::tuple<unsigned short, unsigned short, unsigned short, double>>&,
                                  GlobalParamVector&) {};
      /// Super-tentavive: not yet used
      virtual void setupAlignmentHierarchy(GlobalDerivativesHierarchy&) {}
    };

    /// CDC interface: Fills wire alignment in CDCAlignment based on computed alignment of layers
    class CDCGlobalParamInterface : public IGlobalParamInterface {
    public:
      /// Fills wire alignment in CDCAlignment based on computed alignment of layers
      virtual void readFromResult(std::vector<std::tuple<unsigned short, unsigned short, unsigned short, double>>& result,
                                  GlobalParamVector& thisGlobalParamVector) override final;

    };

    /// Very tentative interface for VXD
    class VXDGlobalParamInterface : public IGlobalParamInterface {
    public:
      /// Very tentaive function: not yet used
      virtual void setupAlignmentHierarchy(GlobalDerivativesHierarchy& hierarchy) override final;
    };

    /// Base accessor class to store different DBObjects e.g. in a vector but allow
    /// to access their stored global parameters
    class GlobalParamSetAccess {
    public:
      /// Constructor
      GlobalParamSetAccess() {}
      /// Virtual destructor (base class, but with no members)
      virtual ~GlobalParamSetAccess() {}
      /// implement: Get the global unique id of DBObject
      virtual unsigned short getGlobalUniqueID() const = 0;
      /// implement: get a global parameter value based on the element number in the DBObject and its parameter
      /// number. Might be that element/param is zero. Check the meaning in DB object
      virtual double getGlobalParam(unsigned short, unsigned short) = 0;
      /// implemtn: set a global param value by element and param number
      virtual void setGlobalParam(double, unsigned short, unsigned short) = 0;
      /// implement: generate list of all global parameters in the DBObject
      virtual std::vector<std::pair<unsigned short, unsigned short>> listGlobalParams() = 0;

      /// Sum value of global @param element @param param with a @param correction
      virtual void updateGlobalParam(double correction, unsigned short element, unsigned short param) {setGlobalParam(getGlobalParam(element, param) + correction, element, param);}
      /// Get a raw pointer to the internal DB object (can be nullptr if not yet constructed)
      /// Use with caution - always check for nullptr
      virtual TObject* getDBObj() = 0;
      /// Get the default DBObject name used by datastore
      virtual std::string getDefaultName() const = 0;
      /// Release the object from internal unique_ptr to be managed elsewhere
      /// Useful to pass it to be stored in DB (and thus later deleted by framework)
      virtual TObject* releaseObject() = 0;
      /// Load the content (by copying obj retrieved from DB) for a given exp/run/event
      virtual void loadFromDB(EventMetaData emd) = 0;
      /// Load using DBObjPtr<DBObjType> which uses current EventMetaData to load valid constants
      virtual void loadFromDBObjPtr() = 0;

      /// Construct the internal DBObject
      virtual void construct() = 0;
      /// Has the internal DBObject been already constructed
      /// The object is constructed at latest on first access to its parameters
      virtual bool isConstructed() const = 0;

      /// Has the object been updated in DB since GlobalParamSet<...> has been constructed? Since last call to this function with resetChangedState=true
      virtual bool hasBeenChangedInDB(bool resetChangedState = true) final {auto tmp = m_hasBeenChangedInDB; if (resetChangedState) m_hasBeenChangedInDB = false; return tmp;}
      /// Function to call when object has been changed in DB since last loaded
      virtual void callbackDB() {m_hasBeenChangedInDB = true;}

      /// Is this set of given type?
      /// @return true if stored set is of given type
      template<class DBObjType>
      bool is() const {return DBObjType::getGlobalUniqueID() == getGlobalUniqueID();}

      /// Is this set empty or otherwise 'invalid'
      /// to identify null sets, end of hierarchy
      bool empty()
      {
        // We consider this to be the definition of empty
        if (is<EmptyGlobalParamSet>())
          return true;
        // This is by convention (also used in hierarchy)
        if (getGlobalUniqueID() == 0)
          return true;
        // This actually constructs the object using Default constructor if not
        // previously loaded from DB (or using construct)
        // TODO this is still not fully impelemnted by DB objects
        if (listGlobalParams().empty())
          return true;
        return false;
      }

      /// Tentative operator to quickly check the set
      /// Returns true when the set is not empty() and the DB
      /// object is constructed and can be accessed
      operator bool()
      {
        return not empty() and isConstructed();
      }

    protected:
      /// Flag for object changed in DB
      bool m_hasBeenChangedInDB {false};
    };

    /// Template class for generic access to DB objects
    ///
    /// Use to access any global calibration DB object, e.g.
    ///
    /// GlobalParamSet<BeamParameters> params;
    /// params.setGlobalParam(0.0012, 0, 1);
    ///
    /// will set X-postion of BeamParameters vertex to 0.0012
    template<class DBObjType>
    class GlobalParamSet : public GlobalParamSetAccess {
    public:
      /// Constructor. Sets callback for DB object changes
      //       explicit GlobalParamSet(bool autoConstruct = false) {
      GlobalParamSet()
      {
        //TODO: re-enable
        //DBObjPtr<DBObjType> dbObject;
        //dbObject.addCallback(this, &GlobalParamSet<DBObjType>::callbackDB);
        // if (autoConstruct)
        //  construct();
      }
      /// Destructor
      ~GlobalParamSet() {m_object.reset();}

      /// The DB object unique id in global calibration
      virtual unsigned short getGlobalUniqueID() const override final {return DBObjType::getGlobalUniqueID();}
      /// Get global parameter of the DB object by its element and parameter number
      /// Note this is not const, it might need to construct the object
      virtual double getGlobalParam(unsigned short element, unsigned short param) override final {ensureConstructed(); return m_object->getGlobalParam(element, param);}
      /// Set global parameter of the DB object by its element and parameter number
      virtual void setGlobalParam(double value, unsigned short element, unsigned short param) override final {ensureConstructed(); m_object->setGlobalParam(value, element, param);}
      /// List global parameters in this DB object
      virtual std::vector<std::pair<unsigned short, unsigned short>> listGlobalParams() override final {ensureConstructed(); return m_object->listGlobalParams();}
      /// Get the DB object default name used by datastore
      virtual std::string getDefaultName() const override final {return DataStore::objectName<DBObjType>("");}

      /// Get the raw pointer to the stored object
      /// WARNING: Use with caution if you really need to access the internal object
      virtual TObject* getDBObj() override final
      {
        if (!m_object)
          return nullptr;

        return m_object.get();
      }

      /// Assignment operator
      GlobalParamSet<DBObjType>& operator=(const GlobalParamSet<DBObjType>& other)
      {
        // check for self-assignment
        if (&other == this)
          return *this;
        m_hasBeenChangedInDB = other.m_hasBeenChangedInDB;
        // Make new unique ptr to a copy of the other internal object
        if (other.m_object)
          m_object.reset(new DBObjType(*(static_cast<DBObjType*>(other.m_object.get()))));
        return *this;
      }

      /// Release the DB object from the internal unique pointer
      /// to be managed elsewhere (e.g. stored to the DB and deleted by it)
      virtual TObject* releaseObject() override final
      {
        //ensureConstructed();
        return m_object.release();
      }

      /// Load content of the object using DBObjPtr<DBObjType>
      /// which will try to load object valid for current EventMetaData
      /// Also resets if the object has been changed
      virtual void loadFromDBObjPtr() override final
      {
        DBObjPtr<DBObjType> dbObject;
        if (dbObject) {
          m_object.reset(new DBObjType(*dbObject));
          hasBeenChangedInDB();  // will set m_hasBeenChangedInDB to false, ignore return value;
        }
      }

      /// Load content of the object valid at given event from DB
      /// Also resets if the object has been changed
      virtual void loadFromDB(EventMetaData event) override final
      {

        std::list<Database::DBQuery> query = {Database::DBQuery(getDefaultName())};

        Database::Instance().getData(event, query);
        // TODO: do not make copy? is this safe with objects with private members made of pointers to other objects?
        if (!query.front().object) {
          B2ERROR("Could not fetch object " << getDefaultName() << " from DB.");
          return;
        }
        m_object.reset(new DBObjType(*(dynamic_cast<DBObjType*>(query.front().object))));
        hasBeenChangedInDB();  // will set m_hasBeenChangedInDB to false, ignore return value;

      }

      /// Construct the internal object using default constructor
      virtual void construct() override final {m_object.reset(new DBObjType());}
      /// Is the internal object already constructed?
      /// we construct the object on first access to the stored values
      virtual bool isConstructed() const override final {return !!m_object;}

      /// Function to call when object has been changed in DB since last loaded
      /// TODO Hide and find the class which calls this to make it friend
      virtual void callbackDB() override final {GlobalParamSetAccess::callbackDB();}


    private:
      /// The internal DB object
      std::unique_ptr<DBObjType> m_object {};
      /// Function to construct the object if not already done
      void ensureConstructed() {if (!m_object) construct();}
    };

    template <>
    unsigned short GlobalParamSet<BeamParameters>::getGlobalUniqueID() const;
    /// The DB object unique id in global calibration
    template <>
    unsigned short GlobalParamSet<BeamParameters>::getGlobalUniqueID() const;
    /// Get global parameter of the DB object by its element and parameter number
    /// Note this is not const, it might need to construct the object
    template <>
    double GlobalParamSet<BeamParameters>::getGlobalParam(unsigned short element, unsigned short param);
    /// Set global parameter of the DB object by its element and parameter number
    template <>
    void GlobalParamSet<BeamParameters>::setGlobalParam(double value, unsigned short element, unsigned short param);
    /// List global parameters in this DB object
    template <>
    std::vector<std::pair<unsigned short, unsigned short>> GlobalParamSet<BeamParameters>::listGlobalParams();




    /// The central user class to manipulate any global constant in any DB object
    /// Used to retrieve global parameters from database and access the for update.
    /// Finally all objects can be released for storing in the DB.
    /// NOTE the objects are not constructed (an thus quite some memory is saved) until
    /// you access them. But the monitoring of changes in DB starts since addDBObj<...>()
    /// TODO correct constness to not construct on listGlobalParams()
    class GlobalParamVector {
    public:
      /// An empty set to which reference is returned if DB object not found
      /// Also used e.g. to terminate hierarchy
      GlobalParamSet<EmptyGlobalParamSet> c_emptyGlobalParamSet {};

      /// Constructor
      /// @param components vector of string with DB objects default names in the global vector
      /// If empty, any component is included by default.
      explicit GlobalParamVector(std::vector<std::string> components = {});

      /// Destructor
      ~GlobalParamVector() {}

      /// Construct all DB objects using default constructors (should be filled with zeros)
      void construct();

      /// TODO: operator += to sum two global vectors
      GlobalParamVector& operator+=(GlobalParamVector& rhs)
      {
        // this += rhs
        // TODO: needs full implementation of listing in DBObjects
        for (auto uid_element_param : listGlobalParams()) {
          double delta = rhs.getGlobalParam(std::get<0>(uid_element_param), std::get<1>(uid_element_param), std::get<2>(uid_element_param));
          updateGlobalParam(delta, std::get<0>(uid_element_param), std::get<1>(uid_element_param), std::get<2>(uid_element_param));
        }
        return *this;
      }

      /// Add a generic interface (cannot be disabled by 'components' in constructor)
      void addSubDetectorInterface(std::shared_ptr<IGlobalParamInterface> interface = {})
      {
        if (interface) {
          if (std::find(m_subDetectorInterfacesVector.begin(), m_subDetectorInterfacesVector.end(),
                        interface) == m_subDetectorInterfacesVector.end())
            m_subDetectorInterfacesVector.push_back(interface);
        }
      }

      /// Add a DB object to the vector, optionally with interface
      /// @param interface shared pointer to an interface added only if this DB object
      /// is listed in 'components' or 'components' not specified
      /// Does nothing if DB object type already added
      template <class DBObjType>
      void addDBObj(std::shared_ptr<IGlobalParamInterface> interface = {})
      {
        if (m_components.empty()
            or std::find(m_components.begin(), m_components.end(), DataStore::objectName<DBObjType>("")) != m_components.end()) {
          m_vector.insert(std::make_pair(DBObjType::getGlobalUniqueID(),
                                         std::unique_ptr<GlobalParamSet<DBObjType>>(new GlobalParamSet<DBObjType>)
                                        ));
          // NOTE: Components disabled this way also disable added interfaces (e.g. if geometry would be needed to load)
          // NOTE: add generic interfaces only once by addSubDetectorInterface(...)
          addSubDetectorInterface(interface);
        }
      }

      /// Has any DB object in vector changed from last call to this function
      /// @param subset set of unique ids of objects we are interested
      bool hasBeenChangedInDB(const std::set<unsigned short>& subset, bool resetChangedState = true)
      {
        bool changed = false;

        for (auto& uID_DBObj : m_vector) {
          if (!subset.empty() and subset.find(uID_DBObj.first) != subset.end())
            continue;
          if (uID_DBObj.second->hasBeenChangedInDB(resetChangedState)) {
            changed = true;
            break;
          }
        }

        return changed;
      }
      /// Const version which does not change the state
      bool hasBeenChangedInDB(const std::set<unsigned short>& subset = {}) {return hasBeenChangedInDB(subset, false);}

      /// Add 'difference' to param value
      /// @param difference value to add to current value
      /// @param uniqueID uinique id of the DB object
      /// @param element id of the element with params inside DB object
      /// @param param id of the parameter of the element
      void updateGlobalParam(double difference, unsigned short uniqueID, unsigned short element, unsigned short param);

      /// Set param value
      /// @param value value to set
      /// @param uniqueID uinique id of the DB object
      /// @param element id of the element with params inside DB object
      /// @param param id of the parameter of the element
      void setGlobalParam(double value, unsigned short uniqueID, unsigned short element, unsigned short param);

      /// Get parameter value
      /// @param uniqueID uinique id of the DB object
      /// @param element id of the element with params inside DB object
      /// @param param id of the parameter of the element
      double getGlobalParam(unsigned short uniqueID, unsigned short element, unsigned short param);

      /// Create vector of all global parameters maintained by GlobalParamVector
      std::vector<std::tuple<unsigned short, unsigned short, unsigned short>> listGlobalParams();

      /// Get the vector of raw pointers to DB objects
      /// Caller takes the ownership of the objects and has to delete them
      /// Use for passing the objects to store in DB (not deleted after GlobalParamVector goes out of scope)
      /// @param onlyConstructed if true, only returns objects which have been actually constructed
      /// Objects are not constructed until accessed or copied from DB (or constructed explicitly)
      std::vector<TObject*> releaseObjects(bool onlyConstructed = true);

      /// Load content of all objects in vector with values valid at current EventMetaData
      /// Uses DBObjPtr<DBObjType> to access current data
      void loadFromDB();
      /// Load content of all objects in vector with values valid for given exp/run/event
      void loadFromDB(const EventMetaData& event);

      /// Function calling interfaces to do post processing after reading millepede result into GlobalParamVector
      void postReadFromResult(std::vector<std::tuple<unsigned short, unsigned short, unsigned short, double>>& result);
      /// Function calling interfaces to build alignment hierarchy after constants changed in DB
      void postHierarchyChanged(GlobalDerivativesHierarchy& hierarchy);

      /// Get the raw pointer toa stored object
      /// WARNING: Use with caution, returns nullptr if object not registered (e.g. disabled via 'components')
      template<class DBObjType>
      DBObjType* getDBObj()
      {
        if (m_vector.find(DBObjType::getGlobalUniqueID()) == m_vector.end())
          return nullptr;

        return static_cast<DBObjType*>(m_vector[DBObjType::getGlobalUniqueID()]->getDBObj());
      }

      /// Get reference to the underlying DB object container
      /// You should check the container
      /// @return GlobalParamSet<DBObjType> or GlobalParamSet<EmptyGlobalParamSet> if DBObjType not
      /// in global vector (e.g. because disabled via 'components')
      template<class DBObjType>
      GlobalParamSetAccess& getGlobalParamSet()
      {
        if (m_vector.find(DBObjType::getGlobalUniqueID()) == m_vector.end())
          return c_emptyGlobalParamSet;

        return static_cast<GlobalParamSet<DBObjType>&>(*m_vector[DBObjType::getGlobalUniqueID()]);
      }

      /// Get map of all contained parameter sets, key = unique id of the set (DB object)
      const std::map<unsigned short, std::unique_ptr<GlobalParamSetAccess>>& getGlobalParamSets() const
      {
        return m_vector;
      }

      /// Get the vector of added interfaces to subdetectors
      const std::vector<std::shared_ptr<IGlobalParamInterface>>& getSubDetectorInterfaces() const
      {
        return m_subDetectorInterfacesVector;
      }

      /// Get set of unique ids of all db objects considered
      std::set<unsigned short> getComponentsIDs() const
      {
        std::set<unsigned short> result;
        for (auto& id_obj : m_vector)
          result.insert(id_obj.first);
        return result;
      }
    private:
      /// The vector (well, actually a map) of DB objects
      std::map<unsigned short, std::unique_ptr<GlobalParamSetAccess>> m_vector {};
      /// Vector of interfaces to run at specific points in the workflow
      std::vector<std::shared_ptr<IGlobalParamInterface>> m_subDetectorInterfacesVector {};
      /// Vector of names of DB objects to consider in the vector - if not here and non-empy, ignored by addDBObj()
      std::vector<std::string> m_components {};
      /// Vector of UniqueIDs of DB objects to consider in the vector
      std::set<unsigned short> m_componentsIDs {};
    };
  }
}