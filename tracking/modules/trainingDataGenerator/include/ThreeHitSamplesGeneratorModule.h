/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Madlener                                          *
 *                                                                        *
 **************************************************************************/

#pragma once

// framework
#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <framework/core/Module.h>

// stl
#include <string>

#include <TFile.h>
#include <TTree.h>

namespace Belle2 {

  /**
   * Module for generating training samples for supervised training of a MLP
   * Combines the spatial coordinates of two Segments (consisting of 2 SpacePoints each)
   * where both Segments share one hit. Thus a training sample consists of three SpacePoint
   * coordinates and a boolean value that flags if this combination of SpacePoints is part
   * of a track or not.
   *
   * NOTE: this is still in a developement stage!
   */
  class ThreeHitSamplesGeneratorModule : public Module {

  public:
    ThreeHitSamplesGeneratorModule(); /**< Constructor */

    void initialize() override; /**< initialize */

    void event() override; /**< event */

    void terminate() override; /**< terminate */

    /** helper struct to keep the root variables contained in this module
     * TODO: make this cleaner (i.e. less ugly)
     */
    struct RootCombinations {
      std::vector<double> Hit1X; /**< (global) X-position of first SpacePoint of three hit combination */
      std::vector<double> Hit1Y; /**< (global) Y-position of first SpacePoint of three hit combination */
      std::vector<double> Hit1Z; /**< (global) Z-position of first SpacePoint of three hit combination */

      std::vector<double> Hit2X; /**< (global) X-position of second SpacePoint of three hit combination */
      std::vector<double> Hit2Y; /**< (global) Y-position of second SpacePoint of three hit combination */
      std::vector<double> Hit2Z; /**< (global) Z-position of second SpacePoint of three hit combination */

      std::vector<double> Hit3X; /**< (global) X-position of third SpacePoint of three hit combination */
      std::vector<double> Hit3Y; /**< (global) Y-position of third SpacePoint of three hit combination */
      std::vector<double> Hit3Z; /**< (global) Z-position of third SpacePoint of three hit combination */

      std::vector<unsigned short> VxdId1; /**< Layer number of first SpacePounsigned short of the three hit combination */
      std::vector<unsigned short> VxdId2; /**< Layer number of second SpacePounsigned short of the three hit combination */
      std::vector<unsigned short> VxdId3; /**< Layer number of third SpacePounsigned short of the three hit combination */

      std::vector<double> pT; /**< pT of associated MCParticle */
      std::vector<double> Momentum; /**< total momentum of associated MCParticle */
      std::vector<int> PDG; /**< pdg-code of associated MCParticle */
      std::vector<double> Charge; /**< charge of associated MCParticle */

      std::vector<bool> Signal; /**< flag for specifying if combination is signal or noise */
    };

  protected:

    // ================================================== parameter members ========================================================

    std::string m_PARAMcontainerName; /**< name of SpacePoint container in StoreArray */

    std::vector<std::string> m_PARAMoutputFileName; /**< name of output file (root file) that holds the training samples */

    // ================================================== internal members =========================================================

    TFile* m_rootFilePtr; /**< pointer to the root output file */

    TTree* m_treePtr; /**< pointer to tree in root output file */

    /** RootCombinations to store information eventwise (avoid writing to root file more than once per event)*/
    RootCombinations m_combinations;

    // ==================================================== COUNTER VARIABLES ======================================================

    unsigned m_combCtr; /**< total number of created hit combinations */

    unsigned m_noiseSampleCtr; /**< total number of noise samples written to root file */

    unsigned m_signalSampleCtr; /**< total number of signal samples written to root file */

    unsigned m_invalidCombiCtr; /**< total number of combinations that were ruled invalid by isValidHitCombination*/

    unsigned m_smallContainerCtr; /**< total number of SPTCs that did not have enough hits (3 in this case)*/

    void initializeCounters()
    {
      m_combCtr = 0;
      m_noiseSampleCtr = 0;
      m_signalSampleCtr = 0;
      m_invalidCombiCtr = 0;
      m_smallContainerCtr = 0;
    } /**< initialize all counters to zero. */

    // ================================================== member functions =========================================================

    /** initialize the root file (set all branch-adresses and tree-names)*/
    void initializeRootFile(const std::string& filename, const std::string& writemode);

    /**
     * split a SpacePointTrackCand into as many N hit combinations as possible
     * @param trackCand, the SpacePointTrackCand to be split
     * @param nHits, the number of hits that form a combination
     *
     * NOTE: at only combinations that pass isValidHitCombination are produced, see the documentation there on the conditions
     *
     */
    std::vector<Belle2::SpacePointTrackCand> splitTrackCandNHitCombos(const Belle2::SpacePointTrackCand* trackCand, unsigned nHits);

    /**
     * decide if a three hit combination is a valid combination
     * NOTE: this function does not decide if a combination is labeled as signal or as background, but only decides if it even
     * reaches the stage where this decision is formed
     *
     * This function can be used to sort out combinations with hits on the same layer, etc...
     * At the moment the function checks if the SpacePoints of the combination are on subsequent layers (i.e. if two SpacePoints are
     * on the same layer the combination is invalid)
     *
     * NOTE: this is only a temporary solution at the moment
     * TODO: decide which cases should be covered by the approach and adapt this function accordingly
     */
    bool isValidHitCombination(const std::vector<const Belle2::SpacePoint*>& combination);

    /**
     * add a complete three hit combination to the m_combinations member
     * NOTE: in a final stage it has to be made sure that the SpacePoints in the combination are always in the right order
     */
    void addHitCombination(const Belle2::SpacePointTrackCand& combination, bool signal, int mcId, RootCombinations& combinations);
  };
}
