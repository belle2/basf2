/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Cate MacQueen (UniMelb)                                  *
 * Last Updated: December 2017                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
/* Additional Info:
*
*/

#ifndef ECLCHARGEDPIDMODULE_H_
#define ECLCHARGEDPIDMODULE_H_

#include <framework/core/Module.h>
#include <string>
#include <TTree.h>
#include <TFile.h>

// ECL
#include <ecl/dataobjects/ECLSimHit.h>
#include <ecl/dataobjects/ECLHit.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLConnectedRegion.h>
#include <ecl/dataobjects/ECLLocalMaximum.h>
#include <ecl/dataobjects/ECLShower.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <ecl/dataobjects/ECLEventInformation.h>
#include <mdst/dataobjects/MCParticle.h>

// FRAMEWORK
#include <framework/gearbox/Unit.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// GEOMETRY
#include <ecl/geometry/ECLNeighbours.h>
#include <ecl/geometry/ECLGeometryPar.h>

namespace Belle2 {

  /** The ECL Data Analysis Module
   *
   * this module dump an ntuple containing ECL-related infos starting from mdst
   *
   */

  class ECLChargedPIDModule : public Module {

  public:

    /**
     * Constructor of the module.
     */
    ECLChargedPIDModule();

    /**
     * Destructor of the module.
     */
    virtual ~ECLChargedPIDModule();

    /**
     *Initializes the Module.
     */
    virtual void initialize();

    virtual void beginRun();

    virtual void event();

    virtual void endRun();

    /**
     * Termination action.
     */
    virtual void terminate();

  private:

    /** members of ECLReconstructor Module */

    TFile* m_rootFilePtr; /**< root file used for storing info */
    std::string m_rootFileName; /**< name of the root file */
    bool m_writeToRoot; /**< if true, a rootFile named by m_rootFileName will be filled with info */
    //int m_myHypoID;

    /** Store array: ECLShower. */
    StoreArray<ECLShower> m_eclShowers;
    /** Store object pointer: ECLEventInformation. */
    StoreObjPtr<ECLEventInformation> m_eclEventInformation;

    /** Default name ECLShower */
    virtual const char* eclShowerArrayName() const
    { return "ECLShowers" ; }

    StoreArray<MCParticle> m_mcParticles; /**< MCParticles StoreArray*/


    //N1 Hypo
    TTree* n1_tree; /**< Root tree and file for saving the output */
    //TFile* m_rootFile;

    // variables
    int n1_iExperiment; /**< Experiment number */
    int n1_iRun; /**< Run number */
    int n1_iEvent; /**< Event number */

    int n1_eclShowerMultip; /**< Number of ECLShowers per event */
    std::vector<double>* n1_eclShowerEnergy; /**< Shower Energy */
    std::vector<double>* n1_eclShowerTheta; /**< Shower Theta */
    std::vector<double>* n1_eclShowerPhi; /**< Shower Phi */
    std::vector<double>* n1_eclShowerR; /**< Shower R */
    std::vector<int>* n1_eclShowerHypothesisId; /**< Shower Particle Hypothesis ID */

    std::vector<double>* n1_eclShowerAbsZernike11; /**< Shower Zernike11 Moment */
    std::vector<double>* n1_eclShowerAbsZernike20; /**< Shower Zernike20 Moment */
    std::vector<double>* n1_eclShowerAbsZernike22; /**< Shower Zernike22 Moment */
    std::vector<double>* n1_eclShowerAbsZernike31; /**< Shower Zernike31 Moment */
    std::vector<double>* n1_eclShowerAbsZernike33; /**< Shower Zernike33 Moment */
    std::vector<double>* n1_eclShowerAbsZernike40; /**< Shower Zernike40 Moment */
    std::vector<double>* n1_eclShowerAbsZernike42; /**< Shower Zernike42 Moment */
    std::vector<double>* n1_eclShowerAbsZernike44; /**< Shower Zernike44 Moment */
    std::vector<double>* n1_eclShowerAbsZernike51; /**< Shower Zernike51 Moment */
    std::vector<double>* n1_eclShowerAbsZernike53; /**< Shower Zernike53 Moment */
    std::vector<double>* n1_eclShowerAbsZernike55; /**< Shower Zernike55 Moment */

