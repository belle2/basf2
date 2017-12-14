#ifndef CDCTRIGGERNEUROMODULE_H
#define CDCTRIGGERNEUROMODULE_H

#include <framework/core/Module.h>
#include <trg/cdc/NeuroTrigger.h>

#include <framework/datastore/StoreArray.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>

namespace Belle2 {

  class CDCTriggerMLP;

  /** The neural network module of the CDC trigger.
   * Select one of several trained neural networks
   * and run it to estimate the z-vertex of a track
   * from track segment hits and 2D track estimates.
   */
  class CDCTriggerNeuroModule : public Module {
  public:
    /** Constructor, for setting module description and parameters. */
    CDCTriggerNeuroModule();

    /** Destructor. */
    virtual ~CDCTriggerNeuroModule() {}

    /** Initialize the module.
     * Load the network weights and register datastore objects.
     */
    virtual void initialize();

    /** Called once for each event.
     * Run the network for all CDCTriggerTracks and store output z-vertex
     * in the same CDCTriggerTrack.
     */
    virtual void event();

  protected:
    /** Name of file where network weights etc. are stored. */
    std::string m_filename;
    /** Name of the TObjArray holding the networks. */
    std::string m_arrayname;
    /** Name of the StoreArray containing the input track segment hits. */
    std::string m_hitCollectionName;
    /** name of the event time StoreObjPtr */
    std::string m_EventTimeName;
    /** Name of the StoreArray containing the input 2D tracks. */
    std::string m_inputCollectionName;
    /** Name of the StoreArray containing the resulting NN tracks. */
    std::string m_outputCollectionName;
    /** Instance of the NeuroTrigger. */
    NeuroTrigger m_NeuroTrigger;
    /** Switch to execute the network with fixed point calculation.
     * (for FPGA simulation) */
    bool m_fixedPoint;
    /** Fixed point precision in bit after radix point.
     *  8 values:
     *  - 2D track parameters: omega, phi
     *  - geometrical values derived from track: crossing angle, reference wire ID
     *  - scale factor: radian to wire ID
     *  - MLP values: nodes, weights, activation function LUT input (LUT output = nodes)
     */
    std::vector<unsigned> m_precision;

    /** list of input 2D tracks */
    StoreArray<CDCTriggerTrack> m_tracks2D;
    /** list of output NN tracks */
    StoreArray<CDCTriggerTrack> m_tracksNN;
    /** list of track segment hits */
    StoreArray<CDCTriggerSegmentHit> m_segmentHits;
  };
}
#endif
