/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <daq/rawdata/modules/DeSerializerPXD.h>


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DeSerializerPXD)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DeSerializerPXDModule::DeSerializerPXDModule() : Module()
{
  //Set module properties
  setDescription("Receives PXD-Data from ONSEN (or a simulator) and stores it as RawPXD in Data Store");
  //setPropertyFlags(c_Input | c_ParallelProcessingCertified);

  addParam("Ports", m_ports, "default port number");
  addParam("Hosts", m_hosts, "default host names");
  m_nEvents = 0;
  m_compressionLevel = 0;
  m_buffer = new int[MAXEVTSIZE];
  //   events_processed = 0;
  //Parameter definition
  B2DEBUG(0, "DeSerializerPXDModule: Constructor done.");
}


DeSerializerPXDModule::~DeSerializerPXDModule()
{
  delete[] m_buffer;
}

void DeSerializerPXDModule::initialize()
{
  if (m_hosts.size() != m_ports.size()) {
    B2ERROR("DeSerializerPXDModule: Parameter error. Hosts and Ports need the same number of entries!");
    DeSerializerPXDModule::terminate();
    return;
  }

  // Open receiver sockets
  for (int i = 0; i < (int)(m_hosts.size()); i++) {
    m_recvs.push_back(new EvtSocketSend(m_hosts[i], m_ports[i]));
  }

  // Initialize EvtMetaData
  m_eventMetaDataPtr.registerInDataStore();

  // Initialize Array of RawCOPPER
  rawpxdary.registerInDataStore();

  B2DEBUG(0, "DeSerializerPXDModule: initialized.");
}

void DeSerializerPXDModule::beginRun()
{
  B2DEBUG(0, "beginRun called.");
}

void DeSerializerPXDModule::event()
{

  // Get a record from socket
  int stat = 0;
  for (auto&    it : m_recvs) {
    do {
      stat = it->recv_pxd_buffer((char*)m_buffer);
      if (stat <= 0) {
        B2INFO("DeserializerPXD Socket failed: stat = " << stat);
        DeSerializerPXDModule::endRun();
        DeSerializerPXDModule::terminate();
        return;
      };
    } while (stat == 0);

    // Put RawPXD in DataStore, stat=lenght_in_Bytes
    rawpxdary.appendNew(m_buffer, stat);

    // What we do NOT check here (yet) is, if all Packets belong to the same event! TODO
  }


  // Create EventMetaData - warning, this is only o.k. if this module is the only one!
  m_eventMetaDataPtr.create();
  m_eventMetaDataPtr->setExperiment(1);
  m_eventMetaDataPtr->setRun(1);
  m_eventMetaDataPtr->setEvent(m_nEvents);

  m_nEvents++;

  return;
}

void DeSerializerPXDModule::endRun()
{
  //fill Run data

  B2DEBUG(0, "endRun done.");
}


void DeSerializerPXDModule::terminate()
{
  B2INFO("terminate called");
}

