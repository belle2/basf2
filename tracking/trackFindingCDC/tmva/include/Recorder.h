/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/varsets/NamedFloatTuple.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/pcore/RootMergeable.h>
#include <TFile.h>
#include <TTree.h>
#include <functional>
#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class to fill a tree.
    class Recorder {
    public:
      /** Construct the Recorder opening the given ROOT file and
       *  create the underlying TTree and add let the given function setup branches.
       *
       *  @param setBranches    Function invoked with the underlying TTree as argument
       *                        which is supposed create Branches and provide the value adresses.
       *  @param rootFileName   Name of ROOT file to which should be written.
       *  @param treeName       Name of the TTree in the ROOT file.
       */
      Recorder(const std::function<void(TTree&)> setBranches,
               const std::string& rootFileName,
               const std::string& treeName = "recorded_tree");


      /** Construct the Recorder opening the given ROOT file and
       *  create the underlying TTree and add variable names and values addresses as branches to it.
       *
       *  @param allVariables   A sequence of named values that are used as addresses of branches
       *  @param rootFileName   Name of ROOT file to which should be written.
       *  @param treeName       Name of the TTree in the ROOT file.
       */
      Recorder(const std::vector<NamedFloatTuple*>& allVariables,
               const std::string& rootFileName,
               const std::string& treeName = "recorded_tree");

      /// Destructor writing the TTree and closing the ROOT File.
      ~Recorder();

      /// Write all captured variables to disk.
      void write();

      /// Capture the registered variable values and write them out.
      void capture();

    private:
      // Reference to the open TFile.
      TFile* m_tFile;

      // Reference to the TTree.
      StoreObjPtr<RootMergeable<TTree>> m_tTree;
    };

  }
}
