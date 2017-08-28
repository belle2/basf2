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
  class CalibRootObjBase : public TNamed {

  public:
    /// The key type in the map - IOV or other representation
    typedef std::pair<int, int> KeyType;

    /// Constructor
    CalibRootObjBase() : TNamed() {};

    /// Destructor
    virtual ~CalibRootObjBase() {};

    /** Allow merging using TFileMerger if saved directly to a file.
    *
    * \note dictionaries containing your Mergeable class need to be loaded, so 'hadd' will not work currently.
    */
    virtual Long64_t Merge(TCollection* hlist) {return 1;};

    virtual void merge(const CalibRootObjBase* other) {};

    KeyType getIOV() {return m_iov;}

  private:
    /** IOVs for managed object */
    KeyType m_iov{ -1, -1};

    ClassDef(CalibRootObjBase, 1) /// Run dependent mergeable wrapper
  };
}
