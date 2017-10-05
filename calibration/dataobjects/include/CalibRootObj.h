#pragma once

#include <string>
#include <TList.h>
#include <TTree.h>
#include <TObject.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TH1D.h>
#include <TH2F.h>
#include <TH2D.h>
#include <alignment/dataobjects/MilleData.h>
#include <calibration/dataobjects/RunRange.h>

#include <TCollection.h>
#include <TDirectory.h>
#include <calibration/dataobjects/CalibRootObjBase.h>
#include <calibration/Utilities.h>
#include <framework/logging/Logger.h>

namespace Belle2 {

  template<class T>
  class CalibRootObj : public CalibRootObjBase {

  public:
    /// Constructor
    CalibRootObj() : CalibRootObjBase() {};

    /// Destructor
    virtual ~CalibRootObj()
    {
      if (m_object) {
        m_object->SetDirectory(nullptr);
        delete m_object;
        B2DEBUG(100, "Deleted the object stored inside " << this->GetName());
      }
    }

    /// Allow merging using TFileMerger if saved directly to a file.
    virtual Long64_t Merge(TCollection* hlist) override
    {
      B2DEBUG(100, "Running Merge() on " << this->GetName());
      Long64_t nMerged = 0;
      if (hlist) {
        const CalibRootObj<T>* xh = 0;
        TIter nxh(hlist);
        while ((xh = dynamic_cast<CalibRootObj<T>*>(nxh()))) {
          // Add xh to me
          merge(xh);
          ++nMerged;
        }
      }
      B2DEBUG(100, "Merged " << nMerged << " objects");
      return nMerged;
    }

    virtual void merge(const CalibRootObj<T>* other)
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
    explicit CalibRootObj(T* object) : CalibRootObjBase()
    {
      B2DEBUG(100, "Creating CalibRootObj");
      if (m_object) {
        delete m_object;
      }

      object->SetDirectory(nullptr);
      m_object = object;
    }

    T* getObject() {return m_object;}

    virtual void write(TDirectory* dir)
    {
      dir->WriteTObject(m_object, m_object->GetName(), "Overwrite");
    }

    virtual void setObjectName(std::string name)
    {
      m_object->SetName(name.c_str());
    }

  private:
    /** Managed object  */
    T* m_object{nullptr};

    /**
     * @brief Construct an object for new iov.
     * The stored template object is copied and
     * new one is created for the iov.
     */
    virtual TNamed* constructObject(std::string name) override
    {
      B2DEBUG(100, "Creating new CalibRootObj with name " << name);
      T* newObj = new T();
      cloneObj(m_object, newObj);
      newObj->SetDirectory(nullptr);
      newObj->Reset();
      CalibRootObj<T>* newCalibObj = new CalibRootObj<T>(newObj);
      newCalibObj->SetName(name.c_str());
      B2DEBUG(100, "Created CalibRootObj with name " << name);
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

    ClassDefOverride(CalibRootObj, 1) /// Run dependent mergeable wrapper
  };
}
