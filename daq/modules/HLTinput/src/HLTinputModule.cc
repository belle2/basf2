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
  //setPropertyFlags(c_TriggersNewRun | c_TriggersEndOfData | c_ReadsDataSingleProcess);
  //setPropertyFlags(DataStore::c_TriggersNewRun | DataStore::c_TriggersEndOfData);
  setPropertyFlags(c_Input);
  //setPropertyFlags(c_WritesDataSingleProcess | c_RequiresSingleProcess);
  //addParam("port", m_port, 20000, string ("Port number for the communication"));
}

HLTInputModule::~HLTInputModule()
{
}

void HLTInputModule::initialize()
{
  m_msgHandler = new MsgHandler(1);

  //char inBufName[] = "inBuffer";
  //int inPort = m_port;

  //m_inBuf = new RingBuffer(inBufName, MAXPACKETSIZE);
  m_inBuf = new RingBuffer("B2DataIn", MAXPACKETSIZE);

  m_pidEvtReceiver = fork();

  if (m_pidEvtReceiver == 0) {
    //m_evtReceiver = new EvtReceiver(inPort);
    m_evtReceiver = new EvtReceiver(c_DataInPort);
    m_evtReceiver->init(m_inBuf);
    B2INFO("EvtReceiver initialized");

    m_evtReceiver->listen();
  } else {
    m_evtReceiver = NULL;
  }
}

void HLTInputModule::beginRun()
{
}

void HLTInputModule::event()
{
  if (m_pidEvtReceiver > 0) {
    while (1) {
      if (m_inBuf->numq() > 0) {
        B2INFO("Starting to process in HLTInput module");
        int type = readData(DataStore::c_Event);
        if (type == -1) {
          B2INFO("HLTInput module: EOF detected!");
          break;
        }
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

      usleep(100);
    }

    //B2INFO ("HLTInput module: event () ends");
  } else {
    EvtReceiver new_sock;
    m_evtReceiver->accept(new_sock);
    std::string data;

    while (1) {
      new_sock >> data;
      if (data.size() > 0) {
        m_inBuf->insq((int*)(data.c_str()), data.size());
        if (data == "EOF") {
          B2INFO("EvtReceiver: EOF detected!");
          break;
        }
      }
    }
  }
}

void HLTInputModule::endRun()
{
  if (m_pidEvtReceiver > 0) {
    //B2INFO ("HLTInput module: endRun () called");
  }
}

void HLTInputModule::terminate()
{
  if (m_pidEvtReceiver > 0) {
    B2INFO("HLTInput module: terminate () called");

    int stat_val;
    pid_t child_pid;
    child_pid = wait(&stat_val);

    delete m_inBuf;
  } else
    B2INFO("EvtReceiver dies");
}

int HLTInputModule::readData(const DataStore::EDurability& indurability)
{
  int size;

  B2INFO("Reading data from ring buffer...");
  char* tmp = new char[MAXPACKETSIZE];
  while ((size = m_inBuf->remq((int*)tmp)) == 0) {
    usleep(100);
  }

  B2INFO("EOF Detected!");
  if (tmp == "EOF")
    return -1;

  std::vector<TObject*> objlist;
  std::vector<std::string> namelist;

  B2INFO("Decoding data...");
  EvtMessage* msg = new EvtMessage(tmp);
  int status = m_msgHandler->decode_msg(msg, objlist, namelist);

  B2INFO("Reading data from ring buffer (size = " << msg->size() << ")");

  RECORD_TYPE type = msg->type();
  DataStore::EDurability durability = (DataStore::EDurability)(msg->header())->reserved[0];
  int nobjs = msg->header()->reserved[1];
  int narrays = msg->header()->reserved[2];

  //B2INFO ("Taken data: " << nobjs << " objects / " << narrays << " arrays");
  B2INFO("Taken data: " << msg->header()->reserved[1] << " objects / "
         << msg->header()->reserved[2] << " arrays"
         << " / size = " << msg->header()->size);

  delete[] tmp;

  B2INFO("Storing data into DataStore");
  for (int i = 0; i < nobjs; i++) {
    DataStore::Instance().storeObject(objlist.at(i), m_objnames[durability].at(i));
  }

  for (int i = 0; i < narrays; i++) {
    DataStore::Instance().storeArray((TClonesArray*)objlist.at(i + nobjs),
                                     m_arraynames[durability].at(i));
  }

  return 0;
}
