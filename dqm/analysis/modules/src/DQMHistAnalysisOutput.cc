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
  B2DEBUG(20, "DQMHistAnalysisOutput: Constructor done.");
}


DQMHistAnalysisOutputModule::~DQMHistAnalysisOutputModule() { }

void DQMHistAnalysisOutputModule::initialize()
{
  ParamTypeList& parnames(getParNames());
  for (ParamTypeList::iterator i = parnames.begin(); i != parnames.end(); ++i) {
    std::string pname = i->first;
    B2DEBUG(20, "Adding : " << pname);
  }
  B2DEBUG(20, "DQMHistAnalysisOutput: initialized.");
}


void DQMHistAnalysisOutputModule::beginRun()
{
  B2DEBUG(20, "DQMHistAnalysisOutput: beginRun called.");
}

void DQMHistAnalysisOutputModule::event()
{
  ParamTypeList& parnames(getParNames());
  IntValueList& vints(getIntValues());
  FloatValueList& vfloats(getFloatValues());
  TextList& texts(getTexts());
  for (ParamTypeList::iterator i = parnames.begin(); i != parnames.end(); ++i) {
    std::string pname = i->first;
    std::string vname = StringUtil::tolower(StringUtil::replace(pname, "/", "."));
    switch (i->second) {
      case INT:
        B2DEBUG(20, vname << " " << vints[pname]);
        break;
      case FLOAT:
        B2DEBUG(20, vname << " " << vfloats[pname]);
        break;
      case TEXT:
        B2DEBUG(20, vname << " " << texts[pname]);
        break;
    }
  }
}

void DQMHistAnalysisOutputModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisOutput : endRun called");
}


void DQMHistAnalysisOutputModule::terminate()
{
  B2DEBUG(20, "terminate called");
}

