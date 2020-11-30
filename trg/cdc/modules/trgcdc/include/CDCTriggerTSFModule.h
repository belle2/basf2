#ifndef CDCTRIGGERTSFModule_H
#define CDCTRIGGERTSFModule_H

#include "framework/core/Module.h"
#include <string>

#include "CLHEP/Geometry/Point3D.h"
#include "trg/trg/Clock.h"

#include <framework/datastore/StoreArray.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <cdc/dataobjects/CDCHit.h>
#include <framework/database/DBObjPtr.h>
#include <trg/cdc/dbobjects/CDCTriggerDeadch.h>

namespace Belle2 {
  typedef HepGeom::Point3D<double> Point3D;

  class TRGCDCLayer;

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
    virtual void initialize() override;

    /** Run the TSF for an event. */
    virtual void event() override;

    /** Clean up pointers. */
    virtual void terminate() override;

    /** remove hit information from last event */
    void clear();

  protected:
    /** name of the input StoreArray */
    std::string m_CDCHitCollectionName;
    /** name of the output StoreArray */
    std::string m_TSHitCollectionName;
    /** The filename of LUT for the inner-most track segments. */
    std::string m_innerTSLUTFilename;
    /** The filename of LUT for outer track segments. */
    std::string m_outerTSLUTFilename;
    /** switch for simulating clock by clock */
    bool m_clockSimulation;
    /** switch for saving the number of true left/right for each pattern */
    bool m_makeTrueLRTable;
    /** filename for the table which contains the number of true left/right
     *  for each pattern in the inner-most super layer */
    std::string m_innerTrueLRTableFilename;
    /** filename for the table which contains the number of true left/right
     *  for each pattern in the outer super layers */
    std::string m_outerTrueLRTableFilename;
    /** mask Dead channel or not. True:mask False:unmask */
    bool m_deadchflag;
    /** dbobject to store deadchannel */
    DBObjPtr<CDCTriggerDeadch> m_db_deadchannel;

  private:
    /** structure to hold pointers to all wires in the CDC */
    std::vector<std::vector<TRGCDCLayer*>> superLayers;
    /** structure to hold pointers to all track segment shapes */
    std::vector<TRGCDCLayer*> tsLayers;
    /** list of clocks used in the TSF */
    std::vector<TRGClock*> clocks;
    /** list of (# true right, # true left, # true background)
     *  for the inner-most super layer */
    std::vector<std::vector<unsigned>> innerTrueLRTable = {};
    /** list of (# true right, # true left, # true background)
     *  for the outer super layers */
    std::vector<std::vector<unsigned>> outerTrueLRTable = {};
    //** number of layers in Super layer**/
    const static int MAX_N_LAYERS = 8;
    /** bad channel mapping */
    bool deadch_map[nSuperLayers][MAX_N_LAYERS][MAX_N_SCELLS] = {};

    /** list of input CDC hits */
    StoreArray<CDCHit> m_cdcHits;
    /** list of output track segment hits */
    StoreArray<CDCTriggerSegmentHit> m_segmentHits;

  };

} // namespace Belle2

#endif // CDCTriggerTSFModule_H
