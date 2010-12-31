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
#include <framework/modules/HLToutput/HLToutput.h>

#include <framework/core/ModuleManager.h>
//#include <framework/dcore/RingBuffer.h>
#include <framework/dcore/EvtReceiver.h>
//#include <framework/dcore/SignalMan.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(HLTOutput, "HLTOutput")

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

HLTOutput::HLTOutput() : Module()
{
  setDescription("HLTOutput module");
  //setPropertyFlags(c_TriggersNewRun | c_TriggersEndOfData | c_ReadsDataSingleProcess);
  //setPropertyFlags(c_TriggersNewRun | c_TriggersEndOfData);
  setPropertyFlags(c_WritesDataSingleProcess | c_RequiresSingleProcess);

  addParam("dest", m_dest, string("localhost"), "Destination IP address");
  addParam("port", m_port, 20000, "Port number for the communication");

  vector<std::string> branchNames;
  addParam("branchNames", m_branchNames[0], branchNames, "Names of branches to be written from event");
  addParam("branchNamesRun", m_branchNames[1], branchNames, "Names of branches to be written from run");
  addParam("branchNamesPersistent", m_branchNames[2], branchNames, "Names of branches to be written from persistent");
}

HLTOutput::~HLTOutput()
{
}

void HLTOutput::initialize()
{
  m_msgHandler = new MsgHandler(1);

  // Set data
  for (int i = 0; i < c_NDurabilityTypes; i++) {
    m_obj_iter[i] = DataStore::Instance().getObjectIterator(static_cast<EDurability>(i));
    m_array_iter[i] = DataStore::Instance().getArrayIterator(static_cast<EDurability>(i));
    m_done[i] = false;
  }

  // Set ring buffer
  char outBufName[] = "outBuffer";

  m_outBuf = new RingBuffer(outBufName, MAXPACKETSIZE);

  // Forking EvtSender
  m_pidEvtSender = fork();

  if (m_pidEvtSender == 0) {
    m_evtSender = new EvtSender(m_dest, m_port);
    m_evtSender->init(m_outBuf);
    B2INFO("EvtSender initialized");

    while (1) {
      int broadCastCode = m_evtSender->broadCasting();
      if (broadCastCode == 2) {
        B2INFO("EvtSender: EOF detected");
        break;
      }
      usleep(100);
    }
  } else {
    m_evtSender = NULL;
  }
}

void HLTOutput::beginRun()
{
  if (m_pidEvtSender > 0)
    B2INFO("Begin a new run...");
}

void HLTOutput::event()
{
  if (m_pidEvtSender > 0) {
    /*
    struct timeval nowTime;
    gettimeofday (&nowTime, NULL);
    std::cout << "LOL = " << nowTime.tv_usec << std::endl;
    */

    //std::string dummyData (m_dummySize, 'c');
    //B2INFO ("Available string length = " << dummyData.max_size ());
    //putData (dummyData);
    putData(c_Event);
    putData("EOF");

    //B2INFO ("HLTOutput module: event () ends");
  }
}

void HLTOutput::endRun()
{
  if (m_pidEvtSender > 0) {
    //B2INFO ("HLTOutput module: endRun () called");
  }
}

void HLTOutput::terminate()
{
  if (m_pidEvtSender > 0) {
    B2INFO("HLTOutput module: terminate () called");

    int stat_val;
    pid_t child_pid;
    child_pid = wait(&stat_val);

    delete m_outBuf;
  } else
    B2INFO("EvtSender dies");
}

void HLTOutput::putData(const std::string data)
{
  if (data.size() > 0)
    m_outBuf->insq((int*)(data.c_str()), data.size());
}

void HLTOutput::putData(const EDurability& durability)
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
