//+
// File : HLTTagger.cc
// Description : Module to create HLTTag
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#include <TSystem.h>
#include <stdlib.h>

#include "daq/rfarm/trigsoft/modules/HLTTagger.h"
#include "framework/datastore/StoreObjPtr.h"
#include "mdst/dataobjects/HLTTag.h"

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(HLTTagger)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

HLTTaggerModule::HLTTaggerModule() : Module()
{
  //Set module properties
  setDescription("Encode DataStore into RingBuffer");
  //  setPropertyFlags(c_Input | c_ParallelProcessingCertified);

  addParam("HLTUnitID", m_hltunit, "HLT unit ID", 0);
  m_nevt = 0;

  //Parameter definition
  B2INFO("HLTTagger: Constructor done.");
}


HLTTaggerModule::~HLTTaggerModule()
{
}

void HLTTaggerModule::initialize()
{
  StoreObjPtr<HLTTag>::registerPersistent();
  B2INFO("HLTTagger: initialized.");
}


void HLTTaggerModule::beginRun()
{
  B2INFO("HLTTagger: beginRun called.");
}


void HLTTaggerModule::event()
{
  StoreObjPtr<HLTTag> tag;
  tag.create();
  tag->HLTEventID(m_nevt);
  tag->HLTUnitID(m_hltunit);
  tag->HLTSetCurrentTime();
  tag->Accept(HLTTag::Global); // Event is supposed to be accepted by default
  tag->Accept(HLTTag::BeamTest);

  // For test:
  //  tag->Accept(HLTTag::Level3);
  //  if ( tag->Accepted() ) printf ( "Taken!!!!\n" );
  //  printf ( "HLTTag word = %8.8x\n", tag->GetSummaryWord() );

  m_nevt++;

  return;
}

void HLTTaggerModule::endRun()
{
  B2INFO("HLTTagger: endRun done.");
}


void HLTTaggerModule::terminate()
{
  B2INFO("HLTTagger: terminate called")
}

