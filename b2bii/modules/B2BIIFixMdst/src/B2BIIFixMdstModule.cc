#include <b2bii/modules/B2BIIFixMdst/B2BIIFixMdstModule.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(B2BIIFixMdst)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

B2BIIFixMdstModule::B2BIIFixMdstModule() : Module()
{
  //Set module properties
  setDescription("Fixes Belle mdst objects (Panther tables and records).");
  setPropertyFlags(c_Input);

  B2DEBUG(1, "B2BIIFixMdst: Constructor done.");
}


B2BIIFixMdstModule::~B2BIIFixMdstModule()
{
}

void B2BIIFixMdstModule::initialize()
{
  B2INFO("B2BIIFixMdst: initialized.");
}

void B2BIIFixMdstModule::beginRun()
{
  B2INFO("B2BIIFixMdst: beginRun called.");
}


void B2BIIFixMdstModule::event()
{
}

void B2BIIFixMdstModule::endRun()
{
  B2INFO("B2BIIFixMdst: endRun done.");
}


void B2BIIFixMdstModule::terminate()
{
  B2INFO("B2BIIFixMdst: terminate called")
}

