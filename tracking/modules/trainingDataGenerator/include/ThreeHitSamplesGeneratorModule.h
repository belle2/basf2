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
   * Combines the spatial coordinates of two Combinations (consisting of 2 SpacePoints each)
   * where both Combinations share one hit. Thus a training sample consists of three SpacePoint
   * coordinates and a boolean value that flags if this combination of SpacePoints is part
   * of a track or not.
   *
   * NOTE: this is still in a developement stage!
   */
  class ThreeHitSamplesGeneratorModule : public Module {

  public:
    ThreeHitSamplesGeneratorModule(); /**< Constructor */

    virtual void initialize(); /**< initialize */

    virtual void event(); /**< event */

    virtual void terminate(); /**< terminate */

    /** helper struct to keep the root variables contained in this module*/
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

    // ================================================== member functions =========================================================

    /** initialize the root file (set all branch-adresses and tree-names)*/
    void initializeRootFile(const std::string& filename, const std::string& writemode);

    /**
     * split a SpacePointTrackCand into as many N hit combinations as possible
     * @param trackCand, the SpacePointTrackCand to be split
     * @param nHits, the number of hits that form a combination
     *
     * NOTE: at the moment only combinations where the three hits are on three different but subsequent layers are produced!
     *
     * TODO: further documentation and decide how to handle trackCands with missing layers
     */
    std::vector<Belle2::SpacePointTrackCand> splitTrackCandNHitCombos(const Belle2::SpacePointTrackCand* trackCand, unsigned nHits);

    /**
     * decide if a three hit combination is a valid combination
     * NOTE: this function does not decide if a combination is labeled as signal or as background, but only decides if it even
     * reaches the stage where this decision is formed
     *
     * This function can be used to sort out combinations with hits on the same layer, etc...
     * TODO: define what a valid combination is. at the moment this returns true always!
     */
    bool isValidHitCombination(const std::vector<const Belle2::SpacePoint*>& combination);

    /**
     * add a complete three hit combination to the m_combinations member
     * NOTE: in a final stage it has to be made sure that the SpacePoints in the combination are always
     */
    void addHitCombination(const Belle2::SpacePointTrackCand& combination, bool signal, RootCombinations& combinations);
  };
}
