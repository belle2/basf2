/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TREEFITTERMODULE_H
#define TREEFITTERMODULE_H

#include <framework/core/Module.h>
//#include <string>

namespace Belle2 {

  class Particle;

  class TreeFitterModule : public Module {
  public:
    TreeFitterModule();
    virtual ~TreeFitterModule();

    virtual void initialize() override;
    virtual void beginRun() override;
    virtual void event() override;
    virtual void endRun() override;
    virtual void terminate() override;

  private:
    //define your own data members here
    std::string m_particleList;     //name of ParticleList
    double m_confidenceLevel;       //minimum confidence level to accept fit
    double m_precision;             //max level of chi2 fluctuation required before the fit is considered stable and converging
    int m_verbose;                  //BaBar verbosity (to be phased out in favor of Belle2's logger)
    // now loaded for individual particles (FT)
    std::vector<int> m_massConstraintList; //PDG codes of particles to mass constrain. Later, this will be a decayDescriptor.
    //    double m_Bfield;                //Bfield from database
    //
    bool doTreeFit(Particle* head);
    bool printDaughters(Particle* mother);
    //

  };
}
#endif
