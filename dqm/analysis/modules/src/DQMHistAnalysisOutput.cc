/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
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
  const IntValueList& vints(getIntValues());
  const FloatValueList& vfloats(getFloatValues());
  const TextList& texts(getTexts());
  for (ParamTypeList::iterator i = parnames.begin(); i != parnames.end(); ++i) {
    std::string pname = i->first;
    std::string vname = StringUtil::tolower(StringUtil::replace(pname, "/", "."));
    switch (i->second) {
      case c_ParamINT:
        B2DEBUG(20, vname << " " << vints.at(pname));
        break;
      case c_ParamFLOAT:
        B2DEBUG(20, vname << " " << vfloats.at(pname));
        break;
      case c_ParamTEXT:
        B2DEBUG(20, vname << " " << texts.at(pname));
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

