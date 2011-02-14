//+
// File : genrawdata.cc
// Description : pseudo raw data generator
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 3 - Sep - 2010
//-

#include "daq/modules/genrawdata/genrawdataModule.h"

#include <framework/core/ModuleManager.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#define RAWDATASIZE 1000*4

using namespace std;
using namespace Belle2;

// Framework interface
REG_MODULE(GenRawData)

// Constructor and destructor

GenRawDataModule::GenRawDataModule() : Module()
{
  //Set module properties
  setDescription("GenRawData module");
  //setPropertyFlags(c_TriggersNewRun | c_TriggersEndOfData);
  setPropertyFlags(c_Input);
  printf("GenRawData: Constructor called \n");

  //  m_buffer = new unsigned int[RAWDATASIZE];
  m_buffer = NULL;
  m_size = 0;
  m_nevt = 0;
  //Parameter definition
  addParam("MaxEvent", m_maxevt, "Number of events to generate", 100);

  cout << "GenRawData : constructor called" << endl;
}


GenRawDataModule::~GenRawDataModule()
{
  printf("GenRawData: Destructor called \n");

}

// Module functions

void GenRawDataModule::initialize()
{
}

void GenRawDataModule::beginRun()
{
}


void GenRawDataModule::event()
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
    DataStore::Instance().storeObject(rh, "RawHeader", DataStore::c_Run);
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
    DataStore::Instance().storeObject(raw, "RawCDC", DataStore::c_Event);
    //    StoreObjPtr<RawCDC> rawcdc;
    //    rawcdc->set_buffer ( m_size, m_buffer );
    //    rawcdc.assignObject ( "RawCDC", c_Run, true );
    setProcessRecordType(prt_Event);
  }
  m_nevt++;
  if (m_nevt > m_maxevt) setProcessRecordType(prt_EndOfData);

}


void GenRawDataModule::endRun()
{

}


void GenRawDataModule::terminate()
{
  printf("GenRawData : termination called\n");

}

