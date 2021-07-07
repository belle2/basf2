/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <daq/rfarm/event/modules/CertifyParallelModule.h>

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
  B2INFO("ElapsedTime: terminate called");
}

