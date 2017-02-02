#ifndef CDCTRIGGER2DFITTERModule_H
#define CDCTRIGGER2DFITTERModule_H

#include "framework/core/Module.h"
#include <string>

namespace Belle2 {

  /** Module for the 2D Fitter of the CDC trigger. */
  class CDCTrigger2DFitterModule : public Module {

  public:

    /** Constructor, for setting module description and parameters. */
    CDCTrigger2DFitterModule();

    /** Destructor */
    virtual ~CDCTrigger2DFitterModule() {}

    /** Initialize the module and register DataStore arrays. */
    virtual void initialize();

    /** Run the 2D fitter for an event. */
    virtual void event();

  protected:
    /** Name of the StoreArray containing the input tracks from the finder. */
    std::string m_inputCollectionName;
    /** Name of the StoreArray containing the resulting fitted tracks. */
    std::string m_outputCollectionName;
    /** Minimal number of hits required for fitting. */
    unsigned m_minHits;

  private:
  };

} // namespace Belle2

#endif // CDCTrigger2DFitterModule_H
