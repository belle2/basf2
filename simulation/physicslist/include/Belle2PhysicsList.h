/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef BELLE2PHYSICSLIST_H
#define BELLE2PHYSICSLIST_H

#include "G4VModularPhysicsList.hh"

namespace Belle2 {

  namespace Simulation {

    /**
     * Custom Geant4 physics list for Belle II with options to
     * add optical physics, standard EM physics and high precision
     * neutrons
     */
    class Belle2PhysicsList: public G4VModularPhysicsList {

    public:
      /** Constructor */
      explicit Belle2PhysicsList(const G4String& physicsListName);
      /** destructor */
      ~Belle2PhysicsList();

      /** Build all particle types used in physics list */
      virtual void ConstructParticle();

      /** Set the secondary particle production thresholds */
      virtual void SetCuts();

      /** Run/event verbosity level */
      void SetVerbosity(G4int verb);

      /** Use parameter to set global cut value */
      void SetProductionCutValue(G4double);

      /** Set cut value for PXD envelope */
      void SetPXDProductionCutValue(G4double);

      /** Set cut value for SVD envelope */
      void SetSVDProductionCutValue(G4double);

      /** Set cut value for CDC envelope */
      void SetCDCProductionCutValue(G4double);

      /** Set cut value for ARICH and TOP envelopes */
      void SetARICHTOPProductionCutValue(G4double);

      /** Set cut value for ECL barrel, forward and backward envelopes */
      void SetECLProductionCutValue(G4double);

      /** Set cut value for BKLM and EKLM envelopes */
      void SetKLMProductionCutValue(G4double);

      /** Use standard EM physics instead of EM option1 */
      void UseStandardEMPhysics(G4bool);

      /** Add optical photon physics */
      void UseOpticalPhysics(G4bool);

      /** Use high precision neutron models below 20 MeV */
      void UseHighPrecisionNeutrons(G4bool);

      /** Simulate neutral long-lived particles with given pdg and mass value */
      void UseLongLivedNeutralParticles();

    private:
      /** Set the produciton cuts to the given value for a list of regions belonging to a sub detector
       * @param name name of the sub detector to print in messages
       * @param regions list of region names to set the cut value
       * @param cutValue production cut in cm, if 0 the global cut value will be used
       */
      void setRegionCuts(const std::string& name, const std::vector<std::string>& regions, double cutValue);

      /** Construct parallel particle types needed for reco */
      void ConstructG4eParticles();

      /** Secondary production thresholds */
      G4double m_globalCutValue;

      G4double m_pxdCutValue;       /**< threshold for PXD */
      G4double m_svdCutValue;       /**< threshold for SVD */
      G4double m_cdcCutValue;       /**< threshold for CDC */
      G4double m_arichtopCutValue;  /**< threshold for ARICH and TOP */
      G4double m_eclCutValue;       /**< threshold for ECL */
      G4double m_klmCutValue;       /**< threshold for BKLM and EKLM */
    };

  } // end of namespace Simulation

} // end of namespace Belle2

#endif // BELLE2PHYSICSLIST_H
