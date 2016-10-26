#ifndef CDCTRIGGER3DFITTERModule_H
#define CDCTRIGGER3DFITTERModule_H

#include "framework/core/Module.h"
#include <string>

namespace Belle2 {

  /** Module for the 3D Fitter of the CDC trigger. */
  class CDCTrigger3DFitterModule : public Module {

  public:

    /** Constructor, for setting module description and parameters. */
    CDCTrigger3DFitterModule();

    /** Destructor */
    virtual ~CDCTrigger3DFitterModule() {}

    /** Initialize the module and register DataStore arrays. */
    virtual void initialize();

    /** Run the 3D fitter for an event. */
    virtual void event();

  protected:
    /** Name of the StoreArray containing the input tracks from the 2D fitter. */
    std::string m_inputCollectionName;
    /** Name of the StoreArray containing the resulting 3D tracks. */
    std::string m_outputCollectionName;

  private:
  };

} // namespace Belle2

#endif // CDCTrigger3DFitterModule_H
