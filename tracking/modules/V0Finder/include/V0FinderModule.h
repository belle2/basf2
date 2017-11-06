#ifndef V0FINDERMODULE_H
#define V0FINDERMODULE_H

#include <framework/core/Module.h>

#include <memory>

namespace Belle2 {
  /** A V0 finder module.
   *
   *  Pairs up all positive and negative tracks,
   *  tries to find vertices between them,
   *  stores found vertices.
   *  Only vertices outside the beam pipe are saved, the others are
   *  recovered in a following analysis level
   *  for use inside the beam pipe, one outside (the difference being
   *
   *  Cut outside the beam pipe is maximum chi^2.
   *  The value used as beam pipe radius is an option.
   *
   *  FIXME once particle hypotheses are thought through, we should
   *  also deal with photon conversion.
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
    std::string m_RecoTrackColName; ///< InputColName of the RecoTracks.
    std::string m_TFRColName; ///< OutputColName of the TrackFitResults matching to the created V0s.
    std::string m_TrackColName; ///< InputColName of the Tracks.
    std::string m_V0ColName; ///< OutputColName of the V0

    double m_beamPipeRadius; ///< Radius where inside/outside beampipe is defined.
    double m_vertexChi2CutOutside; ///< Chi2 cut for V0s outside of the beampipe. Applies to all.

    bool m_validation; ///< Flag if use validation.
    std::string m_V0ValidationVertexColName; ///< OutputColName of the V0ValidationVertex
  };
}
#endif
