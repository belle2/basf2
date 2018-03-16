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
#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>

// ROOT
#include <TFile.h>
#include <TTree.h>

// other stuff
#include <string>

// boost
#include <boost/any.hpp>
#include <boost/iterator/iterator_concepts.hpp>
namespace Belle2 {

  /**
   * Module for analysing the phase space of genfit::TrackCand(s) and SpacePointTrackCand(s)
   * NOTE: this is just a very simple module that takes the MCParticleID of any TC and collects some information from them
   */
  class PhaseSpaceAnalysisModule : public Module {

  public:
    PhaseSpaceAnalysisModule(); /**< Constructor */

    void initialize() override; /**< initialize */
    void event() override; /**< event */
    void terminate() override; /**< terminate */

    /** helper class to have all RootVariables assembled in one container */
    struct RootVariables {
      std::vector<double> MomX; /**< x momentum */
      std::vector<double> MomY; /**< y momentum */
      std::vector<double> MomZ; /**< z momentum */

      /** pseudo rapidity. could also be calculated from momentum variables afterwards. here only for convienience */
      std::vector<double> Eta;
      /** transverse momentum. could also be calculated from momentum variables afterwards. here only for convienience */
      std::vector<double> pT;

      std::vector<double> VertX; /**< x position of vertex*/
      std::vector<double> VertY; /**< y position of vertex*/
      std::vector<double> VertZ; /**< z position of vertex*/

      std::vector<int> PDG; /**< mc particle pdg code */
      std::vector<double> Charge; /**< mc particle charge */

      std::vector<double> Energy; /**< mc particle energy */
      std::vector<double> Mass; /**< mc particle mass */

//       std::vector<unsigned short int> RefereeStatus; /**< refereeStatus of a SPTC (if available) */
    };

    /** enum to differentiate between possible trackCandidate types */
    enum e_trackCandType {
      c_gftc, /**< genfit::TrackCand */
      c_sptc, /**< SpacePointTrackCand */
    };

  protected:

    // ======================================== PARAMETERS ========================================================================
    std::vector<std::string> m_PARAMcontainerNames; /**< collection name of trackCands to be investigated */

    std::vector<std::string> m_PARAMtrackCandTypes; /**< types of the track candidates in containerNames */

    std::vector<std::string> m_PARAMrootFileName; /**< name of the output root file name */

    /** take containers pair-wise and only analize those particles that are found in one container but not in the other */
    bool m_PARAMdiffAnalysis;

    // ======================================== INTERNALLY USED MEMBERS ===========================================================
    std::vector<std::pair<boost::any, e_trackCandType> > m_tcStoreArrays; /**< StoreArrays of the containers */

    RootVariables m_rootVariables; /**< root variables used for collecting data event-wise */

    std::vector<TTree*> m_treePtrs; /**< each container name gets its own tree in the root file */

    TFile* m_rootFilePtr; /**< Pointer to root file */

    std::vector<std::string> m_treeNames;  /**< names of the trees that are stored in the root file */
    // ==================================================== COUNTERS ==============================================================
    unsigned int m_noMcPartCtr; /**< Counter for NULL pointers to MC Particles (should always be zero. just a safety measure) */

    unsigned int m_skippedTCsCtr; /**< Counter for negative MC particle ids (should always be zero. safety measure) */

    std::vector<unsigned int> m_mcPartCtr; /**< container wise coutner of TrackCands / MCParticles */

    /** initialize all counters to 0 to avoid undefined behavior */
    void initializeCounters(size_t nContainers)
    {
      m_noMcPartCtr = 0;
      m_skippedTCsCtr = 0;

      m_mcPartCtr = std::vector<unsigned int>(nContainers, 0);
    }

    /** create a root file with file name @param fileName and write option @param writeOption 'UPDATE' or 'RECREATE' */
    void initializeRootFile(std::string fileName, std::string writeOption, std::vector<std::string> treeNames);
    void getValuesForRoot(Belle2::MCParticle* mcParticle, RootVariables& rootVariables); /**< collect the values for root output */

    /** get all MCPartileIDs of tracks */
    template<typename TrackCandType>
    std::vector<int> getMCParticleIDs(Belle2::StoreArray<TrackCandType> trackCands);

//     /** get all referee statuses from all SpacePointTrackCand */
//     std::vector<unsigned short int> getRefereeStatuses(Belle2::StoreArray<Belle2::SpacePointTrackCand> trackCands);

    /** the first vector in allIds is used as reference to which all other vectors of allIDs are compared.
     * The values that cannot be found in these vectors but are in the reference are collected for each vector.
     * @returns the original allIDs interlaced with the vectors of mcPartIds that were in the reference but not in the vector
     */
    std::vector<std::vector<int> > getDiffIds(const std::vector<std::vector<int> >& allIDs);

  };
}