//+
// File : genrawdata.cc
// Description : pseudo raw data generator
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 3 - Sep - 2010
//-

#include "daq/modules/genrawdata/genrawdata.h"

#include <framework/core/ModuleManager.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreDefs.h>
#include <framework/datastore/StoreArray.h>

#define RAWDATASIZE 1000*4

using namespace std;
using namespace Belle2;

// Framework interface
REG_MODULE(GenRawData, "GenRawData")

// Constructor and destructor

GenRawData::GenRawData() : Module()
{
  //Set module properties
  setDescription("GenRawData module");
  setPropertyFlags(c_TriggersNewRun | c_TriggersEndOfData);
  printf("GenRawData: Constructor called \n");

  //  m_buffer = new unsigned int[RAWDATASIZE];
  m_buffer = NULL;
  m_size = 0;
  m_nevt = 0;
  //Parameter definition
  addParam("MaxEvent", m_maxevt, 100, "Number of events to generate");

  cout << "GenRawData : constructor called" << endl;
}


GenRawData::~GenRawData()
{
  printf("GenRawData: Destructor called \n");

}

// Module functions

void GenRawData::initialize()
{
}

void GenRawData::beginRun()
{
}


void GenRawData::event()
{
  cout << "GenRawData : Evt = " << m_nevt << endl;
  if (m_buffer != NULL) {
    delete[] m_buffer;
    m_buffer = NULL;
  }

  if (m_nevt == 0) {
    m_size = 16 * 4;
    m_buffer = new unsigned int[m_size];
    m_buffer[0] = m_size;
    memset((char*)&m_buffer[1], 0, 15*4);
    RawHeader* rh = new RawHeader(m_size, m_buffer);
    DataStore::Instance().storeObject(rh, "RawHeader", c_Run);
    //    StoreObjPtr<RawHeader> rawheader;
    //    rawheader->set_buffer ( m_size, m_buffer );
    //    rawheader.assignObject ( "RawHeader", c_Run, true );
    setProcessRecordType(prt_BeginRun);
    cout << "BeginRun generated" << endl;
  } else {
    m_size = 4000;
    m_buffer = new unsigned int[m_size];
    m_buffer[0] = m_size;
    memset((char*)&m_buffer[1], 0, 999*4);
    RawCDC* raw = new RawCDC(m_size, m_buffer);
    DataStore::Instance().storeObject(raw, "RawCDC", c_Event);
    //    StoreObjPtr<RawCDC> rawcdc;
    //    rawcdc->set_buffer ( m_size, m_buffer );
    //    rawcdc.assignObject ( "RawCDC", c_Run, true );
    setProcessRecordType(prt_Event);
  }
  m_nevt++;
  if (m_nevt > m_maxevt) setProcessRecordType(prt_EndOfData);

}


void GenRawData::endRun()
{

}


void GenRawData::terminate()
{
  printf("GenRawData : termination called\n");

}

