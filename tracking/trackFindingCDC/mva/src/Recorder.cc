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

/** Impl Declaration **/
#include <framework/datastore/StoreObjPtr.h>
#include <framework/pcore/RootMergeable.h>

#include <functional>
#include <string>

class TTree;
class TFile;

namespace Belle2 {
  namespace TrackFindingCDC {

    class Recorder::Impl {

    public:
      /**
       *  Constructor creating the TFile and TTree as well as
       *  setting up the branches with the given function.
       */
      Impl(const std::function<void(TTree&)>& setBranches,
           const std::string& rootFileName,
           const std::string& treeName = "recorded_tree");

      /// Destructor finalising the tree.
      ~Impl();

      /// Write all captured variables to disk.
      void write();

      /// Capture the registered variable values and write them out.
      void capture();

    private:
      /// Reference to the open TFile.
      TFile* m_tFile;

      /// Reference to the TTree.
      StoreObjPtr<RootMergeable<TTree> > m_tTree;
    };
  }
}

/** Impl Definition **/
#include <framework/datastore/DataStore.h>
#include <framework/pcore/ProcHandler.h>
#include <framework/utilities/FileSystem.h>

#include <TFile.h>
#include <TTree.h>

using namespace Belle2;
using namespace TrackFindingCDC;

Recorder::Impl::Impl(const std::function<void(TTree&)>& setBranches,
                     const std::string& rootFileName,
                     const std::string& treeName)
  : m_tFile(nullptr)
  , m_tTree(treeName, DataStore::c_Persistent)
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

Recorder::Impl::~Impl()
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

void Recorder::Impl::write()
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

void Recorder::Impl::capture()
{
  m_tTree->get().Fill();
}

/** PImpl Interface **/

Recorder::Recorder(const std::function<void(TTree&)>& setBranches,
                   const std::string& rootFileName,
                   const std::string& treeName)
  : m_impl(std::make_unique<Impl>(setBranches, rootFileName, treeName))
{
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

Recorder::~Recorder() = default;

void Recorder::write()
{
  m_impl->write();
}

void Recorder::capture()
{
  m_impl->capture();
}
