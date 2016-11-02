/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/mva/Recorder.h>
#include <framework/utilities/FileSystem.h>

#include <framework/pcore/ProcHandler.h>
#include <TROOT.h>

using namespace Belle2;
using namespace TrackFindingCDC;

Recorder::Recorder(const std::function<void(TTree&)>& setBranches,
                   const std::string& rootFileName,
                   const std::string& treeName) :
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


Recorder::Recorder(const std::vector<Named<Float_t* > >& namedVariables,
                   const std::string& rootFileName,
                   const std::string& treeName) :
  Recorder([ & namedVariables](TTree & tree)
{
  for (const Named<Float_t*>& namedVariable : namedVariables) {
    std::string name = namedVariable.getName();
    Float_t* variable = namedVariable;
    tree.Branch(name.c_str(), variable);
  }
},
rootFileName,
treeName)
{
}

Recorder::~Recorder()
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

void Recorder::write()
{
  if (!ProcHandler::parallelProcessingUsed() or ProcHandler::isOutputProcess()) {
    if (m_tTree) {
      // Due to some weird behaviour we  have to cd to the file of the TTree
      // before saving in order to have the tree in the correct file.
      TDirectory* ptrSavedCurrentTDirectory = gDirectory;

      TFile* tFile = m_tTree->get().GetCurrentFile();
      if (tFile) {
        // We still own the TFile.
        tFile->cd();
        m_tTree->get().Write("", TObject::kOverwrite);
        m_tTree->get().SetDirectory(nullptr);
      }

      if (ptrSavedCurrentTDirectory) {
        ptrSavedCurrentTDirectory->cd();
      } else {
        gROOT->cd();
      }

    }
  }
}

void Recorder::capture()
{
  m_tTree->get().Fill();
}
