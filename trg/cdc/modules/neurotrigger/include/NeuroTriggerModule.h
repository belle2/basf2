#ifndef NEUROTRIGGERMODULE_H
#define NEUROTRIGGERMODULE_H

#include <framework/core/Module.h>
#include <trg/cdc/NeuroTrigger.h>

namespace Belle2 {

  class CDCTriggerMLP;

  /** The NeuroTrigger module of the CDC trigger.
   * Select one of several trained neural networks
   * and run it to estimate the z-vertex of a track
   * from track segment hits and 2D track estimates.
   */
  class NeuroTriggerModule : public Module {
  public:
    /** Constructor, for setting module description and parameters. */
    NeuroTriggerModule();

    /** Destructor. */
    virtual ~NeuroTriggerModule() {}

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
    /** Instance of the NeuroTrigger. */
    NeuroTrigger m_NeuroTrigger;
    /** Switch to execute the network with fixed point calculation.
     * (for FPGA simulation) */
    bool m_fixedPoint;
  };
}
#endif