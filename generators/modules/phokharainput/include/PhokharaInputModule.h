
/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PHOKHARAINPUTMODULE_H
#define PHOKHARAINPUTMODULE_H

#include <framework/core/Module.h>
#include <framework/logging/Logger.h>

#include <generators/phokhara/Phokhara.h>
#include <generators/utilities/InitialParticleGeneration.h>

#include <mdst/dataobjects/MCParticleGraph.h>

#include <vector>

namespace Belle2 {

  /**
   * The Phokhara Generator module.
   * Generates radiative return events using the Phokhara FORTRAN generator.
   */
  class PhokharaInputModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the module parameters.
     */
    PhokharaInputModule();

    /** Destructor. */
    virtual ~PhokharaInputModule();

    /** Initializes the module. */
    virtual void initialize() override;

    /** Method is called for each event. */
    virtual void event() override;

    /** Method is called at the end of the event processing. */
    virtual void terminate() override;

  protected:

    /** Module parameters */
    int m_finalState; /**< Final state: mu+mu-(0), pi+pi-(1), 2pi0pi+pi-(2), 2pi+2pi-(3), ppbar(4), nnbar(5), K+K-(6), K0K0bar(7), pi+pi-pi0(8), lamb(->pi-p)lambbar(->pi+pbar)(9), eta pi+ pi- (10) */
    bool m_replaceMuonsByVirtualPhoton; /**< Replace muons by a virtual photon. */
    int m_nMaxTrials; /**< Events before loop is aborted. */
    int m_nSearchMax; /**< Events used to search maximum of differential cross section. */

    int m_weighted;  /**< generate weighted events */
    int m_LO;  /**< LO: ph0  Born: 1ph(0), Born: 0ph(1), only Born: 0ph(-1) */
    int m_NLO;  /**< 1 photon : Born(0), NLO(1) */
    int m_fullNLO;  /**< full NLO : No(0), Yes(1) */
    int m_QED;  /**< ISR only(0), ISR+FSR(1), ISR+INT+FSR(2) */
    int m_IFSNLO;  /**< IFSNLO: no(0), yes(1) */
    int m_alpha;  /**< vacuum polarization switch: off (0), on (1,[by Fred Jegerlehner]), on (2,[by Thomas Teubner]) */
    int m_pionff;  /**< FF_pion: KS PionFormFactor(0),GS old (1),GS new (2) */
    int m_pionstructure;  /**< for pi+pi- only: f0+f0(600): K+K- model(0), "no structure" model(1), no f0+f0(600)(2), f0 KLOE(3) */
    int m_kaonff;  /**< FF_kaon: KaonFormFactor constrained(0), KaonFormFactor unconstrained(1) KaonFormFactor old(2) */
    int m_narres;  /**< narr_res: no narrow resonances (0), J/psi (1) and psi(2S) (2) only for m_finalState = 0,1,6,7 */
    int m_protonff;  /**< ProtonFormFactor old(0), ProtonFormFactor new(1) */
    int m_chi_sw;  /**< chi_sw: Radiative return(0), Chi production(1), Radiative return + Chi production (2) */
    int m_be_r;  /**< be_r: without beam resolution(0), with beam resolution(1) */

    double m_cmsEnergy = -1.0; /**< CMS energy. */
    std::vector<double> m_ScatteringAngleRangePhoton; /**< Minimal/Maximal photon angle/missing momentum angle. */
    std::vector<double> m_ScatteringAngleRangeFinalStates; /**< Minimal/Maximal pions(muons,nucleons,kaons) momentum angle. */
    double m_MinInvMassHadronsGamma; /**< m_MinInvMassHadronsGamma [GeV^2]. */
    double m_MinInvMassHadrons; /**< m_MinInvMassHadrons [GeV^2]. */
    bool m_ForceMinInvMassHadronsCut; /**< Force application of the above cut. */
    double m_MaxInvMassHadrons; /**< m_MaxInvMassHadrons [GeV^2] */
    double m_MinEnergyGamma; /**< m_MinEnergyGamma [GeV]. */

    double m_epsilon; /**< Soft/hard photon separator*/
    double m_beamres; /**< beam resolution for chi2 studies*/
    std::string m_ParameterFile; /**< file that holds all resonance parameters*/
    bool m_BeamEnergySpread; /**< Simulate beam-energy spread. */

    /** Variables */
    Phokhara m_generator;   /**< The Phokhara generator. */
    MCParticleGraph m_mcGraph; /**< The MCParticle graph object. */

    template <typename T>
    std::vector<T> make_vector(T const& t1, T const& t2);  /**< make_vector. */

    template <typename T>
    std::pair<T, T> vectorToPair(std::vector<T>& vec, const std::string& name = "");

  private:
    /** Method is called to initialize the generator. */
    void initializeGenerator();

    bool m_initialized{false}; /**< True if generator has been initialized. */
    DBObjPtr<BeamParameters> m_beamParams; /**< BeamParameter. */

    InitialParticleGeneration m_initial; /**< initial particle used by BeamParameter class */

  }; /**< vectorToPair. */

  template <typename T>
  inline std::vector<T> PhokharaInputModule::make_vector(T const& t1, T const& t2)
  {
    std::vector<T> v;
    v.push_back(t1);
    v.push_back(t2);
    return v; /**< make_vector */
  }

  template <typename T>
  inline std::pair<T, T> PhokharaInputModule::vectorToPair(std::vector<T>& vec, const std::string& name)
  {
    std::pair<T, T> p;
    if (vec.size() != 2) {
      B2ERROR("The parameter " + name + " has to have exactly 2 elements !");
    } else {
      p.first  = vec[0];
      p.second = vec[1];
    }
    return p;  /**< vectorToPair. */
  }

} // end namespace Belle2

#endif /* PHOKHARAINPUTMODULE_H */
