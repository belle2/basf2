/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/MCParticle.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
namespace Belle2 {
  /**
   *  Module to geometrically match neutral hadrons (KL, neutrons) to ECL clusters
   */

  class NeutralHadronMatcherModule : public Module {
  public:
    NeutralHadronMatcherModule();
    /** Overridden initialize method */
    virtual void initialize() override final;
    /** Overridden event method */
    virtual void event() override final;
  private:
    /** input mcPDG value */
    int m_mcPDG;
    /** input particle lists name */
    std::vector<std::string> m_ParticleLists;
    /** input efficiency correction */
    double m_effcorr;
    /** 3d matching parameter */
    double m_distance;
    /** extra info variable for distance */
    std::string m_infoName;
    /** extra info variable for matched MC */
    std::string m_matchedId;
  };
}
