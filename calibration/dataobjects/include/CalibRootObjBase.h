#pragma once

#include <string>
#include <TNamed.h>
#include <TDirectory.h>
#include <TList.h>
#include <TCollection.h>
#include <calibration/Utilities.h>
#include <calibration/core/MergeableNamed.h>

namespace Belle2 {

  /**
   * @brief Class to implement run dependence
   * into mergeable ROOT-compatible objects.
   */
  class CalibRootObjBase : public MergeableNamed {

  public:

    /// Constructor
    CalibRootObjBase() : MergeableNamed() {};

    /// Destructor
    virtual ~CalibRootObjBase() {};

    virtual CalibRootObjBase* Clone(const char* newname = "") const override {return constructObject(newname);}

    virtual void write(TDirectory* dir) = 0;
    virtual void setObjectName(std::string name) = 0;

  protected:
    virtual CalibRootObjBase* constructObject(std::string name) const = 0;

    ClassDefOverride(CalibRootObjBase, 1) /// Run dependent mergeable wrapper
  };
}
