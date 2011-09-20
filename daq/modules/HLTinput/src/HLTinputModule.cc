/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <sys/wait.h>
#include <daq/modules/HLTinput/HLTinputModule.h>

#include <framework/core/ModuleManager.h>

#include <TFile.h>
#include <TTree.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(HLTInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

HLTInputModule::HLTInputModule() : Module()
{
  setDescription("HLTInput module");
  setPropertyFlags(c_Input);
  addParam("inBufferName", m_inBufferName, string("Incoming buffer name"), string("B2DataIn"));
}

HLTInputModule::~HLTInputModule()
{
}

void HLTInputModule::initialize()
{
  m_msgHandler = new MsgHandler(1);
  m_msgHandler->clear();

  B2INFO("HLTInput: ring buffer initializing...");
  //m_inBuf = new HLTBuffer((char*)m_inBufferName.c_str(), MAXPACKETSIZE);
  m_inBuf = new HLTBuffer(c_DataInPort, MAXPACKETSIZE);

  eventMetaDataPtr = StoreObjPtr<EventMetaData>("EventMetaData", DataStore::c_Event);
  //eventMetaDataPtr->setExperiment(m_expNumber);
  //eventMetaDataPtr->setRun(m_runNumber);
  //eventMetaDataPtr->setEvent(m_evtNumber);
  eventMetaDataPtr->setExperiment(1);
  eventMetaDataPtr->setRun(1);
  eventMetaDataPtr->setEvent(1);
}

void HLTInputModule::beginRun()
{
  /*
  while (1) {
    if (m_inBuf->numq() > 0) {
      B2INFO("HLTInput: Data comes in");
      break;
    }

    usleep(100);
  }
  */
}

void HLTInputModule::event()
{
  B2INFO("HLTInput module: event () starts");
  /*
  eventMetaDataPtr = StoreObjPtr<EventMetaData>("EventMetaData", DataStore::c_Event);
  //eventMetaDataPtr->setExperiment(m_expNumber);
  //eventMetaDataPtr->setRun(m_runNumber);
  //eventMetaDataPtr->setEvent(m_evtNumber);
  eventMetaDataPtr->setExperiment(1);
  eventMetaDataPtr->setRun(1);
  eventMetaDataPtr->setEvent(1);
  */

  B2INFO("NUMQ = " << m_inBuf->numq());

  int type = 1;
  while (type) {
    if (m_inBuf->numq() > 0) {
      B2INFO("Starting to process in HLTInput module");
      type = readData(DataStore::c_Event);
      //eventMetaDataPtr->setEndOfData();
      /*
      if (type == -1) {
        B2INFO("HLTInput module: EOF detected!");
        eventMetaDataPtr->setEndOfData();
        break;
      }
      */
      /*
      int size;
      char* tmp = new char[MAXPACKETSIZE];
      while ((size = m_inBuf->remq ((int*)tmp)) == 0) {
        usleep (100);
      }

      std::string input (tmp);

      B2INFO ("Message got (size = " << input.size () << ")");
      B2INFO ("Message got: " << input);

      if (input == "EOF") {
        B2INFO ("HLTInput module: EOF detected!");

        break;
      }
        */

      //delete input;
    }

//    if (type == 0) {
//      B2INFO ("HLTInput module: event done, trying to get out of the loop...");
//      break;
//    }

    usleep(100);

  }


  //eventMetaDataPtr->setEndOfData();
  B2INFO("HLTInput module: event () ends");
}

void HLTInputModule::endRun()
{
  B2INFO("HLTInput module: endRun () ends");
}

void HLTInputModule::terminate()
{
  B2INFO("HLTInput module: terminate () called");
}

int HLTInputModule::readData(const DataStore::EDurability& indurability)
{

  std::vector<TObject*> objlist;
  std::vector<std::string> namelist;

  B2INFO("Reading data from ring buffer...");
  char* tmp = new char[MAXPACKETSIZE];
  int size = (m_inBuf->remq((int*)tmp) + 1) * 4 - 1;

  while (size == 0) {
    usleep(100);
  }

  if (!strcmp(tmp, "EOF")) {
    B2INFO("EOF Detected!");
    return -1;
  }

  B2INFO("Decoding data...(" << tmp << " with size of " << size << ")");
  EvtMessage* msg = new EvtMessage(tmp);
  //int status = m_msgHandler->decode_msg(msg, objlist, namelist);
  m_msgHandler->decode_msg(msg, objlist, namelist);

  B2INFO("Reading data from ring buffer (size = " << msg->size() << ")");

  //RECORD_TYPE type = msg->type();
  msg->type();
  DataStore::EDurability durability = (DataStore::EDurability)(msg->header())->reserved[0];
  int nobjs = msg->header()->reserved[1];
  int narrays = msg->header()->reserved[2];

  char filename[255];
  sprintf(filename, "HLTInput%ld-%d.root", time(NULL), msg->header()->size);
  std::cout << filename << std::endl;
  TFile* oFile = new TFile(filename, "RECREATE", "test");
  TTree* oTree = new TTree("tree", "tree");


  //B2INFO ("Taken data: " << nobjs << " objects / " << narrays << " arrays");
  B2INFO("Taken data: " << msg->header()->reserved[1] << " objects / "
         << msg->header()->reserved[2] << " arrays"
         << " / size = " << msg->header()->size);

  delete[] tmp;

  B2INFO("Storing data into DataStore");

  for (int i = 0; i < nobjs; i++) {
    B2INFO("Storing objects...");
    //objlist[i]->Print ();
    if (!DataStore::Instance().storeObject(objlist[i], namelist[i]), durability)
      B2INFO("storing failed!");
    B2INFO("set branch = " << objlist[i]->GetName());
    oTree->Branch(objlist[i]->GetName(), &objlist[nobjs+i]);
    //DataStore::Instance().storeObject(objlist.at(i), m_objectNames[durability].at(i));
  }

  for (int i = 0; i < narrays; i++) {
    B2INFO("Storing arrays..." << durability);
    //objlist[nobjs + i]->Print ();
    if (!DataStore::Instance().storeArray((TClonesArray*)objlist[nobjs + i], namelist[nobjs + i]), durability)
      B2INFO("storing failed!");
    //DataStore::Instance().storeArray((TClonesArray*)objlist.at(i + nobjs), m_branchNames[durability].at(i));
    B2INFO("set branch = " << objlist[nobjs+i]->GetName());
    oTree->Branch(objlist[nobjs+i]->GetName(), &objlist[nobjs+i]);
  }

  B2INFO("Data taking done!");

  StoreIter* obj_iter = DataStore::Instance().getObjectIterator(static_cast<DataStore::EDurability>(0));
  StoreIter* arr_iter = DataStore::Instance().getArrayIterator(static_cast<DataStore::EDurability>(0));

  obj_iter->first();
  while (!obj_iter->isDone()) {
    B2INFO("       object key = " << obj_iter->key());
    obj_iter->next();
  }
  arr_iter->first();
  while (!arr_iter->isDone()) {
    B2INFO("       array key = " << arr_iter->key());
    arr_iter->next();
  }

  B2INFO("======= Writing ROOT file =======");
  oTree->Fill();
  oTree->Write();
  delete oFile;

  return 0;
}
