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

#include <memory>

#include <framework/logging/Logger.h>

#include <framework/dbobjects/BeamParameters.h>
#include <alignment/dbobjects/VXDAlignment.h>
#include <alignment/dbobjects/CDCCalibration.h>
#include <alignment/dbobjects/BKLMAlignment.h>
#include <eklm/dbobjects/EKLMAlignment.h>

#include <cdc/dbobjects/CDCTimeZeros.h>
#include <cdc/dbobjects/CDCTimeWalks.h>
#include <cdc/dbobjects/CDCAlignment.h>

#include <framework/database/Database.h>

#include <alignment/Hierarchy.h>

#include <framework/database/DBObjPtr.h>

namespace Belle2 {
  namespace alignment {
    class GlobalParamVector;

    class IGlobalParamInterface {
    public:
      //REVISE: does have def implementation...
      virtual void readFromResult(std::vector<std::tuple<unsigned short, unsigned short, unsigned short, double>>&,
                                  GlobalParamVector&) {};

      virtual void setupAlignmentHierarchy(RigidBodyHierarchy&) {}
    };

    class CDCGlobalParamInterface : public IGlobalParamInterface {
    public:
      virtual void readFromResult(std::vector<std::tuple<unsigned short, unsigned short, unsigned short, double>>& result,
                                  GlobalParamVector& thisGlobalParamVector) final;

    };

    class VXDGlobalParamInterface : public IGlobalParamInterface {
    public:
      virtual void setupAlignmentHierarchy(RigidBodyHierarchy& hierarchy) final;
    };

    class GlobalParamSetAccess {
    public:
      GlobalParamSetAccess() {}
      virtual ~GlobalParamSetAccess() {}
      virtual unsigned short getGlobalUniqueID() = 0;
      virtual double getGlobalParam(unsigned short, unsigned short) = 0;
      virtual void setGlobalParam(double, unsigned short, unsigned short) = 0;
      virtual std::vector<std::pair<unsigned short, unsigned short>> listGlobalParams() = 0;

      void updateGlobalParam(double correction, unsigned short element, unsigned short param) {setGlobalParam(getGlobalParam(element, param) + correction, element, param);}

      virtual TObject* getDBObj() {return nullptr;}
      virtual std::string getDefaultName() = 0;
      virtual TObject* releaseObject() = 0;
      virtual void loadFromDB(EventMetaData emd) = 0;

      virtual void construct() = 0;
      virtual bool isConstructed() = 0;
      virtual bool hasBeenChanged() = 0;

      virtual void setHasBeenChanged(bool) = 0;
      virtual void hasChanged() = 0;

      virtual void readFromResult(std::vector<std::tuple<unsigned short, unsigned short, unsigned short, double>>&) {};
    };

    template<class DBObjType>
    class GlobalParamSet : public GlobalParamSetAccess {
    public:
      GlobalParamSet() : m_dbObject("")
      {
        m_dbObject.addCallback(this, &GlobalParamSet<DBObjType>::hasChanged);
      }
      // TODO: resetHasBeenCHanged
      ~GlobalParamSet() {m_object.reset();}

      virtual unsigned short getGlobalUniqueID() final {return DBObjType::getGlobalUniqueID();}
      virtual double getGlobalParam(unsigned short element, unsigned short param) final {ensureConstructed(); return m_object->getGlobalParam(element, param);}
      virtual void setGlobalParam(double value, unsigned short element, unsigned short param) final {ensureConstructed(); m_object->setGlobalParam(value, element, param); setHasBeenChanged(true);}
      virtual std::vector<std::pair<unsigned short, unsigned short>> listGlobalParams() final {ensureConstructed(); return m_object->listGlobalParams();}

                                                                  virtual std::string getDefaultName() final {return DataStore::objectName<DBObjType>("");}
      virtual TObject* getDbObj() final {
        if (!m_object)
          return nullptr;

        return m_object.get();
      }

      virtual TObject* releaseObject() final {
        //ensureConstructed();
        return m_object.release();
      }

