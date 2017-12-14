#ifndef CDCTRIGGER2DFITTERModule_H
#define CDCTRIGGER2DFITTERModule_H

#include "framework/core/Module.h"
#include <string>
#include <vector>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <framework/dataobjects/EventT0.h>

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
    /** Name of the StoreArray containing the input track segment hits. */
    std::string m_hitCollectionName;
    /** name of the event time StoreObjPtr */
    std::string m_EventTimeName;
    /** Name of the StoreArray containing the input tracks from the finder. */
    std::string m_inputCollectionName;
    /** Name of the StoreArray containing the resulting fitted tracks. */
    std::string m_outputCollectionName;
    /** Minimal number of hits required for fitting. */
    unsigned m_minHits;
    /** Switch between nominal drift velocity and xt table */
    bool m_xtSimple;
    /** Switch between drift time and wire position for phi */
    bool m_useDriftTime;

  private:
    /** geometry constants: number of wires per super layer */
    std::vector<double> nWires = {};
    /** geometry constants: radius of priority layers */
    std::vector<double> rr = {};
    /** geometry constants: drift length - drift time relation */
    std::vector<std::vector<double>> xtTables = {};

    /** list of input tracks from finder */
    StoreArray<CDCTriggerTrack> m_finderTracks;
    /** list of output tracks from fitter */
    StoreArray<CDCTriggerTrack> m_fitterTracks;
    /** StoreObjPtr contraining the event time */
    StoreObjPtr<EventT0> m_eventTime;
  };

} // namespace Belle2

#endif // CDCTrigger2DFitterModule_H