    int n1_mcMultip; /**< Multiplicity of MCParticles */
    std::vector<int>* n1_mcPdg; /**< MCParticle PDG code */
    std::vector<int>* n1_mcMothPdg; /**< MCParticle mother particle PDG code */
    std::vector<double>* n1_mcEnergy; /**< MCParticle energyx */
    std::vector<double>* n1_mcP; /**< MCParticle momentum */
    std::vector<double>* n1_mcTheta; /**< MCParticle polar direction */
    std::vector<double>* n1_mcPhi; /**< MCParticle azimuthal direction */

    int n1_trkMultip; /**< Track Multiplicity */
    std::vector<int>* n1_trkPdg; /**< Track PDG code */
    std::vector<int>* n1_trkCharge; /**< Track charge */
    std::vector<double>* n1_trkP; /**< Track momentum */
    std::vector<double>* n1_trkTheta; /**< Track polar direction */
    std::vector<double>* n1_trkPhi; /**< Track azimuthal direction */
    std::vector<double>* n1_trkR; /**< Track R */

    std::vector<double>* n1_eclEoP; /**< ECL Shower Energy on reconstructed Track momentum */


    //N2 Hypo
    TTree* n2_tree; /**< Root tree and file for saving the output */

    //variables
    int n2_iExperiment; /**< Experiment number */
    int n2_iRun; /**< Run number */
    int n2_iEvent; /**< Event number */

    int n2_eclShowerMultip; /**< Number of ECLShowers per event */
    std::vector<double>* n2_eclShowerEnergy; /**< Shower Energy */
    std::vector<double>* n2_eclShowerTheta; /**< Shower Theta */
    std::vector<double>* n2_eclShowerPhi; /**< Shower Phi */
    std::vector<double>* n2_eclShowerR; /**< Shower R */
    std::vector<int>* n2_eclShowerHypothesisId; /**< Shower Particle Hypothesis ID */

    std::vector<double>* n2_eclShowerAbsZernike11; /**< Shower Zernike11 Moment */
    std::vector<double>* n2_eclShowerAbsZernike20; /**< Shower Zernike20 Moment */
    std::vector<double>* n2_eclShowerAbsZernike22; /**< Shower Zernike22 Moment */
    std::vector<double>* n2_eclShowerAbsZernike31; /**< Shower Zernike31 Moment */
    std::vector<double>* n2_eclShowerAbsZernike33; /**< Shower Zernike33 Moment */
    std::vector<double>* n2_eclShowerAbsZernike40; /**< Shower Zernike40 Moment */
    std::vector<double>* n2_eclShowerAbsZernike42; /**< Shower Zernike42 Moment */
    std::vector<double>* n2_eclShowerAbsZernike44; /**< Shower Zernike44 Moment */
    std::vector<double>* n2_eclShowerAbsZernike51; /**< Shower Zernike51 Moment */
    std::vector<double>* n2_eclShowerAbsZernike53; /**< Shower Zernike53 Moment */
    std::vector<double>* n2_eclShowerAbsZernike55; /**< Shower Zernike55 Moment */

    int n2_mcMultip; /**< Multiplicity of MCParticles */
    std::vector<int>* n2_mcPdg; /**< MCParticle PDG code */
    std::vector<int>* n2_mcMothPdg; /**< MCParticle mother particle PDG code */
    std::vector<double>* n2_mcEnergy; /**< MCParticle energyx */
    std::vector<double>* n2_mcP; /**< MCParticle momentum */
    std::vector<double>* n2_mcTheta; /**< MCParticle polar direction */
    std::vector<double>* n2_mcPhi; /**< MCParticle azimuthal direction */

    int n2_trkMultip; /**< Track Multiplicity */
    std::vector<int>* n2_trkPdg; /**< Track PDG code */
    std::vector<int>* n2_trkCharge; /**< Track charge */
    std::vector<double>* n2_trkP; /**< Track momentum */
    std::vector<double>* n2_trkTheta; /**< Track polar direction */
    std::vector<double>* n2_trkPhi; /**< Track azimuthal direction */
    std::vector<double>* n2_trkR; /**< Track R */

    std::vector<double>* n2_eclEoP; /**< ECL Shower Energy on reconstructed Track momentum */

  };
}

#endif
