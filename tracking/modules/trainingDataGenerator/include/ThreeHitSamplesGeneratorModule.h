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
#include <framework/core/Module.h>
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

    virtual void initialize(); /**< initialize */
    virtual void event(); /**< event */
    virtual void terminate(); /**< terminate */

  protected:

    std::string m_PARAMcontainerName; /**< name of SpacePoint container in StoreArray */

    std::vector<std::string> m_PARAMoutputFileName; /**< name of output file (root file) that holds the training samples */

    TFile* m_rootFilePtr; /**< pointer to the root output file */

    TTree* m_treePtr; /**< pointer to tree in root output file */

    /** initialize the root file (set all branch-adresses and tree-names)*/
    void initializeRootFile(const std::string& filename, const std::string& writemode);
  };
}
