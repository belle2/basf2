#pragma once

#include <string>
#include <TList.h>
#include <TTree.h>
#include <TObject.h>
#include <TCollection.h>
#include <calibration/dataobjects/CalibRootObjBase.h>
#include <calibration/Utilities.h>

namespace Belle2 {

  /**
   * @brief Class to implement run dependence
   * into mergeable ROOT-compatible objects.
   */
  template<class T>
  class CalibRootObjNew : public CalibRootObjBase {

  public:
    /// Constructor
    CalibRootObjNew() : CalibRootObjBase() {};

    /// Destructor
    virtual ~CalibRootObjNew()
    {
      if (m_object) {
        m_object->SetDirectory(nullptr);
        delete m_object;
        B2DEBUG(100, "Deleted the object stored inside " << this->GetName());
      }
    }

    /** Allow merging using TFileMerger if saved directly to a file.
    *
    * \note dictionaries containing your Mergeable class need to be loaded, so 'hadd' will not work currently.
    */
    virtual Long64_t Merge(TCollection* hlist) override
    {
      B2DEBUG(100, "Running Merge() on " << this->GetName());
      Long64_t nMerged = 0;
      if (hlist) {
        const CalibRootObjNew<T>* xh = 0;
        TIter nxh(hlist);
        while ((xh = dynamic_cast<CalibRootObjNew<T>*>(nxh()))) {
          // Add xh to me
          merge(xh);
          ++nMerged;
        }
      }
      B2DEBUG(100, "Merged " << nMerged << " objects");
      return nMerged;
    }

    virtual void merge(const CalibRootObjNew* other)
    {
      TList list;
      list.SetOwner(false);
      list.Add(other->m_object);
      m_object->Merge(&list);
    }

    /**
     * @brief User constructor
     * @param object Pointer to prepared ROOT object
     * (TH1F, TTree etc.) to be stored as template for
     * later automatic object creation.
     *
     * Takes ownership of object (do not delete it!)
     */
    explicit CalibRootObjNew(T* object) : CalibRootObjBase()
    {
      B2DEBUG(100, "Creating CalibRootObjNew");
      if (m_object) {
        delete m_object;
      }

      object->SetDirectory(nullptr);
      m_object = object;
    }

    T* getObject() {return m_object;}

  private:
    /** Managed object  */
    T* m_object{nullptr};

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
    virtual TNamed* constructObject(std::string name) override
    {
      B2DEBUG(100, "Creating new CalibRootObjNew with name " << name);
      T* newObj = new T();
      cloneObj(m_object, newObj);

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
      newObj->SetDirectory(nullptr);
      newObj->Reset();
      CalibRootObjNew<T>* newCalibObj = new CalibRootObjNew<T>(newObj);
      newCalibObj->SetName(name.c_str());
      B2DEBUG(100, "Created CalibRootObjNew with name " << name);
      return newCalibObj;
    }


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
      B2DEBUG(100, "Held object wil be treated as a generic TObject and have Copy() called.");
      // Construct the object by making a copy
      // of the source object
      source->Copy(*dest);
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
      B2DEBUG(100, "Held object is a TTree which will be have CloneTree() called.");
      if (dest) {
        // Empty tree is in dest by default.
        delete dest;
        dest = nullptr;
      }
      // Construct the TTree by making a copy
      // of the source TTree
      dest = source->CloneTree();
    }

    ClassDefOverride(CalibRootObjNew, 1) /// Run dependent mergeable wrapper
  };
}
