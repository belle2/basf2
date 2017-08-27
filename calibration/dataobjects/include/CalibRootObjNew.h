#pragma once

#include <string>
#include <TNamed.h>
#include <TList.h>
#include <TCollection.h>

namespace Belle2 {

  /**
   * @brief Class to implement run dependence
   * into mergeable ROOT-compatible objects.
   */
  template<class T>
  class CalibRootObjNew : public TNamed {

  public:
    /// The key type in the map - IOV or other representation
    typedef std::pair<int, int> KeyType;

    /// Constructor
    CalibRootObjNew() : TNamed() {};

    /// Destructor
    virtual ~CalibRootObjNew()
    {
      if (m_object) {
        m_object->SetDirectory(nullptr);
        delete m_object;
      }
    }

    /** Allow merging using TFileMerger if saved directly to a file.
    *
    * \note dictionaries containing your Mergeable class need to be loaded, so 'hadd' will not work currently.
    */
    virtual Long64_t Merge(TCollection* hlist)
    {
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
      return nMerged;
    }

    virtual void merge(const CalibRootObjNew<T>* other)
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
    explicit CalibRootObjNew(T* object) : TNamed()
    {
      if (m_object) {
        delete m_object;
      }

      object->SetDirectory(nullptr);
      m_object = object;
    }

    KeyType getIOV() {return m_iov;}

    T* getObject() {return m_object;}

  private:
    /** Managed object  */
    T* m_object{nullptr};
    /** IOVs for managed object */
    KeyType m_iov{ -1, -1};

    ClassDef(CalibRootObjNew, 1) /// Run dependent mergeable wrapper
  };
}
