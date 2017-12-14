#include "daq/slc/apps/dqmviewd/DQMFileReader.h"

#include "daq/slc/apps/dqmviewd/DQMViewCallback.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include "TH2.h"
#include "TFile.h"

#include <iostream>

using namespace Belle2;

DQMFileReader::~DQMFileReader()
{
  if (m_file) m_file->Close();
  m_file = NULL;
}

TH1* DQMFileReader::getHist(const std::string& name)
{
  return m_hist_m.getHist(name);
}

bool DQMFileReader::init(int index, DQMViewCallback& callback)
{
  m_mutex.lock();
  if (m_file) m_file->Close();
  m_file = TMapFile::Create(m_hist_m.getFileName().c_str());
  m_hist_m.clear();
  TMapRec* mr = m_file->GetFirst();
  int i = 0;
  while (m_file->OrgAddress(mr)) {
    LogFile::debug("%s:%d", __FILE__, __LINE__);
    TObject* obj = m_file->Get(mr->GetName());
    if (obj != NULL) {
      LogFile::debug("%s:%d", __FILE__, __LINE__);
      TString class_name = obj->ClassName();
      if (class_name.Contains("TH1") ||  class_name.Contains("TH2")) {
        LogFile::debug("%s:%d", __FILE__, __LINE__);
        TH1* h = (TH1*)obj;
        std::string name = h->GetName();
        StringList str_v = StringUtil::split(name, '/');
        std::string dir = "";
        if (str_v.size() > 1) {
          dir = str_v[0];
          name = str_v[1];
        }
        LogFile::debug("%s/%s:(%d, %f, %f)", dir.c_str(), name.c_str(), (int)h->GetXaxis()->GetNbins(),
                       h->GetXaxis()->GetXmin(), h->GetXaxis()->GetXmax());
        std::string vname = StringUtil::form("package[%d].hist[%d].", index, i);
        callback.add(new NSMVHandlerText(vname + "name", true, false, h->GetName()));
        callback.add(new NSMVHandlerText(vname + "title", true, false, h->GetTitle()));
        callback.add(new NSMVHandlerInt(vname + "dim", true, false, h->GetDimension()));
        callback.add(new NSMVHandlerText(vname + "xaxis.title", true, false, h->GetXaxis()->GetTitle()));
        callback.add(new NSMVHandlerInt(vname + "xaxis.nbins", true, false, h->GetXaxis()->GetNbins()));
        callback.add(new NSMVHandlerInt(vname + "xaxis.min", true, false, h->GetXaxis()->GetXmin()));
        callback.add(new NSMVHandlerInt(vname + "xaxis.max", true, false, h->GetXaxis()->GetXmin()));
        callback.add(new NSMVHandlerText(vname + "yaxis.title", true, false, h->GetYaxis()->GetTitle()));
        callback.add(new NSMVHandlerInt(vname + "yaxis.nbins", true, false, h->GetYaxis()->GetNbins()));
        callback.add(new NSMVHandlerInt(vname + "yaxis.min", true, false, h->GetYaxis()->GetXmin()));
        callback.add(new NSMVHandlerInt(vname + "yaxis.max", true, false, h->GetYaxis()->GetXmin()));
        callback.add(new NSMVHandlerText(vname + "zaxis.title", true, false, h->GetZaxis()->GetTitle()));
        callback.add(new NSMVHandlerInt(vname + "zaxis.nbins", true, false, h->GetZaxis()->GetNbins()));
        callback.add(new NSMVHandlerInt(vname + "zaxis.min", true, false, h->GetZaxis()->GetXmin()));
        TH1* h1 = (TH1*)h->Clone((name + "_copy").c_str());
        h1->Reset();
        h1->Add(h);
        m_hist_m.addHist(h1, dir, name);
        m_name_v.push_back(name);
        i++;
      }
    }
    mr = mr->GetNext();
  }
  int nhists = m_hist_m.getHists().size();
  bool ready = m_ready = nhists > 0;
  if (ready) {
    std::string vname = StringUtil::form("package[%d].", index);
    callback.add(new NSMVHandlerInt(vname + "nhists", true, false, nhists));
  }
  m_mutex.unlock();
  return ready;
}

int DQMFileReader::update()
{
  m_mutex.lock();
  m_file->Update();
  bool updated = false;
  for (StringList::iterator it = m_name_v.begin();
       it != m_name_v.end(); it++) {
    LogFile::debug("%s:%d", __FILE__, __LINE__);
    std::string name = *it;
    std::string dir = m_hist_m.getDirectory(name);
    TH1* h = m_hist_m.getHist(name);
    if (dir.size() > 0) name = dir + "/" + name;
    TH1* h0 = (TH1*)m_file->Get(name.c_str());
    if (h->GetEntries() != h0->GetEntries()) {
      LogFile::debug("%s:%.0f - %.0f = %.0f", name.c_str(), h0->GetEntries()
                     , h->GetEntries(), h0->GetEntries() - h->GetEntries());
      h->Reset();
      h->Add(h0);
      updated = true;
    }
    delete h0;
  }
  if (updated) m_updateid++;
  int updateid = m_updateid;
  m_mutex.unlock();
  return updateid;
}

bool DQMFileReader::dump(const std::string& dir,
                         unsigned int expno, unsigned int runno)
{
  m_mutex.lock();
  m_file->Update();
  std::string filepath = Form("%s/DQM_%s_%04d_%06d.root",
                              dir.c_str(), m_name.c_str(), expno, runno);
  LogFile::info("created DQM dump file: %s", filepath.c_str());
  TFile* file = new TFile(filepath.c_str(), "recreate");
  for (StringList::iterator it = m_name_v.begin();
       it != m_name_v.end(); it++) {
    LogFile::debug("%s:%d", __FILE__, __LINE__);
    std::string name = *it;
    std::string dir = m_hist_m.getDirectory(name);
    if (dir.size() > 0) name = dir + "/" + name;
    TH1* h = (TH1*)m_file->Get(name.c_str());
    h->Write();
  }
  file->Close();
  delete file;
  m_file->cd();
  m_mutex.unlock();
  return true;
}

