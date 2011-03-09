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
    B2ERROR ("Ring buffer is not set");
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

  m_outBuf = new RingBuffer((char*)m_outBufferName.c_str(), MAXPACKETSIZE);
}

void HLTOutputModule::beginRun()
{
}

void HLTOutputModule::event()
{
  putData(DataStore::c_Event);
  //putData("EOF");
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
      for (int i = 0; i < m_branchNames[durability].size(); i++) {
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
      for (int i = 0; i < m_branchNames[durability].size(); i++) {
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
         << msg->header()->reserved[2] << "arrays");

  while (1) {
    B2INFO("Writing data into ring buffer (size = " << msg->size() << ")");
    int stat = m_outBuf->insq((int*)msg->buffer(), (msg->size() - 1) / 4 + 1);
    if (stat >= 0)
      break;
    usleep(100);
  }

  B2INFO("HLTOutput: putData () function done!");

  delete msg;
}
