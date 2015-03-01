/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/modules/opentfile/OpenTFileModule.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/DataStore.h>
#include <TFile.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(OpenTFile)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
OpenTFileModule::OpenTFileModule() :
  Module(),
  m_param_fileName("out.root"),
  m_param_storeName(""),
  m_param_option("RECREATE")
{
  //Set module properties
  setDescription("Opens a .root file and makes it available to other modules through the DataStore as a StoreObjPtr.");

  //Parameter definition
  addParam("fileName", m_param_fileName, "Name of the file to be opened."); // Enforced parameter
  addParam("storeName", m_param_storeName, "Name of the TFile StoreObjPtr on the DataStore.", std::string(""));
  addParam("option", m_param_option, "Option string to forward to the TFile constructor.", std::string("RECREATE"));
}


OpenTFileModule::~OpenTFileModule() { }

void OpenTFileModule::initialize()
{
  StoreObjPtr<TFile> storedTFile(m_param_storeName, DataStore::c_Persistent);
  storedTFile.registerInDataStore(m_param_storeName, DataStore::c_DontWriteOut | DataStore::c_ErrorIfAlreadyRegistered);

  storedTFile.construct(m_param_fileName.c_str(), m_param_option.c_str());
}

void OpenTFileModule::terminate()
{
  StoreObjPtr<TFile> storedTFile(m_param_storeName, DataStore::c_Persistent);
  if (storedTFile) {
    storedTFile->Close();
    bool replace = true;
    storedTFile.assign(nullptr, replace);
  }
}
