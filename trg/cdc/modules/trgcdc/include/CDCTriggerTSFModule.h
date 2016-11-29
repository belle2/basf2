#ifndef CDCTRIGGERTSFModule_H
#define CDCTRIGGERTSFModule_H

#include "framework/core/Module.h"
#include <string>

#include "CLHEP/Geometry/Point3D.h"

namespace Belle2 {

  typedef HepGeom::Point3D<double> Point3D;

  class TRGCDCLayer;
  class TRGCDCSegment;

  /** Module for the Track Segment Finder of the CDC trigger.
   *  The CDC wires are organized in track segments with fixed shape.
   *  Within each track segment a logic combines the separate wire hits. */
  class CDCTriggerTSFModule : public Module {

  public:

    /** Constructor, for setting module description and parameters. */
    CDCTriggerTSFModule();

    /** Destructor */
    virtual ~CDCTriggerTSFModule() {}

    /** Initialize the module and register DataStore arrays. */
    virtual void initialize();

    /** Run the TSF for an event. */
    virtual void event();

    /** Clean up pointers. */
    virtual void terminate();

    /** remove hit information from last event */
    void clear();

  protected:
    /** name of the input StoreArray */
    std::string m_CDCHitCollectionName;
    /** The filename of LUT for the inner-most track segments. */
    std::string m_innerTSLUTFilename;
    /** The filename of LUT for outer track segments. */
    std::string m_outerTSLUTFilename;

  private:
    /** structure to hold pointers to all wires in the CDC */
    std::vector<std::vector<TRGCDCLayer*>> superLayers;
    /** structure to hold pointers to all track segment shapes */
    std::vector<TRGCDCLayer*> tsLayers;
  };

} // namespace Belle2

#endif // CDCTriggerTSFModule_H