      void loadFromDBObjPtr()
      {
        if (m_dbObject) {
          m_object.reset(new DBObjType(*m_dbObject));
        }
      }
      virtual void loadFromDB(EventMetaData event) final {

        std::list<Database::DBQuery> query = {Database::DBQuery(getDefaultName())};

        Database::Instance().getData(event, query);
        // TODO: do not make copy? is this safe with objects with private members made of pointers to other objects?
        if (!query.front().object)
        {
          B2ERROR("Could not fetch object " << getDefaultName() << " from DB.");
          return;
        }
        m_object.reset(new DBObjType(*(dynamic_cast<DBObjType*>(query.front().object))));

      }
      virtual void construct() final {m_object.reset(new DBObjType());}
      virtual bool isConstructed() final {return !!m_object;}
      virtual bool hasBeenChanged() final {return m_hasBeenChanged;}
      virtual void hasChanged() final {setHasBeenChanged(true);}

      virtual void setHasBeenChanged(bool hasBeenChanged = true) {m_hasBeenChanged = hasBeenChanged;}

    private:
      bool m_hasBeenChanged {false};
      std::unique_ptr<DBObjType> m_object {};
      DBObjPtr<DBObjType> m_dbObject {""};
      void ensureConstructed() {if (!m_object) construct();}
    };

    class GlobalParamVector {
    public:
      explicit GlobalParamVector(std::vector<std::string> components = {});

      ~GlobalParamVector()
      {
        clear();
      }

      void clear()
      {
        m_vector.clear();
        m_subDetectorInterfacesVector.clear();
        m_components.clear();
      }

      void addSubDetectorInterface(std::shared_ptr<IGlobalParamInterface> interface = {})
      {
        if (interface) {
          if (std::find(m_subDetectorInterfacesVector.begin(), m_subDetectorInterfacesVector.end(),
                        interface) == m_subDetectorInterfacesVector.end())
            m_subDetectorInterfacesVector.push_back(interface);
        }
      }

      template <class DBObjType>
      void addDBObj(std::shared_ptr<IGlobalParamInterface> interface = {})
      {
        if (m_components.empty()
            or std::find(m_components.begin(), m_components.end(), DBStore::objectName<DBObjType>("")) == m_components.end()) {
          m_vector.insert(std::make_pair(DBObjType::getGlobalUniqueID(),
                                         std::unique_ptr<GlobalParamSet<DBObjType>>(new GlobalParamSet<DBObjType>)
                                        ));
          // NOTE: Components disabled this way also disable added interfaces (e.g. if geometry would be needed to load)
          // NOTE: add generic interfaces only once by addSubDetectorInterface(...)
          addSubDetectorInterface(interface);
        }
      }

      bool hasBeenUpdated()
      {
        return false;
      }

      void updateGlobalParam(double difference, unsigned short uniqueID, unsigned short element, unsigned short param);

      void setGlobalParam(double value, unsigned short uniqueID, unsigned short element, unsigned short param);

      double getGlobalParam(unsigned short uniqueID, unsigned short element, unsigned short param);

      std::vector<std::tuple<unsigned short, unsigned short, unsigned short>> listGlobalParams();

      /// Get the vector of raw pointers to DB objects
      /// Caller takes the ownership of the objects and has to delete them
      /// Use for passing the objects to store in DB (not deleted after GlobalParamVector goes out of scope)
      std::vector<TObject*> releaseObjects(bool onlyChanged = true);

      void loadFromDB(EventMetaData event);

      void postReadFromResult(std::vector<std::tuple<unsigned short, unsigned short, unsigned short, double>>& result);

      template<class DBObjType>
      DBObjType* getDBObj()
      {
        if (m_vector.find(DBObjType::getGlobalUniqueID()) == m_vector.end())
          return nullptr;

        return static_cast<DBObjType*>(m_vector[DBObjType::getGlobalUniqueID()]->getDBObj());
      }

    private:
      std::map<unsigned short, std::unique_ptr<GlobalParamSetAccess>> m_vector {};
      std::vector<std::shared_ptr<IGlobalParamInterface>> m_subDetectorInterfacesVector {};

      std::vector<std::string> m_components {};
    };


    /// Set with no parameters, terminates hierarchy etc.
    class EmptyGlobaParamSet : public TObject {
    public:
      /// Get global unique id = 0
      static unsigned short getGlobalUniqueID() {return 0;}
      /// There no params stored here, returns always 0.
      double getGlobalParam(unsigned short, unsigned short) {return 0.;}
      /// No parameters to set. Does nothing
      void setGlobalParam(double, unsigned short, unsigned short) {}
      /// No parameters, returns empty vector
      std::vector<std::pair<unsigned short, unsigned short>> listGlobalParams() {return {};}
    };
  }
}