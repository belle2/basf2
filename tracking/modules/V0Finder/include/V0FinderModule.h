#ifndef V0FINDERMODULE_H
#define V0FINDERMODULE_H

// unique_ptr lives in <memory> but inclusion leads to lots of warnings
//#include <memory>

#include <framework/core/Module.h>
#include <framework/core/Module.h>

#include "TH1.h"
#include "TH2.h"
#include "TTree.h"

#include "genfit/GFRaveVertexFactory.h"

namespace Belle2 {
  /** A V0 finder module.
   *
   *  Pairs up all positive and negative tracks, tries ot find vertices between them,
   *  stores found vertices fulfilling a chi^2 cut and a minimum separation from the IP
   *  (both configurable) as Belle2:V0.
   */
  class V0FinderModule : public Module {
  public:
    /** Constructor, for setting module description and parameters. */
    V0FinderModule();

    /** Destructor (empty). */
    virtual ~V0FinderModule();

    /** Use this to initialize resources or memory your module needs.
     *
     *  Registers the various StoreArrays and their Relations.  Also does the usual
     *  geometry setup needed for anything that uses extrapolations.
     */
    virtual void initialize();

    /** Called once before a new run begins.
     *
     * (empty)
     */
    virtual void beginRun();

    /** Called once for each event.
     *
     * Input: Belle2::Track, output Belle2:V0
     */
    virtual void event();

    /** Called once when a run ends.
     *
     *  (empty)
     */
    virtual void endRun();

    /** Clean up anything you created in initialize().
     *
     *  (empty)
     */
    virtual void terminate();
  private:
    std::string m_GFTrackColName;
    std::string m_TFRColName;
    std::string m_TrackColName;
    std::string m_V0ColName;

    double m_minD0;               // Minimum perigee distance.
    double m_vertexChi2Cut;       // Maximum chi2 of vertex fit.
    double m_distRfromIP;         // minimum required distance from IP in R
    double m_distZfromIP;         // minimum required distance from IP in Z

    // FIXME Rave leaks memory on initialization, so we only initialize this once.
    // Unfortunately, there's a much bigger leak inside Rave::PropagatorWrapper.
    std::unique_ptr<genfit::GFRaveVertexFactory> m_vertexFactory;
  };
}
#endif
