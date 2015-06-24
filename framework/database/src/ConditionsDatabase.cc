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

#include <framework/database/ConditionsDatabase.h>

#include <framework/conditions/ConditionsService.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

#include <TClonesArray.h>
#include <TFile.h>
#include <TTree.h>

#include <cstdio>

using namespace std;
using namespace Belle2;

void ConditionsDatabase::createDefaultInstance()
{
  if (s_instance) {
    B2WARNING("A database instance was already created.");
  } else {
    s_instance = new ConditionsDatabase();
  }
}

void ConditionsDatabase::createInstance(const std::string& restBaseName, const std::string& fileBaseName,
                                        const std::string& fileBaseLocal)
{
  createDefaultInstance();
  ConditionsService::getInstance()->setRESTbasename(restBaseName);
  ConditionsService::getInstance()->setFILEbasename(fileBaseName);
  ConditionsService::getInstance()->setFILEbaselocal(fileBaseLocal);
}


pair<TObject*, IntervalOfValidity> ConditionsDatabase::getData(const EventMetaData& event, const string& name)
{
  pair<TObject*, IntervalOfValidity> result;
  result.first = 0;

  if ((m_currentExperiment != event.getExperiment()) || (m_currentRun != event.getRun())) {
    m_currentExperiment = event.getExperiment();
    m_currentRun = event.getRun();
    ConditionsService::getInstance()->getPayloads(m_globalTag, std::to_string(m_currentExperiment), std::to_string(m_currentRun));
  }

  if (!ConditionsService::getInstance()->payloadExists("topcaf" + name)) {
    B2ERROR("No payload " << name << " found in the database.");
    return result;
  }

  std::string filename = ConditionsService::getInstance()->getPayloadFileURL("topcaf", name);
  if (filename.empty()) {
    B2ERROR("Failed to get " << name << " from database.");
    return result;
  }

  TDirectory* saveDir = gDirectory;
  TFile* file = TFile::Open(filename.c_str());
  saveDir->cd();
  if (!file || !file->IsOpen()) {
    B2ERROR("Could not open payload file " << file << ".");
    delete file;
    return result;
  }

  result.first = file->Get(name.c_str());
  delete file;
  if (!result.first) {
    B2ERROR("Failed to get " << name << " from database. Object not found in payload file.");
    return result;
  }

  conditionsPayload paylodInfo = ConditionsService::getInstance()->getPayloadInfo("topcaf", name);
  result.second = IntervalOfValidity(stoi(paylodInfo.expInitial), stoi(paylodInfo.runInitial), stoi(paylodInfo.expFinal),
                                     stoi(paylodInfo.runFinal));
  return result;
}

bool ConditionsDatabase::storeData(const std::string& name, TObject* object, IntervalOfValidity& iov)
{
  TDirectory* saveDir = gDirectory;
  TFile* file = TFile::Open("payload.root", "RECREATE");

  object->Write(name.c_str(), TObject::kSingleKey);
  file->WriteObject(&iov, "IoV");

  file->Close();
  delete file;
  saveDir->cd();

  ConditionsService::getInstance()->writePayloadFile("payload.root", "topcaf", name);
  remove("payload.root");

  return true;
}
