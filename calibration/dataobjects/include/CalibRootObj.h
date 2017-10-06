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

    virtual void merge(const MergeableNamed* other) override
    {
      auto castedOther = dynamic_cast<const CalibRootObj<T>*>(other);
      TList list;
      list.SetOwner(false);
      list.Add(castedOther->getObject());
      m_object->Merge(&list);
    }

    virtual void clear() override {m_object->Reset();}
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

    T* getObject() const {return m_object;}

    virtual void write(TDirectory* dir) override
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
    virtual CalibRootObj<T>* constructObject(std::string name) const override
    {
      B2DEBUG(100, "Creating new CalibRootObj with name " << name);
      T* newObj = (T*)cloneObj(m_object, name);
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
    TNamed* cloneObj(TNamed* source, std::string newName) const
    {
      //B2DEBUG(100, "Held object wil be treated as a generic TObject and have Copy() called.");
      // Construct the object by making a copy
      // of the source object
      //source->Copy(*dest);
      B2DEBUG(100, "Held object wil be treated as a generic TNamed and have Clone(newname) called.");
      TNamed* dest = (TNamed*)source->Clone(newName.c_str());
      return dest;
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
    TTree* cloneObj(TTree* source, std::string newName) const
    {
      B2DEBUG(100, "Held object is a TTree which will be have CloneTree() called.");
      // Construct the TTree by making a copy
      // of the source TTree
      TTree* dest = source->CloneTree(0);
      dest->SetName(newName.c_str());
      return dest;
    }

    ClassDefOverride(CalibRootObj, 1) /// Run dependent mergeable wrapper
  };
}
