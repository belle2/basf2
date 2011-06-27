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
#include <ctime>
#include <daq/modules/HLToutput/HLToutputModule.h>

#include <framework/core/ModuleManager.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(HLTOutput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

HLTOutputModule::HLTOutputModule() : Module()
{
  setDescription("HLTOutput module");
  //setPropertyFlags(c_TriggersNewRun | c_TriggersEndOfData | c_ReadsDataSingleProcess);
  //setPropertyFlags(c_TriggersNewRun | c_TriggersEndOfData);
  //setPropertyFlags(c_WritesDataSingleProcess | c_RequiresSingleProcess);
  setPropertyFlags(c_Input);

  addParam("outBufferName", m_outBufferName, string("Outgoing buffer name"), string("B2DataOut"));

  vector<std::string> branchNames;
  addParam("branchNames", m_branchNames[0], string("Names of branches to be written from event"), branchNames);
  addParam("branchNamesRun", m_branchNames[1], string("Names of branches to be written from run"), branchNames);
  addParam("branchNamesPersistent", m_branchNames[2], string("Names of branches to be written from persistent"), branchNames);
}

HLTOutputModule::~HLTOutputModule()
{
}

void HLTOutputModule::initialize()
{
  /*
  if (m_outBufferName.size () == 0) {
    B2ERROR ("HLT buffer is not set");
    return;
  }
  */

  m_msgHandler = new MsgHandler(1);

  // Set data
  for (int i = 0; i < DataStore::c_NDurabilityTypes; i++) {
    m_obj_iter[i] = DataStore::Instance().getObjectIterator(static_cast<DataStore::EDurability>(i));
    m_array_iter[i] = DataStore::Instance().getArrayIterator(static_cast<DataStore::EDurability>(i));
    m_done[i] = false;
  }

  m_outBuf = new HLTBuffer(c_DataOutPort, MAXPACKETSIZE);
  m_testBuf = new HLTBuffer(c_DataInPort, MAXPACKETSIZE);
}

void HLTOutputModule::beginRun()
{
}

void HLTOutputModule::event()
{
  putData(DataStore::c_Event);
  //putData("EOF");
  B2INFO("NUMQ = " << m_outBuf->numq());
}

void HLTOutputModule::endRun()
{
}

void HLTOutputModule::terminate()
{
}

void HLTOutputModule::putData(const std::string data)
{
  if (data.size() > 0)
    m_outBuf->insq((int*)(data.c_str()), data.size());
}

