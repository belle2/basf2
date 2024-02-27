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

#include <analysis/dataobjects/Particle.h>

#include <klm/dbobjects/KLMLikelihoodParameters.h>
#include <klm/muid/MuidElementNumbers.h>
#include <klm/dataobjects/KLMMuidLikelihood.h>
#include <klm/dataobjects/KLMHit2d.h>

#include <mdst/dataobjects/Track.h>
#include <mva/dataobjects/DatabaseRepresentationOfWeightfile.h>
#include <mva/interface/Expert.h>
#include <mva/interface/Weightfile.h>


#include <string>

namespace Belle2 {
  /**
   * Get information from KLMMuidLikelihood
   */
  class KLMNNmuidModule : public Module {

  public:

    /**
     * Constructor.
     */
    KLMNNmuidModule();

    /**
     * Destructor.
     */
    ~KLMNNmuidModule();

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

    /**
     * Initialize mva expert, dataset and features
     * Called every time the weightfile in the database changes in begin run
    */
    void init_mva(MVA::Weightfile& weightfile);

    /**
     * Get the NN-based muon probability.
     * @param[in] part  target charged particle.
     */
    float getNNmuProbability(const Particle* part);

    /** Database identifier or file used to load the weights. */
    std::string m_identifier = "NNKLMmuonProbability";

    /** Database pointer to the database representation of the weightfile. */
    std::unique_ptr<DBObjPtr<DatabaseRepresentationOfWeightfile>> m_weightfile_representation;

    /** Pointer to the current MVA expert. */
    std::unique_ptr<MVA::Expert> m_expert;

    /** Pointer to the current dataset. */
    std::unique_ptr<MVA::SingleDataset> m_dataset;

    /** Name of charged particle candidates list. */
    std::string m_inputListName;

    double m_hitChiCut;

    /** KLMHit2d lists. */
    StoreArray<KLMHit2d> m_klmHit2ds;

    /** Container of hit widths of one track. */
    float m_hitpattern_width[29];

    float m_hitpattern_distance[29];

    /** Container of hit steplength of one track. */
    float m_hitpattern_steplength[29];

    /** Container of hit chi2 of one track. */
    float m_hitpattern_chi2[29];

    /** Container of extrapolation situation at each KLM layer of one track. */
    int m_hitpattern_hasext[29];
    int m_hitpattern_nhits[29];

  };
}
