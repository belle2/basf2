//+
// File : DQMHistAnalysisOutput.cc
// Description :
//
// Author : Tomoyuki Konno, Tokyo Metropolitan Univerisity
// Date : 25 - Dec - 2015
//-


#include <dqm/analysis/modules/DQMHistAnalysisOutput.h>

#include <daq/slc/base/StringUtil.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisOutput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisOutputModule::DQMHistAnalysisOutputModule() : DQMHistAnalysisModule()
{
  //Parameter definition
  B2DEBUG(1, "DQMHistAnalysisOutput: Constructor done.");
}


DQMHistAnalysisOutputModule::~DQMHistAnalysisOutputModule() { }

void DQMHistAnalysisOutputModule::initialize()
{
  ParamTypeList& parnames(getParNames());
  for (ParamTypeList::iterator i = parnames.begin(); i != parnames.end(); i++) {
    std::string pname = i->first;
    B2INFO("Addding : " << pname);
  }
  B2INFO("DQMHistAnalysisOutput: initialized.");
}


void DQMHistAnalysisOutputModule::beginRun()
{
  B2INFO("DQMHistAnalysisOutput: beginRun called.");
}

void DQMHistAnalysisOutputModule::event()
{
  ParamTypeList& parnames(getParNames());
  IntValueList& vints(getIntValues());
  FloatValueList& vfloats(getFloatValues());
  TextList& texts(getTexts());
  for (ParamTypeList::iterator i = parnames.begin(); i != parnames.end(); i++) {
    std::string pname = i->first;
    std::string vname = StringUtil::tolower(StringUtil::replace(pname, "/", "."));
    switch (i->second) {
      case INT:
        B2INFO(vname << " " << vints[pname]);
        break;
      case FLOAT:
        B2INFO(vname << " " << vfloats[pname]);
        break;
      case TEXT:
        B2INFO(vname << " " << texts[pname]);
        break;
    }
  }
}

void DQMHistAnalysisOutputModule::endRun()
{
  B2INFO("DQMHistAnalysisOutput : endRun called");
}


void DQMHistAnalysisOutputModule::terminate()
{
  B2INFO("terminate called");
}