void HLTOutputModule::putData(const DataStore::EDurability& durability)
{
  B2INFO("HLTOutput: putData () function starts");
  B2INFO("   MsgHandler initialized.");
  m_msgHandler->clear();

  B2INFO("   Starting to iterate objects...");
  m_obj_iter[durability]->first();
  int nobj = 0;
  while (!m_obj_iter[durability]->isDone()) {
    if (!(m_branchNames[durability].size())) {
      m_msgHandler->add(m_obj_iter[durability]->value(), m_obj_iter[durability]->key());
      nobj++;
    } else {
      for (unsigned int i = 0; i < m_branchNames[durability].size(); i++) {
        if (m_branchNames[durability][i] == m_obj_iter[durability]->key()) {
          m_msgHandler->add(m_obj_iter[durability]->value(), m_obj_iter[durability]->key());
          nobj++;
        }
      }
    }
    m_obj_iter[durability]->next();
  }
  B2INFO("     " << nobj << " objects found!");

  B2INFO("   Starting to iterate arrays...");
  m_array_iter[durability]->first();
  int narray = 0;
  while (!m_array_iter[durability]->isDone()) {
    if (!(m_branchNames[durability].size())) {
      m_msgHandler->add(m_array_iter[durability]->value(), m_array_iter[durability]->key());
      narray++;
    } else {
      for (unsigned int i = 0; i < m_branchNames[durability].size(); i++) {
        if (m_branchNames[durability][i] == m_array_iter[durability]->key()) {
          m_msgHandler->add(m_array_iter[durability]->value(), m_array_iter[durability]->key());
          narray++;
        }
      }
    }
    m_array_iter[durability]->next();
  }
  B2INFO("     " << narray << " arrays found!");

  B2INFO("   Encoding EvtMessage...");
  EvtMessage* msg = m_msgHandler->encode_msg(MSG_EVENT);
  (msg->header())->reserved[0] = (int)durability;
  (msg->header())->reserved[1] = nobj;
  (msg->header())->reserved[2] = narray;

  B2INFO("   Encoding summary: " << msg->header()->reserved[1] << " objects / "
         << msg->header()->reserved[2] << " arrays");

  while (1) {
    B2INFO("Writing data into ring buffer (" << m_outBuf->shmid() << ") (size = " << msg->size() << ")");
    B2INFO("    real size = " << msg->buffer());
    //int stat = m_outBuf->insq((int*)msg->buffer(), (msg->size() - 1) / 4 + 1);
    std::string sentMsg(msg->buffer());
    int stat = m_outBuf->insq((int*)(msg->buffer()), (msg->header()->size - 1) / 4 + 1);

    //char testMessage[100000] = "Hello this is a test message and this should be replaced to realistic message in future...";
    //int stat = m_outBuf->insq((int*)(testMessage), (strlen (testMessage) - 1) / 4 + 1);

    //int stat2 = m_testBuf->insq ((int*)(sentMsg.c_str ()), (sentMsg.size () - 1) / 4 + 1);
    m_testBuf->insq((int*)(msg->buffer()), (msg->header()->size - 1) / 4 + 1);
    if (stat >= 0)
      break;
    usleep(100);
  }

  // Checking the encoded data
  std::vector<TObject*> testObjListDirect, testObjListTransfer;
  std::vector<std::string> testNameListDirect, testNameListTransfer;

  MsgHandler* msgHandlerDirect = new MsgHandler(1);
  msgHandlerDirect->clear();
  EvtMessage* testMsgDirect = new EvtMessage(msg->buffer());
  msgHandlerDirect->decode_msg(testMsgDirect, testObjListDirect, testNameListDirect);
  testMsgDirect->type();

  MsgHandler* msgHandlerTransfer = new MsgHandler(1);
  msgHandlerTransfer->clear();
  char* tmpMessage = new char[MAXPACKETSIZE];
  strcpy(tmpMessage, "8!");
  //m_testBuf->remq((int*)tmpMessage);
  EvtMessage* testMsgTransfer = new EvtMessage(tmpMessage);
  msgHandlerTransfer->decode_msg(testMsgTransfer, testObjListTransfer, testNameListTransfer);
  testMsgTransfer->type();

  B2INFO("=====Encoding/decoding test summary======");
  B2INFO("raw: Direct=" << msg->buffer() << "(" << strlen(msg->buffer()) << ")"
         << " Transfer=" << tmpMessage << "(" << strlen(tmpMessage) << ")");
  B2INFO(" Direct: " << testMsgDirect->header()->reserved[1] << " objs / "
         << testMsgDirect->header()->reserved[2] << " arrays / size = "
         << testMsgDirect->header()->size);
  B2INFO(" Transfer: " << testMsgTransfer->header()->reserved[1] << " objs / "
         << testMsgTransfer->header()->reserved[2] << " arrays / size = "
         << testMsgTransfer->header()->size);
  B2INFO("msg: Direct=" << strlen(testMsgDirect->msg()) << " Transfer=" << strlen(testMsgTransfer->msg()));
  if (!strcmp(testMsgDirect->msg(), testMsgTransfer->msg())) {
    B2INFO(" Two messages are identical");
  } else {
    B2WARNING(" Two messages are different!");
    B2WARNING("    Direct: " << testMsgDirect->msg());
    B2WARNING("    Transfer: " << testMsgTransfer->msg());
  }

  B2INFO("HLTOutput: putData () function done!");

  delete msg;
}
