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
#include <klm/dataobjects/bklm/BKLMHit1d.h>

#include <klm/bklm/geometry/GeometryPar.h>

#include <klm/muidgnn/MuidBuilder_fixed.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>

#include <string>

namespace Belle2 {
  /**
   * Get information from KLMMuidLikelihood
   */
  class KLMHitStudyModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    KLMHitStudyModule();

    /**  */
    void initialize() override;

    /** n */
    void event() override;

    /** n */
    void getStripPosition();

    /** n */
    int GetMatchedHits(KLMHit2d* klmhit, int hit_inBKLM, const MCParticle* mcpart);

    /** n */
    void CheckMCParticle(RelationVector<KLMDigit>& digitrelations, const MCParticle* mcpart, int& matchedHits);

    /** n */
    void CheckMCParticle(KLMDigit* digit, const MCParticle* mcpart, int& matchedHits);

    /** n */
    int IterateMCMother(MCParticle* mcpart1, const MCParticle* mcpart);

    /** n */
    int HitOnTrack(KLMHit2d* klmhit, const Track* track);

    /** n */
    void getApplicationVariables(Particle* part);


  private:

    std::string  m_inputListName;

    double m_hitChiCut;

    DBObjPtr<KLMLikelihoodParameters> m_LikelihoodParameters;

    //StoreArray<KLMMuidLikelihood> m_klmMuidLikelihoods;

    StoreArray<KLMHit2d> m_klmHit2ds;
    StoreArray<BKLMHit1d> m_bklmHit1ds;

    std::unique_ptr<MuidBuilder_fixed> mubuilder;
    std::unique_ptr<MuidBuilder_fixed> pibuilder;

    bklm::GeometryPar* m_bklmGeoPar;

    int get_binary(int number, int position)
    {
      if (number & (1 << position)) {
        return 1;
      } else {
        return 0;
      }
    }

    std::vector<std::vector<float>> m_attributelist_hit;
    std::vector<std::vector<float>> m_attributelist_ext;
    std::vector<int> m_layerinfo_hit;
    std::vector<int> m_layerinfo_ext;
    float m_hitpattern_distance[29];
    float m_hitpattern_steplength[29];
    float m_hitpattern_width[29];
    float m_hitpattern_chi2[29];
    int m_hitpattern_nhits[29];
    int m_hitpattern_hasext[29];


  };
}
