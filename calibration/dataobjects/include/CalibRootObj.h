#pragma once

#include <string>
#include <vector>

#include <framework/logging/Logger.h>
#include <framework/pcore/Mergeable.h>

#include <TList.h>
#include <TTree.h>
#include <TH1.h>
#include <TH2.h>
#include <alignment/dataobjects/MilleData.h>
#include <framework/pcore/MapMergeable.h>
#include <calibration/dataobjects/RunRange.h>


namespace Belle2 {

  /**
   * @brief Class to implement run dependence
   * into mergeable ROOT-compatible objects.
   *
   * TODO: go to smart pointers (we have ROOT6!)
   *
   */
  template<class T>
  class CalibRootObj : public Mergeable {

  public:
    //typedef std::pair<int,int> KeyType;
    /// The key type in the map - IOV or other representation
    typedef std::string KeyType;

    /// Constructor
    CalibRootObj() : Mergeable() {};

    /// Destructor
    virtual ~CalibRootObj()
    {
      removeSideEffects();
      for (auto obj : m_objects)
        delete obj;

      m_objects.clear();
      m_iovs.clear();

      if (m_object) {
        m_object->SetDirectory(nullptr);
        delete m_object;
        m_object = nullptr;
      }
    }

    /**
     * @brief Implementation of merging. Merges
     * corresponding objects (for each iov separately).
     *
     * If lists of objects differ,
     * additional objects are added to the list
     * of output object before merging.
     *
     * @param other Object to be merged into this one
     * @return void
     */
    virtual void merge(const Mergeable* other) override
    {
      auto* data = static_cast<const CalibRootObj<T>*>(other);

      // Call GetObject for each IOV to ensure all
      // objects are created (merges list of objects...)
      for (unsigned int i = 0; i < data->m_iovs.size(); i++) {
        getObject(data->m_iovs[i]);
      }

      // Now merge content of corresponding objects
      // TODO: Ugly...
      for (unsigned int i = 0; i < m_iovs.size(); i++) {
        for (unsigned int j = 0; j < data->m_iovs.size(); j++) {
          if (m_iovs[i] == data->m_iovs[j]) {
            TList list;
            list.SetOwner(false);
            list.Add(data->m_objects[j]);
            m_objects[i]->Merge(&list);
          }
        }
      }
    }

    /**
     * @brief Implementation of clearing
     * Objects for each iov are cleared.
     * The object list is kept!
     *
     * @return void
     */
    virtual void clear() override
    {
      for (unsigned int i = 0; i < m_objects.size(); i++) {
        // Get ROOT class information
        //TClass *cl = m_objects[i]->IsA();
        // Reset contents of a ROOT object
        //if (cl && cl->GetMethodWithPrototype("Reset", "Option_t*"))
        //  m_objects[i]->Execute("Reset", "");
        // Reset contents of Belle2::Mergeable object
        //else if (cl && cl->GetMethodWithPrototype("clear", ""))
        //  m_objects[i]->Execute("clear", "");
        m_objects[i]->Reset();
      }
    }

    /**
     * @brief Implementation of the
     * "ugly little method" for disconnecting
     * histos in worker processes from files.
     *
     * @return void
     */
    virtual void removeSideEffects() override
    {
      for (unsigned int i = 0; i < m_objects.size(); i++) {
        // Get ROOT class information
        //TClass *cl = m_objects[i]->IsA();
        // Disconnect object from any files for ROOT objects
        //if (cl && cl->GetMethodWithPrototype("SetDirectory", "TDirectory*"))
        //  m_objects[i]->Execute("SetDirectory", "0");
        m_objects[i]->SetDirectory(nullptr);
      }
    }

    /**
     * @brief User constructor
     * @param object Pointer to prepared ROOT object
     * (TH1F, TTree etc.) to be stored as template for
     * later automatic object creation.
     *
     * Takes ownership of object (do not delete it!)
     */
    explicit CalibRootObj(T* object) : Mergeable()
    {
      if (m_object) {
        delete m_object;
        m_object = nullptr;
      }

      object->SetDirectory(nullptr);
      m_object = object;
    }

