/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <mdst/dataobjects/MCParticle.h>

#include <fstream>
#include <string>
#include <map>

namespace Belle2 {

  /**
   * Stage 1 of the Herwig multistep pipeline.
   *
   * Reads KKMC MCParticles (e+, e-, virtual photon/Z0, q, qbar) from the StoreArray,
   * accumulates them into a single LHE file and writes per-event
   * KKMC sidecar files so that Stage 3 can link the full truth tree.
   *
   * The shared LHE file must be written in event order, which requires a single input process.
   */
  class HerwigLHEWriterModule : public Module {

  public:
    HerwigLHEWriterModule();
    virtual ~HerwigLHEWriterModule();

    /** Register required MCParticle StoreArray. */
    virtual void initialize() override;
    /** Close LHE file and write manifest.txt. */
    virtual void terminate() override;
    /** Accumulate one event into an LHE file and write its KKMC sidecar. */
    virtual void event() override;

  private:
    /** Per-event record stored for manifest.txt */
    struct EventRecord {
      std::string sidecarFile; /**< Basename of the KKMC sidecar file. */
    };

    // Steerable module parameters
    std::string m_workDir{};          /**< User-specified working dir; auto-created via mkdtemp if empty. */
    double m_showerScale{11.0};       /**< Maximum shower scale ceiling in GeV (default 11.0). */

    // Non-steerable internal state
    std::string m_workingDir{};       /**< Resolved working directory path. */
    std::ofstream m_lheStream;        /**< Output stream to all_events.lhe (open from initialize to terminate). */
    std::map<int, EventRecord> m_manifest; /**< global_event_number -> EventRecord */
    unsigned int m_batchSeed{0};      /**< Batch seed drawn from gRandom in initialize(). */
    int m_eventCounter{0};            /**< Counts event() calls. */

    /** Write one <event>...</event> block to m_lheStream. */
    bool writeLHEEventBlock(const std::vector<MCParticle*>& quarks);

    /** Write per-event KKMC sidecar file: all MCParticles with momenta + parent links. */
    bool writeKKMCSidecar(int globalEvtNum);
  };

} // namespace Belle2
