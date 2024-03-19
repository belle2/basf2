/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <klm/dataobjects/KLMMuidLikelihood.h>
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/eklm/EKLMElementNumbers.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/KLMNNLikelihood.h>
#include <mva/dataobjects/DatabaseRepresentationOfWeightfile.h>
#include <mva/interface/Expert.h>
#include <mva/interface/Weightfile.h>


#include <string>

namespace Belle2 {
  /**
   * Get information from KLMMuidLikelihood
   */
  class KLMMuonIDDNNExpertModule : public Module {

  public:

    /**
     * Constructor.
     */
    KLMMuonIDDNNExpertModule();

    /**
     * Destructor.
     */
    ~KLMMuonIDDNNExpertModule();

    /**
     * Initializer.
     */
    void initialize() override;

    /**
     * Called when entering a new run.
     */
    void beginRun() override;

    /**
     * This method is called for each event.
     */
    void event() override;

    /**
     * This method is called if the current run ends.
     */
    void endRun() override;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate() override;

  private:

    // Required array for Tracks
    StoreArray<Track> m_tracks;

    // Neural Network based KLM muon likelihood
    StoreArray<KLMNNLikelihood> m_klmNNLikelihoods;

    /**
     * Initialize mva expert, dataset and features
     * Called every time the weightfile in the database changes in begin run
    */
    void init_mva(MVA::Weightfile& weightfile);

    /**
     * Get the NN-based muon probability.
     * @param[in] track  target track.
     * @param[in] klmll  KLMMuidLikelihood object related from the target track.
     */
    float getNNmuProbability(const Track* track, const KLMMuidLikelihood* klmll);

    /** Total KLM layers  */
    static constexpr int m_TotalKLMLayers = BKLMElementNumbers::getMaximalLayerNumber() + EKLMElementNumbers::getMaximalLayerNumber();

    /** Database identifier or file used to load the weights. */
    std::string m_identifier = "NNmuidWeightFile.root";

    /** Database pointer to the database representation of the weightfile. */
    std::unique_ptr<DBObjPtr<DatabaseRepresentationOfWeightfile>> m_weightfile_representation;

    /** Pointer to the current MVA expert. */
    std::unique_ptr<MVA::Expert> m_expert;

    /** Pointer to the current dataset. */
    std::unique_ptr<MVA::SingleDataset> m_dataset;

    /** Name of charged particle candidates list. */
    std::string m_inputListName;

    /** Container of hit widths of one track. */
    float m_hitpattern_width[m_TotalKLMLayers];

    /** Container of hit steplength of one track. */
    float m_hitpattern_steplength[m_TotalKLMLayers];

    /** Container of hit chi2 of one track. */
    float m_hitpattern_chi2[m_TotalKLMLayers];

    /** Container of extrapolation situation at each KLM layer of one track. */
    int m_hitpattern_hasext[m_TotalKLMLayers];

  };
}
