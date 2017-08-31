#pragma once

#include <string>
#include <TNamed.h>
#include <TDirectory.h>
#include <TList.h>
#include <TCollection.h>
#include <calibration/Utilities.h>

namespace Belle2 {

  /**
   * @brief Class to implement run dependence
   * into mergeable ROOT-compatible objects.
   */
  class CalibRootObjBase : public TNamed {

  public:

    /// Constructor
    CalibRootObjBase() : TNamed() {};

    /// Destructor
    virtual ~CalibRootObjBase() {};

    /** Allow merging using TFileMerger if saved directly to a file.
    *
    * \note dictionaries containing your Mergeable class need to be loaded, so 'hadd' will not work currently.
    */
    virtual Long64_t Merge(TCollection* hlist) = 0;

    //virtual void merge(const CalibRootObjBase* other) {}

    virtual TNamed* Clone(const char* newname)
    {
      TNamed* obj = constructObject(newname);
      return obj;
    }

    virtual void write(TDirectory* dir) = 0;
    virtual void setObjectName(std::string name) = 0;

  protected:
    virtual TNamed* constructObject(std::string name) = 0;

    ClassDef(CalibRootObjBase, 1) /// Run dependent mergeable wrapper
  };
}
