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

#include <klm/dataobjects/KLMHit2d.h>
#include <klm/dataobjects/KLMMuidLikelihood.h>
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/eklm/EKLMElementNumbers.h>

#include <mdst/dataobjects/Track.h>

#include <reconstruction/dataobjects/KLMMuonIDDNNInputVariable.h>

#include <mva/dataobjects/DatabaseRepresentationOfWeightfile.h>
#include <mva/interface/Expert.h>
#include <mva/interface/Weightfile.h>

#include <string>

namespace Belle2 {
  /**
   * KLMMuonIDDNNExpert for calculating Neural Network based muonID.
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

    /** Required array for Tracks. */
    StoreArray<Track> m_tracks;

    /** Input variables of DNN. */
    StoreArray<KLMMuonIDDNNInputVariable> m_inputVariable;

    /**
     * Initialize mva expert, dataset and features.
     * Called every time the weightfile in the database changes in begin run.
    */
    void init_mva(MVA::Weightfile& weightfile);

    /**
     * Get the NN-based muon probability.
     * @param[in] track  target track.
     * @param[in] klmll  KLMMuidLikelihood object related from the target track.
     * @return Neural Networkd based muonID.
     */
    float getNNmuProbability(const Track* track, const KLMMuidLikelihood* klmll);

    /** Total BKLM layers. */
    static constexpr int m_maxBKLMLayers = BKLMElementNumbers::getMaximalLayerNumber();

    /** Total EKLM layers. */
    static constexpr int m_maxEKLMLayers = EKLMElementNumbers::getMaximalLayerNumber();

    /** Database identifier or file used to load the weights. */
    std::string m_identifier = "KLMMuonIDDNNWeightFile";

    /** Database pointer to the database representation of the weightfile. */
    std::unique_ptr<DBObjPtr<DatabaseRepresentationOfWeightfile>> m_weightfile_representation;

    /** Pointer to the current MVA expert. */
    std::unique_ptr<MVA::Expert> m_expert;

    /** Pointer to the current dataset. */
    std::unique_ptr<MVA::SingleDataset> m_dataset;

    /** Container of hit widths of one track. */
    std::array < float, m_maxBKLMLayers + m_maxEKLMLayers > m_hitpattern_width;

    /** Container of hit steplength of one track. */
    std::array < float, m_maxBKLMLayers + m_maxEKLMLayers > m_hitpattern_steplength;

    /** Container of hit chi2 of one track. */
    std::array < float, m_maxBKLMLayers + m_maxEKLMLayers > m_hitpattern_chi2;

    /** Container of extrapolation situation at each KLM layer of one track. */
    std::array < bool, m_maxBKLMLayers + m_maxEKLMLayers > m_hitpattern_hasext;

    /** BKLM phi-measuring strip width (cm) by layer. */
    std::array < float, m_maxBKLMLayers > m_BarrelPhiStripWidth;

    /** BKLM Z-measuring strip width (cm) by layer. */
    std::array < float, m_maxBKLMLayers > m_BarrelZStripWidth;

    /** EKLM scintillator strip width (cm). */
    float m_EndcapScintWidth;

    /**
     * Get Hit width (cluster size) of a KLMHit2d.
     * @param[in] klmhit target KLMHit2d object.
     * @return hit width of the KLMHit2d object.
     */
    float getHitWidth(const KLMHit2d* klmhit);

  };
}
