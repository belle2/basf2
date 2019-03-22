#include <analysis/modules/ChargedParticleIdentificator/ChargedPidMVAModule.h>

using namespace Belle2;

REG_MODULE(ChargedPidMVA)

ChargedPidMVAModule::ChargedPidMVAModule() : Module()
{
  setDescription("This is an example module. Please make sure to provide a meaningful description of what your module actually does. Make it as long as you need to.");

  //add module parameters here [see one of the following tutorials]
}


ChargedPidMVAModule::~ChargedPidMVAModule()
{
}


void ChargedPidMVAModule::initialize()
{
}

void ChargedPidMVAModule::beginRun()
{
}

void ChargedPidMVAModule::event()
{
}

void ChargedPidMVAModule::endRun()
{
}

void ChargedPidMVAModule::terminate()
{
}
