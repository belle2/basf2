#pragma once

#include <framework/core/Module.h>

namespace Belle2 {
  /** A module matching the V0s from the mcV0Matcher to MC particles. .
   */
  class MCV0MatcherModule : public Module {
  public:
    /** Constructor, for setting module description and parameters. */
    MCV0MatcherModule();

    /** Destructor (empty). */
    virtual ~MCV0MatcherModule();

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
     * Input: Belle2::Track, Belle2::TrackFitResult, Belle2:V0
     * Output: Relation between Belle2:V0 and Belle2::MCParticle
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
    std::string m_TrackColName;  ///< Belle2::Track collection name (input).
    std::string m_TFRColName;   ///< Belle2::TrackFitResult collection name (input).
    std::string m_V0ColName;   ///< V0 collection name (input).
    std::string m_MCParticleColName;   ///< MCParticle collection name (input).

  };
}
