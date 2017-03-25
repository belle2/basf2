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

#include <framework/logging/Logger.h>

#include <framework/dbobjects/BeamParameters.h>
#include <alignment/dbobjects/VXDAlignment.h>
#include <alignment/dbobjects/CDCCalibration.h>
#include <alignment/dbobjects/BKLMAlignment.h>
#include <eklm/dbobjects/EKLMAlignment.h>

#include <framework/database/Database.h>

namespace Belle2 {
  namespace alignment {

    class GlobalParamSetAccess {
    public:
      virtual unsigned short getGlobalUniqueID() = 0;
      virtual double getGlobalParam(unsigned short, unsigned short) = 0;
      virtual void setGlobalParam(double, unsigned short, unsigned short) = 0;
      virtual std::vector<std::pair<unsigned short, unsigned short>> listGlobalParams() = 0;

      virtual std::string getDefaultName() = 0;
      virtual TObject* releaseObject() = 0;
      virtual void loadFromDB(EventMetaData emd) = 0;

      virtual void construct() = 0;
    };

    template<class DBObjType>
    class GlobalParamSet : public GlobalParamSetAccess {
    public:
      GlobalParamSet() {}
      ~GlobalParamSet() {m_object.reset();}

      virtual unsigned short getGlobalUniqueID() final {return DBObjType::getGlobalUniqueID();}
      virtual double getGlobalParam(unsigned short element, unsigned short param) final {ensureConstructed(); return m_object->getGlobalParam(element, param);}
      virtual void setGlobalParam(double value, unsigned short element, unsigned short param) final {ensureConstructed(); m_object->setGlobalParam(value, element, param);}
      virtual std::vector<std::pair<unsigned short, unsigned short>> listGlobalParams() final {ensureConstructed(); return m_object->listGlobalParams();}

                                                                  virtual std::string getDefaultName() final {return DBStore::objectName<DBObjType>("");}
                                                                  virtual TObject* releaseObject() final {ensureConstructed(); return m_object.release();}
      virtual void loadFromDB(EventMetaData event) final {
        std::list<Database::DBQuery> query = {Database::DBQuery(getDefaultName())};

        Database::Instance().getData(event, query);
        // TODO: do not make copy? is this safe with objects with private members made of pointers to other objects?
        m_object.reset(new DBObjType(*(dynamic_cast<DBObjType*>(query.front().object))));
      }


      virtual void construct() final {m_object.reset(new DBObjType());}
    private:
      std::unique_ptr<DBObjType> m_object {};
      void ensureConstructed() {if (!m_object) construct();}
    };

    class GlobalParamVector {
    public:
      GlobalParamVector()
      {
        addDBObj<BeamParameters>();
        addDBObj<VXDAlignment>();
        addDBObj<CDCCalibration>();
        addDBObj<BKLMAlignment>();
        addDBObj<EKLMAlignment>();
      }

      ~GlobalParamVector()
      {
        m_vector.clear();
      }

      template <class DBObjType>
      void addDBObj()
      {
        m_vector.insert(std::make_pair(DBObjType::getGlobalUniqueID(),
                                       std::unique_ptr<GlobalParamSet<DBObjType>>(new GlobalParamSet<DBObjType>)
                                      ));
      }

//       std::list<Database::DBQuery> getDBQueryList() {
//         std::list<Database::DBQuery> result;
//         for (auto & uID_DBObj : m_vector) {
//           result.push_back(Database::DBQuery(uID_DBObj.second->getDefaultName()));
//         }
//         return result;
//       }
//

      void updateGlobalParam(double difference, unsigned short uniqueID, unsigned short element, unsigned short param)
      {
        auto prev = getGlobalParam(uniqueID, element, param);
        setGlobalParam(prev + difference, uniqueID, element, param);
      }

      void setGlobalParam(double value, unsigned short uniqueID, unsigned short element, unsigned short param)
      {
        auto dbObj = m_vector.find(uniqueID);
        if (dbObj != m_vector.end()) {
          dbObj->second->setGlobalParam(value, element, param);
        } else {
          B2WARNING("Did not found DB object with unique id " << uniqueID << " in global vector. Cannot set value");
        }
      }

      double getGlobalParam(unsigned short uniqueID, unsigned short element, unsigned short param)
      {
        auto dbObj = m_vector.find(uniqueID);
        if (dbObj != m_vector.end()) {
          return dbObj->second->getGlobalParam(element, param);
        } else {
          B2WARNING("Did not found DB object with unique id " << uniqueID << " in global vector. Returning 0.");
          return 0.;
        }
      }

      std::vector<std::tuple<unsigned short, unsigned short, unsigned short>> listGlobalParams()
      {
        std::vector<std::tuple<unsigned short, unsigned short, unsigned short>> params;
        for (auto& uID_DBObj : m_vector) {
          for (auto element_param : uID_DBObj.second->listGlobalParams()) {
            params.push_back({uID_DBObj.first, element_param.first, element_param.second});
          }
        }
        return params;
      }

      /// Get the vector of raw pointers to DB objects
      /// Caller takes the ownership of the objects and has to delete them
      /// Use for passing the objects to store in DB (not deleted after GlobalParamVector goes out of scope)
      std::vector<TObject*> releaseObjects()
      {
        std::vector<TObject*> result;
        for (auto& uID_DBObj : m_vector) {
          result.push_back({uID_DBObj.second->releaseObject()});
        }
        return result;
      }

      void loadFromDB(EventMetaData event)
      {
        for (auto& uID_DBObj : m_vector) {
          uID_DBObj.second->loadFromDB(event);
        }
      }

    private:
      std::map<unsigned short, std::unique_ptr<GlobalParamSetAccess>> m_vector {};
    };


    /// Set with no parameters, terminates hierarchy etc.
    class EmptyGlobaParamSet {
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