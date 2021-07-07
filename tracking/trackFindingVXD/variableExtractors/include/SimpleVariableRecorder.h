/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <functional>
#include <string>
#include <vector>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/pcore/RootMergeable.h>
#include <framework/pcore/ProcHandler.h>
#include <tracking/trackFindingVXD/utilities/Named.h>
#include <TFile.h>
#include <TTree.h>
#include <TROOT.h>


namespace Belle2 {

  /** Class to write collected variables into a root file,
   * Used by VXDQETrainingDataCollectorModule
   */
  class SimpleVariableRecorder {

  public:

    /**
     *  Construct the Recorder opening the given ROOT file and
     *  create the underlying TTree and add let the given function setup branches.
     *
     *  @param setBranches    Function invoked with the underlying TTree as argument
     *                        which is supposed create Branches and provide the value adresses.
     *  @param rootFileName   Name of ROOT file to which should be written.
     *  @param treeName       Name of the TTree in the ROOT file.
     */
    SimpleVariableRecorder(const std::function<void(TTree&)>& setBranches,
                           const std::string& rootFileName,
                           const std::string& treeName = "recorded_tree") :
      m_tFile(nullptr),
      m_tTree(treeName, DataStore::c_Persistent)
    {
      TDirectory* ptrSavedCurrentTDirectory = gDirectory;

      m_tFile = new TFile(rootFileName.c_str(), "RECREATE");
      m_tFile->cd();

      m_tTree.registerInDataStore(DataStore::c_DontWriteOut | DataStore::c_ErrorIfAlreadyRegistered);
      m_tTree.construct(treeName.c_str(), treeName.c_str());
      if (m_tTree) {
        setBranches(m_tTree->get());
      }

      if (ptrSavedCurrentTDirectory) {
        ptrSavedCurrentTDirectory->cd();
      } else {
        gROOT->cd();
      }
    }

    /**
     *  Construct the Recorder opening the given ROOT file and
     *  create the underlying TTree and add let the given function setup branches.
     *
     *  @param namedVariables  Vector of collected named varibles to be turned into Branches in a TTree
     *  @param fileName   Name of ROOT file to which should be written.
     *  @param treeName       Name of the TTree in the ROOT file.
     */
    // cppcheck does not recognize that m_tfile is initialized by calling the other constructor
    // cppcheck-suppress uninitMemberVar
    SimpleVariableRecorder(std::vector<Named<float*>>& namedVariables, const std::string& fileName,
                           const std::string& treeName) :
      SimpleVariableRecorder([ & namedVariables](TTree & tree)
    {
      for (auto& variable : namedVariables) {
        tree.Branch(variable.getName().c_str(), variable.getValue());
      }
    }, fileName, treeName)
    { }

    /// Destructor that closes used TTrees and TFiles
    ~SimpleVariableRecorder()
    {
      if (!ProcHandler::parallelProcessingUsed() or ProcHandler::isOutputProcess()) {
        if (m_tTree) {
          m_tTree->get().SetDirectory(nullptr);
        }
        if (m_tFile) {
          m_tFile->Close();
        }
      }
    }

    /** copy constructor needs to be implemented if needed as class has dynamic memory/resource allocation
      (as pointed out by cppcheck) */
    SimpleVariableRecorder(SimpleVariableRecorder&) = delete;
    /** assignment operator ("=") needs to be implemented if needed as class has dynamic memory/resource allocation
      (as pointed out by cppcheck)*/
    SimpleVariableRecorder& operator=(SimpleVariableRecorder&) = delete;


    /// Record varibles by filling the TTree
    void record() { m_tTree->get().Fill();}

    /// Write out TFile to root file
    void write()
    {
      if (!ProcHandler::parallelProcessingUsed() or ProcHandler::isOutputProcess()) {
        if (m_tTree) {
          // Due to some weird behaviour we  have to cd to the file of the TTree
          // before saving in order to have the tree in the correct file.
          TDirectory* tmpDirectory = gDirectory;

          TFile* tFile = m_tTree->get().GetCurrentFile();
          if (tFile) {
            // We still own the TFile.
            tFile->cd();
            m_tTree->get().Write("", TObject::kOverwrite);
            m_tTree->get().SetDirectory(nullptr);
          }
          // return to previous directory
          if (tmpDirectory) {
            tmpDirectory->cd();
          } else {
            gROOT->cd();
          }

        }
      }
    }

  protected:
    /// ROOT file to which should be written
    TFile* m_tFile;
    /// TTree that contains recorded variables
    StoreObjPtr<RootMergeable<TTree> > m_tTree;
  };
}
