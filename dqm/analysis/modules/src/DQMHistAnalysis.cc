//+
// File : DQMHistAnalysisModule.cc
// Description : DQM histgram analysis module
//
// Author : Tomoyuki Konno, Tokyo Metropolitan Univerisity
// Date : 20 - Dec - 2015
//-

#include <dqm/analysis/modules/DQMHistAnalysis.h>
#include <TROOT.h>
#include <TClass.h>



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
    if (g_hist[histname]) {
      //Want to search elsewhere if null-pointer saved in map
      return g_hist[histname];
    } else {
      B2ERROR("Histogram " << histname << " listed as being in memfile but points to nowhere.");
    }
  }
  B2INFO("Histogram " << histname << " not in memfile.");

  //Histogram not in list, search in memory for it
  gROOT->cd();

  //Following the path to the histogram
  TDirectory* d = gROOT;
  TString myl = histname;
  TString tok;
  Ssiz_t from = 0;
  while (myl.Tokenize(tok, from, "/")) {
    TString dummy;
    Ssiz_t f;
    f = from;
    if (myl.Tokenize(dummy, f, "/")) { // check if its the last one
      auto e = d->GetDirectory(tok);
      if (e) {
        B2INFO("Cd Dir " << tok);
        d = e;
      }
      d->cd();
    } else {
      break;
    }
  }

  TH1* found_hist = findHist(d, tok);
  if (found_hist) {
    g_hist[histname] = found_hist;//Can't use addHist as we want to overwrite invalid entries
  }
  return found_hist;

}

TH1* DQMHistAnalysisModule::findHist(const std::string& dirname, const std::string& histname)
{
  if (dirname.size() > 0) {
    return findHist(dirname + "/" + histname);
  }
  return findHist(histname);
}


TH1* DQMHistAnalysisModule::findHist(const TDirectory* histdir, const TString& histname)
{
  TObject* obj = histdir->FindObject(histname);
  if (obj != NULL) {
    if (obj->IsA()->InheritsFrom("TH1")) {
      B2INFO("Histogram " << histname << " found in mem");
      return (TH1*)obj;
    }
  } else {
    B2INFO("Histogram " << histname << " NOT found in mem");
  }
  return NULL;
}


void DQMHistAnalysisModule::setIntValue(const std::string& parname, int vint)
{
  if (g_parname.find(parname) == g_parname.end() && g_vint.find(parname) == g_vint.end()) {
    g_parname.insert(ParamTypeList::value_type(parname, c_ParamINT));
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
    g_parname.insert(ParamTypeList::value_type(parname, c_ParamFLOAT));
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
    g_parname.insert(ParamTypeList::value_type(parname, c_ParamTEXT));
    g_text.insert(TextList::value_type(parname, text));
  } else if (g_text.find(parname) == g_text.end()) {
    B2ERROR(parname + " is already registered as non-text data type");
  } else {
    g_text[parname] = text;
  }
}

