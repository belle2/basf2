/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <trg/cdc/NeuroTrigger.h>
#include <trg/cdc/NeuroTriggerParameters.h>
#include <trg/cdc/NeuroTrainer.h>
#include <framework/core/Module.h>
#include <trg/cdc/dataobjects/CDCTriggerMLPData.h>

namespace Belle2 {
  /** Description
   *
   */
  class CDCTriggerNeuroIDHistModule : public Module {
  public:
    /** Constructor, for setting module description and parameters. */
    CDCTriggerNeuroIDHistModule();
    /** Destructor. */
    virtual ~CDCTriggerNeuroIDHistModule() {}

    /** Initialize the module.
     * Initialize the networks and register datastore objects.
     */
    virtual void initialize() override;

    /** Called once for each event.
     * Prepare input and target for each track and store it.
     */
    virtual void event() override;

    /** Do the training for all sectors. */
    virtual void terminate() override;
  private:
    /** List of input tracks. */
    StoreArray<CDCTriggerTrack> m_tracks;
    /** dataset for all idhist prepare data */
    std::vector<CDCTriggerMLPData> m_trainSets_prepare;
    /** Instance of the NeuroTrigger. */
    NeuroTrigger m_NeuroTrigger;
    /** Switch to rescale out of range target values or ignore them. */
    bool m_rescaleTarget;
    /** Number of samples to prepare input ranges. */
    int m_nPrepare;
    /** Cut on the hit counters to get relevant ID ranges. */
    double m_relevantCut;
    /** Switch to apply cut to single hit counter or to sum over counters. */
    bool m_cutSum;
    /** Parameters for the NeuroTrigger. */
    NeuroTrigger::Parameters m_parameters;
    /** Parameters for the NeuroTrigger. */
    NeuroTriggerParameters m_neuroParameters;
    /** base name for idhist file and config file */
    std::string m_idHistName;
    /** name for the input configuration file which holds all the parameters and
     * the idhist tables for each expert */
    std::string m_configFileName;
    /** name for the output configuration file which holds all the parameters and
     * the idhist tables for each expert */
    std::string m_writeconfigFileName;
    /** Name of the MCParticles/RecoTracks collection used as target values. */
    std::string m_targetCollectionName;
    /** Switch between MCParticles or RecoTracks as targets. */
    bool m_trainOnRecoTracks;
    std::string m_hitCollectionName;
    /** name of the event time StoreObjPtr */
    std::string m_inputCollectionName;
    /** Switch between MCParticles or RecoTracks as targets. */

  };
}

