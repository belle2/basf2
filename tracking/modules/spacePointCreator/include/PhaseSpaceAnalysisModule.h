/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Madlener                                          *
 *                                                                        *
 **************************************************************************/

# pragma once

// framework
#include <framework/core/Module.h>

// ROOT
#include <TFile.h>
#include <TTree.h>

// other stuff
#include <string>

namespace Belle2 {

  /**
   * Module for analysing the phase space of a genfit::TrackCand (resp. the MCParticle that is related to the TrackCand)
   */
  class PhaseSpaceAnalysisModule : public Module {

  public:
    PhaseSpaceAnalysisModule(); /**< Constructor */

    virtual void initialize(); /**< initialize */
    virtual void event(); /**< event */
    virtual void terminate(); /**< terminate */

  protected:
    std::vector<std::string> m_PARAMgfTCNames; /**< collection name of genfit::TrackCands to be investigated */
    std::vector<std::string> m_PARAMrootFileName; /**< name of the output root file name */

    bool diffAnalysis;

    unsigned int m_gftcCtr; /**< Counter for collected GFTCs / MC Particles */
    unsigned int m_noMcPartCtr; /**< Counter for NULL pointers to MC Particles (should always be zero. just a safety measure) */
    unsigned int m_skippedTCsCtr; /**< Counter for negative MC particle ids (should always be zero. safety measure) */
    unsigned int m_noMissingTCsCtr; /**< Counter for counting events in which at least one GFTC was missing after conversion */


    struct RootVariables { // TODO
      std::vector<double> MomX; /**< x momentum */
      std::vector<double> MomY; /**< y momentum */
      std::vector<double> MomZ; /**< z momentum */

      std::vector<double> Eta; /**< pseuod rapidity. could also be calculated from momentum variables afterwards. here only for convienience */
      std::vector<double> pT; /**< transverse momentum. could also be calculated from momentum variables afterwards. here only for convienience */

      std::vector<double> VertX; /**< x position of vertex*/
      std::vector<double> VertY; /**< y position of vertex*/
      std::vector<double> VertZ; /**< z position of vertex*/

      std::vector<int> PDG; /**< mc particle pdg code */
      std::vector<double> Charge; /**< mc particle charge */

      std::vector<double> Energy; /**< mc particle energy */
      std::vector<double> Mass; /**< mc particle mass */
    };

    void initializeRootFile(std::string fileName, std::string writeOption); /**< create a root file with file name @param fileName and write option @param writeOption 'UPDATE' or 'RECREATE');
     */
    void writeToRoot(RootVariables& rootVariables); /**< write variables to root file, only called once per event and not always when rootVariables is updated */
    void getValuesForRoot(Belle2::MCParticle* mcParticle, RootVariables& rootVariables); /**< collect the values for root output */

  private:
    TFile* m_rootFilePtr; /**< Pointer to root file */
    TTree* m_treePtr; /**< Pointer to tree in root file */

    std::vector<double> m_rootMomX; /**< x momentum */
    std::vector<double> m_rootMomY; /**< y momentum */
    std::vector<double> m_rootMomZ; /**< z momentum */

    std::vector<double> m_rootEta; /**< pseuod rapidity. could also be calculated from momentum variables afterwards. here only for convienience */
    std::vector<double> m_rootPt; /**< transverse momentum. could also be calculated from momentum variables afterwards. here only for convienience */

    std::vector<double> m_rootVertX; /**< x position of vertex*/
    std::vector<double> m_rootVertY; /**< y position of vertex*/
    std::vector<double> m_rootVertZ; /**< z position of vertex*/

    std::vector<double> m_rootCharge; /**< mc particle charge */
    std::vector<double> m_rootEnergy; /**< mc particle energy */
    std::vector<double> m_rootMass; /**< mc particle mass */
    std::vector<int> m_rootPdg; /**< mc particle pdg code */
  };

}