    /**
     * @brief Construct an object for new iov.
     * The stored template object is copied and
     * new one is created for the iov.
     *
     * Can be called also after initialize in any
     * process. The newly created objects are
     * added into output object list and here objects
     * with corresponding IOV are merged.
     *
     * @param iov IOV identifier of the new object
     * @return void
     */
    void constructObject(const KeyType& iov)
    {
      // If IOV is found, do not add object again
      if (std::find(m_iovs.begin(), m_iovs.end(), iov) != m_iovs.end())
        return;

      // If our object was already streamed out, i.e. when
      // output file is reloaded, just create new empty object.
      // It should be anyway checked for number of entries
      // before usage. Continuing in filling should result in
      // ROOT errors on histogram/tree merging and tree filling.
      // Other Mergeables should probably work well as soon as they
      // need no constructor parameters.
      if (!m_object) {
        m_object = new T();
      }

      T* newobj = new T();
      cloneObj(m_object, newobj);

      // Make unique name for named objects
      if (auto tnamed = dynamic_cast<TNamed*>(newobj)) {
        tnamed->SetName(newobj->GetName() + TString("-") + TString(iov));
      }

      // Get ROOT class information
      //TClass *cl = newobj->IsA();
      // Disconnect object from any files for ROOT objects
      //if (cl && cl->GetMethodWithPrototype("SetDirectory", "TDirectory*"))
      //  newobj->Execute("SetDirectory", "0");
      // Reset contents of a ROOT object
      //if (cl && cl->GetMethodWithPrototype("Reset", "Option_t*"))
      //  newobj->Execute("Reset", "");
      // Reset contents of Belle2::Mergeable object
      //else if (cl && cl->GetMethodWithPrototype("clear", ""))
      //  newobj->Execute("clear", "");

      newobj->SetDirectory(nullptr);
      newobj->Reset();

      // Add object to list
      m_objects.push_back(newobj);
      m_iovs.push_back(iov);
    }

    /**
     * @brief Get reference to object for given IOV
     * @param iov IOV of object
     * @return T& Corresponding object. A new object+iov is created
     *  and returned if the IOV is not found.
     */
    T& getObject(const KeyType& iov)
    {
      for (unsigned int i = 0; i < m_iovs.size(); i++) {
        if (iov == m_iovs[i]) {
          return *m_objects[i];
        }
      }
      // Object for new IOV requested.
      // Just create it, add to our list and return it
      constructObject(iov);
      return *m_objects[m_objects.size() - 1];
    }

    /**
     * @brief Get all managed objects with their IOVs
     * @return Vector of pairs of object pointers and IOV
     */
    std::vector<std::pair<T*, KeyType>> getObjects()
    {
      std::vector<std::pair<T*, KeyType>> result;
      for (unsigned int i = 0; i < m_iovs.size(); i++) {
        result.push_back(make_pair(m_objects[i], m_iovs[i]));
      }
      return result;
    }

    /// Check whether object of given IOV exists
    bool objectExists(const KeyType& iov)
    {
      for (unsigned int i = 0; i < m_iovs.size(); i++) {
        if (iov == m_iovs[i]) {
          return true;
        }
      }
      return false;
    }

    /// Replace internal object by a new one (possibly dangerous!)
    void replaceObject(T* newobj)
    {
      if (m_object)
        delete m_object;
      newobj->SetDirectory(nullptr);
      m_object = newobj;
    }

  private:
    /**
     * @brief Clone "normal" TObject
     * Makes a deep copy of source object
     * to dest object
     *
     * NOTE: For T=TTree this function is
     * never instantiated (as it is non-virtual
     * and not used in that case)
     *
     * @return void
     */
    void cloneObj(TObject* source, TObject* dest)
    {
      // Construct the object by making a copy
      // of the source object
      source->Copy(*dest);
      //*((T*) dest) = *((T*) source);
    }

    /**
     * @brief Clone TTree (Needs special treatment)
     * Makes a deep copy of source TTree
     * to dest TTree
     *
     * NOTE: For T not a TTree, this function is
     * never instantiated (as it is non-virtual
     * and not used in that case)
     *
     * @return void
     */
    void cloneObj(TTree* source, TTree*& dest)
    {
      if (dest) {
        // Empty tree is in dest by default.
        delete dest;
        dest = nullptr;
      }
      // Construct the TTree by making a copy
      // of the source TTree
      dest = source->CloneTree();
    }

    /** List of created managed objects  */
    std::vector<T*> m_objects{};
    /** List of IOVs for managed objects */
    std::vector<KeyType> m_iovs{};
    /** Template object used for later object creation */
    T* m_object{nullptr}; // Internal original object for making copies

    ClassDefOverride(CalibRootObj, 1) /**< Run dependent mergeable wrapper */
  };
}
