/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <boost/python/def.hpp>

#include <framework/database/Database.h>

#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

#include <TClonesArray.h>
#include <TFile.h>
#include <TTree.h>

using namespace std;
using namespace Belle2;

Database& Database::Instance()
{
  static Database instance;
  return instance;
}

Database::Database(): m_globalTag("database.root"),  m_dbFile(0)
{
}

Database::~Database()
{
  m_dbFile->Write();
  delete m_dbFile;
}


bool Database::connectDatabase()
{
  if (!m_dbFile) {
    string fileName = FileSystem::findFile(m_globalTag);
    if (fileName.empty()) {
      B2WARNING("The database file " << m_globalTag << " was not found. Creating a new database file.");
      fileName = m_globalTag;
    }
    TDirectory* saveDir = gDirectory;
    m_dbFile = TFile::Open(fileName.c_str(), "UPDATE");
    saveDir->cd();
    if (!m_dbFile || !m_dbFile->IsOpen()) {
      B2ERROR("Could not open database file " << m_globalTag << ".");
      return false;
    }
  }

  return true;
}

pair<TObject*, IntervalOfValidity> Database::getData(const EventMetaData& event, const string& name)
{
  pair<TObject*, IntervalOfValidity> result;
  result.first = 0;

  if (!connectDatabase()) {
    B2ERROR("Failed to get " << name << " from database.");
    return result;
  }

  TTree* tree = (TTree*) m_dbFile->Get(name.c_str());
  if (!tree) {
    B2ERROR("Failed to get " << name << " from database. No tree found in database file.");
    return result;
  }

  TObject* object = 0;
  tree->SetBranchAddress("object", &object);
  IntervalOfValidity* iov = 0;
  tree->SetBranchAddress("iov", &iov);
  for (int iEntry = 0; iEntry < tree->GetEntriesFast(); iEntry++) {
    tree->GetEntry(iEntry);
    if (iov->contains(event)) {
      result.first = object;
      result.second = *iov;
      return result;
    }
  }

  B2ERROR("Failed to get " << name << " from database. No matching entry for experiment/run " << event.getExperiment() << "/" <<
          event.getRun() << " found.");
  return result;
}

void Database::getData(const EventMetaData& event, std::list<DBQuery>& query)
{
  for (auto& entry : query) {
    auto objectIov = getData(event, entry.name);
    entry.object = objectIov.first;
    entry.iov = objectIov.second;
  }
}

bool Database::storeData(const std::string& name, TObject* object, IntervalOfValidity& iov)
{
  if (!connectDatabase()) {
    B2ERROR("Failed to store " << name << " in database.");
    return false;
  }

  TTree* tree = (TTree*) m_dbFile->Get(name.c_str());
  if (!tree) {
    TDirectory* saveDir = gDirectory;
    m_dbFile->cd();
    tree = new TTree(name.c_str(), "");
    tree->Branch("object", &object);
    tree->Branch("iov", &iov);
    saveDir->cd();
  } else {
    tree->SetBranchAddress("object", &object);
    IntervalOfValidity* piov = &iov;
    tree->SetBranchAddress("iov", &piov);
  }

  tree->Fill();

  return true;
}

bool Database::storeData(std::list<DBQuery>& query)
{
  bool result = true;
  for (auto& entry : query) {
    result = result && storeData(entry.name, entry.object, entry.iov);
  }
  return result;
}


void Database::exposePythonAPI()
{
  using namespace boost::python;

  def("set_global_tag", &Database::setGlobalTag);
}
