#ifndef V0FINDERMODULE_H
#define V0FINDERMODULE_H

#include <framework/core/Module.h>
#include <framework/gearbox/Const.h>
#include <memory>

#include "genfit/GFRaveVertexFactory.h"

namespace Belle2 {
  /** A V0 finder module.
   *
   *  Pairs up all positive and negative tracks, tries to find vertices between them,
   *  stores found vertices.  There are two sets of cuts, one intended
   *  for use inside the beam pipe, one outside (the difference being
   *  that B decay tracks can all be matched up to form a vertex, so
   *  the number of V0s would be n_positiveTracks * n_negativeTracks,
   *  and this module would be useless).
   *
   *  Cuts outside the beam pipe are maximum chi^2.
   *  Cuts inside the beam pipe are a mass window around the Kshort
   *  mass and a maximum chi^2.
   *  The value used a beam pipe radius is also an option.
   *  FIXME once particle hypotheses are thought through, we should
   *  also deal with Lambda, photon conversion.
   *
   *  The resulting pairs of tracks are stored as mdst::V0.
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
    std::string m_GFTrackColName;  //!< Genfit track column name in the DataStore
    std::string m_TFRColName;  //!< TrackFitResult column name in the DataStore
    std::string m_TrackColName;  //!< Track column name in the DataStore
    std::string m_V0ColName;  //!< V0 column name in the DataStore

    double m_beamPipeRadius;
    double m_vertexChi2CutInside;
    double m_massWindowKshortInside;
    double m_vertexChi2CutOutside;
    int m_v0Pdg;

    Const::ChargedStable m_trackHypothesis = Const::pion;
    Const::ParticleType m_v0Hypothesis = Const::Kshort;
  };
}
#endif
