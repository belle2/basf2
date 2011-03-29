/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Zbynek Drasal, Peter Kvasnicka                           *
 *         Based on Geant4 code by author(s) listed below.      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
//
// ********************************************************************
// * DISCLAIMER                                                       *
// *                                                                  *
// * The following disclaimer summarizes all the specific disclaimers *
// * of contributors to this software. The specific disclaimers,which *
// * govern, are listed with their locations in:                      *
// *   http://cern.ch/geant4/license                                  *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.                                                             *
// *                                                                  *
// * This  code  implementation is the  intellectual property  of the *
// * GEANT4 collaboration.                                            *
// * By copying,  distributing  or modifying the Program (or any work *
// * based  on  the Program)  you indicate  your  acceptance of  this *
// * statement, and all its terms.                                    *
// ********************************************************************
//
// -------------------------------------------------------------------
//
// This product includes software developed by Members of the Geant4
// Collaboration ( http://cern.ch/geant4 ).
//
// GEANT4 Classes utilized:
//
// File names:    G4UniversalFluctuation (by Vladimir Ivanchenko)
//                G4MollerBhabhaModel (by Vladimir Ivanchenko)
//                G4MuBetheBlochModel (by Vladimir Ivanchenko)
//                G4BetheBlochModel (by Vladimir Ivanchenko)
//
// -------------------------------------------------------------------

#ifndef SIENERGYFLUCT_H
#define SIENERGYFLUCT_H

#include <generators/dataobjects/MCParticle.h>
#include <TRandom3.h>

namespace Belle2 {

  /**
   * Special class providing particle energy loss fluctuations in Si material (Landau fluctuations). The main method
   * simply follows the strategy taken in Geant4 class G4UniversalFluctuation by V. Ivanchenko. As the fluctuation is
   * strongly dependent on particle type and it's energy, detailed calculations of mean ionisation losses have been
   * implemented as well. The differ for hadrons (follows Geant4 class G4BetheBlochModel), muons (follows Geant4
   * class G4MuBetheBlochModel) and electrons & positrons (follows Geant4 class G4MollerBhabhaModel) ... All the details
   * about physics used can be found in http://cern.ch/geant4/UserDocumentation/UsersGuides/PhysicsReferenceManual/fo/PhysicsReferenceManual.pdf
   *
   * @author Z. Drasal, Charles University, Prague
   * basf2 implementation: Peter Kvasnicka, Charles Univesity, Prague
   */
  class SiEnergyFluct {

  public:

    /**
     * Constructor.
     */
    SiEnergyFluct(double cutOnDeltaRays);

    /**
     * Destructor.
     */
    ~SiEnergyFluct();

    /**
     * Calculate energy loss fluctuations.
     * The model used to get the fluctuations is
     * essentially the same as in Glandz in Geant3
     * (Cern program library W5013, phys332). L. Urban
     * et al. NIM A362, p.416 (1995) and Geant4 - Physics
     * Reference Manual.
     */
    double SampleFluctuations(const MCParticle * part, const double length);

  protected:

  private:

    /**
     * Calculate actual dEdx for hadrons.
     * Based on the ComputeDEDXPerVolume method from
     * G4BetheBlochModel Geant4 class.
     */
    double getHadronDEDX(const MCParticle * part);

    /**
     * Calculate actual dEdx for muons.
     * Based on ComputeDEDXPerVolume method from G4MuBetheBlochModel
     * Geant4 class.
     */
    double getMuonDEDX(const MCParticle * part);

    /**
     * Calculate actual dEdx for electrons & positrons.
     * Bbased on ComputeDEDXPerVolume method
     * G4MollerBhabhaModel from Geant4 class.
     */
    double getElectronDEDX(const MCParticle * part);

    // Data members
    TRandom3 m_random;        /**< Random generator. */

    const MCParticle * m_prevMCPart;  /**< Pointer to the most recent MCParticle referenced. */
    double m_prevMeanLoss;      /**< Most recent dE/dx calculated by the SampleFluctuations method. */
    double m_cutOnDeltaRays;    /**< Cut on secondary electrons - must be the same as in Geant4. */

    double m_charge;        /**< Charge of the current MCParticle. */

    double m_twoln10;       /**< A constant related to dE/dx. */

    double m_eexc;          /**< A constant related to silicon. */
    double m_eexc2;         /**< A constant related to silicon. */
    double m_KBethe;          /**< A constant related to silicon. */
    double m_Zeff;          /**< A constant related to silicon. */
    double m_th;            /**< A constant related to silicon. */

    double m_aden;  /**< A constants related to silicon & dEdx -> density effect. */
    double m_cden;  /**< A constants related to silicon & dEdx -> density effect. */
    double m_mden;  /**< A constants related to silicon & dEdx -> density effect. */
    double m_x0den; /**< A constants related to silicon & dEdx -> density effect. */
    double m_x1den; /**< A constants related to silicon & dEdx -> density effect. */

    double m_xgi[8];  /**< Constants related to silicon & dEdx -> density effect. */
    double m_wgi[8];  /**< Constants related to silicon & dEdx -> density effect. */

    double m_limitKinEnergy;  /**< A constants related to silicon & dEdx -> density effect. */
    double m_logLimitKinEnergy; /**< A constants related to silicon & dEdx -> density effect. */
    double m_alphaPrime;      /**< A constants related to silicon & dEdx -> density effect. */

    double m_minLoss; /**< A constant related to silicon & universal fluctuations. */
    double m_minNumberInteractionsBohr; /**< A constant related to silicon & universal fluctuations. */
    double m_nmaxCont1; /**< A constant related to silicon & universal fluctuations. */
    double m_nmaxCont2; /**< A constant related to silicon & universal fluctuations. */

    double m_facwidth;  /**< A constant related to silicon & universal fluctuations. */
    double m_f1Fluct; /**< A constant related to silicon & universal fluctuations. */
    double m_f2Fluct; /**< A constant related to silicon & universal fluctuations. */
    double m_e1Fluct; /**< A constant related to silicon & universal fluctuations. */
    double m_e2Fluct; /**< A constant related to silicon & universal fluctuations. */
    double m_e1LogFluct;  /**< A constant related to silicon & universal fluctuations. */
    double m_e2LogFluct;  /**< A constant related to silicon & universal fluctuations. */
    double m_ipotFluct; /**< A constant related to silicon & universal fluctuations. */
    double m_ipotLogFluct;  /**< A constant related to silicon & universal fluctuations. */
    double m_e0;      /**< A constant related to silicon & universal fluctuations. */

  }; // Class

} // Namespace

#endif // SIENERGYFLUCT_H
