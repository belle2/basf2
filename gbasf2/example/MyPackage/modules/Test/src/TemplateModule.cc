#include "../include/TemplateModule.h"
#include <framework/core/ModuleManager.h>

using namespace std;
using namespace Belle2;

REG_MODULE(Template)

TemplateModule::TemplateModule(bool selfReg) : Module()
{
  B2INFO(selfReg);
  setDescription("Add a description of your module here");
}

TemplateModule::~TemplateModule()
{
}

void TemplateModule::initialize()
{
}

void TemplateModule::beginRun()
{
}

void TemplateModule::event()
{
  B2INFO("Template: Processing event");
}

void TemplateModule::endRun()
{
}

void TemplateModule::terminate()
{
}
