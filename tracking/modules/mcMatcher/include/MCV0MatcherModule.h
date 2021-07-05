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
    ~MCV0MatcherModule();

    /** Use this to initialize resources or memory your module needs.
     *
     *  Registers the various StoreArrays and their Relations.  Also does the usual
     *  geometry setup needed for anything that uses extrapolations.
     */
    void initialize() override;

    /** Called once before a new run begins.
     *
     * (empty)
     */
    void beginRun() override;

    /** Called once for each event.
     *
     * Input: Belle2::Track, Belle2::TrackFitResult, Belle2:V0
     * Output: Relation between Belle2:V0 and Belle2::MCParticle
     */
    void event() override;

    /** Called once when a run ends.
     *
     *  (empty)
     */
    void endRun() override;

    /** Clean up anything you created in initialize().
     *
     *  (empty)
     */
    void terminate() override;
  private:
    std::string m_TrackColName;  ///< Belle2::Track collection name (input).
    std::string m_TFRColName;   ///< Belle2::TrackFitResult collection name (input).
    std::string m_V0ColName;   ///< V0 collection name (input).
    std::string m_MCParticleColName;   ///< MCParticle collection name (input).

  };
}
