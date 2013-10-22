//+
// File : CertifyParallelModule.cc
// Description : Module to ensure parallel processing
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#include <daq/rfarm/event/modules/CertifyParallelModule.h>
#include <stdlib.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CertifyParallel)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CertifyParallelModule::CertifyParallelModule() : Module()
{
  //Set module properties
  setPropertyFlags(c_ParallelProcessingCertified);

  //Parameter definition
  B2INFO("CertifyParallel: Constructor done.");
}


CertifyParallelModule::~CertifyParallelModule()
{
}

void CertifyParallelModule::initialize()
{
}


void CertifyParallelModule::beginRun()
{
  m_nevent = 0;
}


void CertifyParallelModule::event()
{
  m_nevent++;

}

void CertifyParallelModule::endRun()
{
  printf("Total event = %d\n", m_nevent);
}


void CertifyParallelModule::terminate()
{
  B2INFO("ElapsedTime: terminate called")
}

