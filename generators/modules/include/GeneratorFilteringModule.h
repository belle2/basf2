#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/MCInitialParticles.h>
#include <mdst/dataobjects/MCParticle.h>
#include <TVector3.h>
#include <string>

namespace Belle2 {
  /**
   * generator filtering
   *
   */
  class GeneratorFilteringModule : public Module {
  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    GeneratorFilteringModule();

    /** Initialise the parameters */
    void initialize() override;

    /** Event processor */
    void event() override;

  protected:
    /** Called for each particle, performs basic cut checks */
    void checkParticle(const MCParticle& mc);

    std::string m_particleList; /**< The name of the MCParticle collection. */
    StoreArray<MCParticle> m_mcparticles; /**< Store array for the MCParticles */
    StoreObjPtr<MCInitialParticles> m_initial; /**< Pointer to the actual beam parameters */

  private:
    int m_nSignalLepton = 0; /**< Number of good signal leptons */
    int m_nTauLepton = 0; /**< Number of good tau leptons counted */
    int m_goodEvent = 0; /**< Flag indicating if event should be retained */

    double m_missingPx = 0.0; /**< Sum of px components for neutrinos in event */
    double m_missingPy = 0.0; /**< Sum of py components for neutrinos in event */
    double m_missingPz = 0.0; /**< Sum of pz components for neutrinos in event */
    double m_missingE = 0.0; /**< Sum of energy for neutrinos in event */

    TVector3 m_signalLeptonPVec; /**< Vector storing momentum components of signal lepton */
    std::vector<TVector3> m_tauLeptonPVecs; /**< Vector of momentum vectors for tau leptons */
    double m_signalLeptonZ = 0.0; /**< Storing the production vertex z-component for signal lepton */
    std::vector<double> m_tauLeptonZs; /**< Vector of production vertex z-components for tau leptons */

    double m_signalLeptonPDG; /**< PDG code of the signal lepton. */
    double m_signalLeptonPMin; /**< Minimum momentum (CMS) for signal lepton. */
    double m_signalLeptonPMax; /**< Maximum momentum (CMS) for signal lepton.  */
    double m_tauLeptonPMin; /**< Minimum momentum (CMS) for tau lepton. */
    double m_tauLeptonPMax; /**< Maximum momentum (CMS) for tau lepton.  */
    double m_projectionMin; /**< Minimum value for projection of tau lepton onto signal lepton momentum (in CMS) */
    double m_projectionMax; /**< Maximum value for projection of tau lepton onto signal lepton momentum (in CMS) */
    double m_angleMin; /**< Minimum value for the angle between the tau and signal lepton momentum vectors (in CMS) */
    double m_angleMax; /**< Maximum value for the angle between the tau and signal lepton momentum vectors (in CMS) */
    double m_zDiffMin; /**< Minimum value for difference between production vertex z-component for signal and tau leptons */
    double m_zDiffMax; /**< Maximum value for difference between production vertex z-component for signal and tau leptons */
    double m_UMin; /**< Minimum value of U = E - p (calculated using vector sum of momenta and energy for neutrinos in CMS) */
    double m_UMax; /**< Maximum value of U = E - p (calculated using vector sum of momenta and energy for neutrinos in CMS) */
  };
}

