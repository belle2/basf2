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

#include <klm/bklm/geometry/GeometryPar.h>

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
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    KLMNNmuidModule();

    /**  */
    void initialize() override;

    /** n */
    void beginRun() override;

    /** n */
    void event() override;

    /** n */
    void endRun() override;

    /** n */
    void terminate() override;

    /** n */
    void getApplicationVariables(Particle* part);


  private:

    /**
     * Initialize mva expert, dataset and features
     * Called every time the weightfile in the database changes in begin run
    */
    void init_mva(MVA::Weightfile& weightfile);

    /** Database identifier or file used to load the weights. */
    std::string m_identifier = "NNKLMmuonID";

    /** Pointer to the current MVA expert. */
    std::unique_ptr<MVA::Expert> m_expert;

    /** Pointer to the current dataset. */
    std::unique_ptr<MVA::SingleDataset> m_dataset;

    std::string m_inputListName;

    double m_hitChiCut;

    DBObjPtr<KLMLikelihoodParameters> m_LikelihoodParameters;

    //StoreArray<KLMMuidLikelihood> m_klmMuidLikelihoods;

    StoreArray<KLMHit2d> m_klmHit2ds;

    bklm::GeometryPar* m_bklmGeoPar;

    float m_hitpattern_width[29];
    float m_hitpattern_distance[29];
    float m_hitpattern_steplength[29];
    float m_hitpattern_chi2[29];
    int m_hitpattern_nhits[29];
    int m_hitpattern_hasext[29];

  };
}
