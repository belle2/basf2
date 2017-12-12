//+
// File : DQMHistAnalysisModule.cc
// Description : DQM histgram analysis module
//
// Author : Tomoyuki Konno, Tokyo Metropolitan Univerisity
// Date : 20 - Dec - 2015
//-

#include <dqm/analysis/modules/DQMHistAnalysis.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysis)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisModule::ParamTypeList DQMHistAnalysisModule::g_parname;
DQMHistAnalysisModule::IntValueList DQMHistAnalysisModule::g_vint;
DQMHistAnalysisModule::FloatValueList DQMHistAnalysisModule::g_vfloat;
DQMHistAnalysisModule::TextList DQMHistAnalysisModule::g_text;
DQMHistAnalysisModule::HistList DQMHistAnalysisModule::g_hist;

DQMHistAnalysisModule::DQMHistAnalysisModule() : Module()
{
  //Set module properties
  setDescription("Histgram Analysis module");
}


DQMHistAnalysisModule::~DQMHistAnalysisModule()
{

}

void DQMHistAnalysisModule::addHist(const std::string& dirname, const std::string& histname, TH1* h)
{
  if (dirname.size() > 0) {
    g_hist.insert(HistList::value_type(dirname + "/" + histname, h));
  } else {
    g_hist.insert(HistList::value_type(histname, h));
  }
}

const DQMHistAnalysisModule::HistList& DQMHistAnalysisModule::getHistList()
{
  return g_hist;
}

TH1* DQMHistAnalysisModule::findHist(const std::string& histname)
{
  if (g_hist.find(histname) != g_hist.end()) {
    return g_hist[histname];
  }
  return NULL;
}

TH1* DQMHistAnalysisModule::findHist(const std::string& dirname, const std::string& histname)
{
  if (dirname.size() > 0) {
    return findHist(dirname + "/" + histname);
  }
  return findHist(histname);
}

void DQMHistAnalysisModule::setIntValue(const std::string& parname, int vint)
{
  if (g_parname.find(parname) == g_parname.end() && g_vint.find(parname) == g_vint.end()) {
    g_parname.insert(ParamTypeList::value_type(parname, INT));
    g_vint.insert(IntValueList::value_type(parname, vint));
  } else if (g_vint.find(parname) == g_vint.end()) {
    B2ERROR(parname + " is already registered as non-int data type");
  } else {
    g_vint[parname] = vint;
  }
}

void DQMHistAnalysisModule::setFloatValue(const std::string& parname, float vfloat)
{
  if (g_parname.find(parname) == g_parname.end() && g_vfloat.find(parname) == g_vfloat.end()) {
    g_parname.insert(ParamTypeList::value_type(parname, FLOAT));
    g_vfloat.insert(FloatValueList::value_type(parname, vfloat));
  } else if (g_vfloat.find(parname) == g_vfloat.end()) {
    B2ERROR(parname + " is already registered as non-float data type");
  } else {
    g_vfloat[parname] = vfloat;
  }
}

void DQMHistAnalysisModule::setText(const std::string& parname, const std::string& text)
{
  if (g_parname.find(parname) == g_parname.end() && g_text.find(parname) == g_text.end()) {
    g_parname.insert(ParamTypeList::value_type(parname, TEXT));
    g_text.insert(TextList::value_type(parname, text));
  } else if (g_text.find(parname) == g_text.end()) {
    B2ERROR(parname + " is already registered as non-text data type");
  } else {
    g_text[parname] = text;
  }
}

