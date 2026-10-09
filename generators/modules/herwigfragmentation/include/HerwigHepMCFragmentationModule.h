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
#include <mdst/dataobjects/MCParticleGraph.h>
#include <generators/evtgen/EvtGenInterface.h>

#include <string>
#include <map>
#include <memory>

namespace HepMC { class GenEvent; }

namespace Belle2 {

  class HerwigFragHelper;

  /**
   * Stage 3 of the Herwig multistep pipeline.
   *
   * Reads pre-generated HepMC events (produced by a single batch Herwig invocation
   * in Stage 2) and KKMC sidecars (written by HerwigLHEWriterModule in Stage 1),
   * reconstructs the full MCParticle truth tree, and runs EvtGen + PHOTOS.
   *
   * No subprocess calls in event(), it is parallelizable;
   * workers read independent (offset-indexed) regions of the shared all_events.hepmc
   * file; the file is read-only after Stage 2 completes.
   */
  class HerwigHepMCFragmentationModule : public Module {

  public:
    HerwigHepMCFragmentationModule();
    virtual ~HerwigHepMCFragmentationModule();

    /** Open and validate HepMC file; load all events and manifest; initialize EvtGen. */
    virtual void initialize() override;
    /** Log success rate; warn if below 97% threshold. */
    virtual void terminate() override;
    /** Load one HepMC event, reconstruct MCParticle truth tree, apply EvtGen decays. */
    virtual void event() override;

  private:
    // Steerable module parameters
    std::string m_workDir{};          /**< Working directory written by HerwigLHEWriterModule (Stage 1). */
    std::string m_decFile{};          /**< EvtGen main decay file. */
    std::string m_userDecFile{};      /**< EvtGen user decay file. */
    bool m_useEvtGen{true};           /**< Use EvtGen for particle decays (default true). */
    bool m_coherentMixing{true};      /**< Coherent B0/B0bar mixing in EvtGen. */
    bool m_keepTempFiles{false};      /**< If false, delete WorkDir and all pipeline files in terminate(). */

    // Non-steerable internal state: read-only after initialize()
    std::string m_hepmcPath{};        /**< Resolved path to all_events.hepmc. */

    /** Mapping from global basf2 event number to HepMC event stored in memory. */
    std::map<int, HepMC::GenEvent*> m_hepmcEvents;

    /** Mapping from global event number to sidecar filename (basename only). */
    std::map<int, std::string> m_sidecarMap;

    std::unique_ptr<HerwigFragHelper> m_helper;    /**< Two-pass HepMC-to-graph helper. */
    MCParticleGraph m_mcParticleGraph;              /**< Particle graph rebuilt per event. */
    EvtGenInterface m_evtGenInterface;              /**< Persistent EvtGen interface; setup() called once in initialize(). */

    int m_eventCounter{0};            /**< Counts event() calls. */
    int m_successCounter{0};          /**< Counts successfully processed events. */

    /** Load the manifest.txt written by HerwigLHEWriterModule::terminate(). */
    bool loadManifest();

    /** Load all HepMC events from all_events.hepmc into m_hepmcEvents map.
     *  Key = global event number. Primary strategy: signal_process_id if all values
     *  are positive and unique. Fallback (always active with Herwig 7.2.0, which sets
     *  signal_process_id=-1): sequential manifest-order to HepMC-order mapping.
     *  Note: IDPRUP in the LHE file is hardcoded to 1 (process ID), not the event number.
     */
    bool loadHepMCEvents();

    /** Add KKMC particles from sidecar file to m_mcParticleGraph.
     * @param[in]  globalEvtNum  basf2 global event number (selects the sidecar file)
     * @param[out] quarkIndices  graph indices of the q/qbar particles, used by HerwigFragHelper for parent linking
     * @return true on success
     */
    bool loadKKMCSidecar(int globalEvtNum, std::vector<int>& quarkIndices);

    /** Apply EvtGen filter and write decayed particles to StoreArray. */
    bool applyEvtGenDecays();
  };

} // namespace Belle2
