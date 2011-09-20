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
#include <daq/modules/HLToutput2/HLToutput2Module.h>

#include <framework/core/ModuleManager.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(HLTOutput2)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

HLTOutput2Module::HLTOutput2Module() : Module()
{
  setDescription("HLTOutput2 module");
  setPropertyFlags(c_Input);

  addParam("outBufferName", m_outBufferName, string("Outgoing buffer name"), string("B2DataOut"));

  vector<std::string> branchNames;
  addParam("branchNames", m_branchNames[0], string("Names of branches to be written from event"), branchNames);
  addParam("branchNamesRun", m_branchNames[1], string("Names of branches to be written from run"), branchNames);
  addParam("branchNamesPersistent", m_branchNames[2], string("Names of branches to be written from persistent"), branchNames);
}

HLTOutput2Module::~HLTOutput2Module()
{
}

void HLTOutput2Module::initialize()
{
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

void HLTOutput2Module::beginRun()
{
}

void HLTOutput2Module::event()
{
  putData(DataStore::c_Event);
  //putData("EOF");
  B2INFO("NUMQ = " << m_outBuf->numq());
}

void HLTOutput2Module::endRun()
{
}

void HLTOutput2Module::terminate()
{
  B2INFO("HLTOutput2 Module terminates...");
  //putData("EOF");
}

void HLTOutput2Module::putData(const std::string data)
{
  if (data.size() > 0)
    m_outBuf->insq((int*)(data.c_str()), data.size() / 4 + 1);
  //m_outBuf->insq((int*)(data.c_str()), data.size());
}

void HLTOutput2Module::putData(const DataStore::EDurability& durability)
{
  B2INFO("HLTOutput2: putData () function starts");
  m_msgHandler->clear();

  m_obj_iter[durability]->first();
  int nobj = 0;

  while (!m_obj_iter[durability]->isDone()) {
    m_msgHandler->add(m_obj_iter[durability]->value(), m_obj_iter[durability]->key());
    nobj++;
    m_obj_iter[durability]->next();
  }

  m_array_iter[durability]->first();
  int narray = 0;
  while (!m_array_iter[durability]->isDone()) {
    TClonesArray* arr = (TClonesArray*)m_array_iter[durability]->value();
    m_msgHandler->add(m_array_iter[durability]->value(), m_array_iter[durability]->key());
    narray++;
    m_array_iter[durability]->next();
  }
  /*
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
  */

  B2INFO("   Encoding EvtMessage...");
  EvtMessage* msg = m_msgHandler->encode_msg(MSG_EVENT);
  (msg->header())->reserved[0] = (int)durability;
  (msg->header())->reserved[1] = nobj;
  (msg->header())->reserved[2] = narray;

  B2INFO("======================================================");
  B2INFO("   Encoding summary: " << msg->header()->reserved[1] << " objects / "
         << msg->header()->reserved[2] << " arrays");
  B2INFO("======================================================");

  m_outBuf->insq((int*)(msg->buffer()), (msg->header()->size - 1) / 4 + 1);

  B2INFO("HLTOutput2: putData () function done!");

  delete msg;
}
