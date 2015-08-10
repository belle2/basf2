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

#include <vxd/dataobjects/VxdID.h>
#include <boost/property_tree/ptree.hpp>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>

#include <TFile.h>
#include <framework/pcore/RootMergeable.h>
#include <framework/datastore/StoreObjPtr.h>
#include <alignment/dataobjects/MilleData.h>

using namespace std;
namespace Belle2 {
  /**
   */
  class ManagedStorableList {
  public:
    typedef std::function< void(TFile*) > write_prototype;
    typedef std::function< void() >       reset_prototype;
    typedef std::function< bool(TFile*) > read_prototype;

    struct StorableFunctors {
      write_prototype write;
      reset_prototype reset;
      read_prototype read;
    };

    typedef pair<string, StorableFunctors> StorablePrototypeEntry;
    typedef vector<StorablePrototypeEntry> StorablePrototypeMap;

    void writeAll(TFile* file)
    {
      for (StorablePrototypeEntry& entry : theMap) {
        entry.second.write(file);
      }
    }
    void resetAll()
    {
      for (StorablePrototypeEntry& entry : theMap) {
        entry.second.reset();
      }
    }
    bool readAll(TFile* file)
    {
      for (StorablePrototypeEntry& entry : theMap) {
        entry.second.read(file);
      }
      return true;
    }

    void addObject(string name, StorableFunctors& functors)
    {
      theMap.push_back(make_pair(name, functors));
    }

    //! Manage RootMergeable< some ROOT histogram or tree > defined as module private members
    template<class T, class ...Args> void manage(StoreObjPtr< RootMergeable<T> >& obj, Args&& ... params)
    {
      string name = m_baseName + "_" + obj.getName();
      obj.registerInDataStore(name, DataStore::c_DontWriteOut);
      if (!obj.isValid()) {
        obj.construct(std::forward<Args>(params)...);
        obj->get().SetName(name.c_str());
        obj->get().SetDirectory(nullptr);
      }


      write_prototype write =
      [ = ](TFile * file) -> void {
        StoreObjPtr< RootMergeable<T> > object(name, DataStore::c_Persistent);
        object->write(file);
      };

      reset_prototype reset =
      [ = ]() -> void {
        StoreObjPtr< RootMergeable<T> > object(name, DataStore::c_Persistent);
        object->clear();
      };

      read_prototype read =
      [ = ](TFile * file) -> bool {
        if (!file || !file->IsOpen())
          return false;
        file->cd();
        StoreObjPtr< RootMergeable<T> > object(name, DataStore::c_Persistent);
        object->get().Read(object->get().GetName());
        object->get().SetDirectory(nullptr);
        return true;
      };

      StorableFunctors functors;
      functors.read = read;
      functors.reset = reset;
      functors.write = write;

      addObject(name, functors);
    }


    /**
     * @brief Manage all other mergeable classes
     *        Your object has to be initialised with name and durability c_Persistent before
     *        (for RootMergeables, use a special template, which also sets histogram/tree name to that of DataStore object)
     * @param obj Your private variable with StoreObjPtr<T>
     * @param params Parameters to be passed to object constructor
     * @return void
     */
    template<class T, class ...Args> void manage(StoreObjPtr<T>& obj, Args&& ... params)
    {
      string name = m_baseName + "_" + obj.getName();
      obj.registerInDataStore(name, DataStore::c_DontWriteOut);
      if (!obj.isValid()) {
        obj.construct(std::forward<Args>(params)...);
      }
      write_prototype write =
      [ = ](TFile * file) -> void {
        StoreObjPtr< T > object(name, DataStore::c_Persistent);
        file->cd();
        object->Write(name.c_str());
      };
      reset_prototype reset =
      [ = ]() -> void {
        StoreObjPtr< T > object(name, DataStore::c_Persistent);
        object->clear();
      };
      read_prototype read =
      [ = ](TFile * file) -> bool {
        if (!file || !file->IsOpen())
          return false;
        file->cd();
        StoreObjPtr< T > object(name, DataStore::c_Persistent);
        object->Read(name.c_str());
        return true;
      };
      StorableFunctors functors;
      functors.read = read;
      functors.reset = reset;
      functors.write = write;
      theMap.push_back(make_pair(name, functors));
    }

    void test()
    {
      StoreObjPtr<RootMergeable<TH1D> > obj("myname");
      manage<TH1D>(obj);

      //manage< StoreObjPtr<RootMergeable<TH1D> > >("my_first_histogram");
    }

    void setBaseName(string basename) { m_baseName = basename; }

  private:
    StorablePrototypeMap theMap = {};
    string m_baseName = "";

  };

} // namespace Belle2


