#include "daq/slc/apps/dqmviewd/HistSender.h"

#include "daq/slc/apps/dqmviewd/DQMViewCallback.h"
#include "daq/slc/apps/dqmviewd/ZipDeflater.h"

#include <daq/slc/system/BufferedWriter.h>
#include <daq/slc/system/StreamSizeCounter.h>
#include <daq/slc/system/TCPSocketWriter.h>
#include <daq/slc/system/TCPSocketReader.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <cstdlib>
#include <vector>

using namespace Belle2;

const int HistSender::FLAG_LIST = 1;
const int HistSender::FLAG_CONFIG = 2;
const int HistSender::FLAG_UPDATE = 3;

HistSender::~HistSender()
{
  // m_socket.close();
  // LogFile::debug("GUI disconnected.");
  m_configured = false;
}

bool HistSender::update(std::vector<TH1*>& hist, bool newconf)
{
  try {
    TCPSocketWriter writer(m_socket);
    int count = 0;
    if (newconf) m_configured = false;
    if (!m_configured) {
      writer.writeInt(FLAG_CONFIG);
      writer.writeInt(hist.size());
      for (size_t n = 0; n < hist.size(); n++) {
        TH1* h = hist[n];
        std::string name = h->GetName();
        LogFile::info(name);
        StringList str_v = StringUtil::split(name, '/');
        //LogFile::info(name);
        std::string dirname = "";
        if (str_v.size() > 1) {
          dirname = str_v[0];
          name = str_v[1];
        }
        //LogFile::info(dirname);
        TString class_name = h->ClassName();
        writer.writeString(class_name.Data());
        writer.writeString(dirname);
        writer.writeString(name);
        writer.writeString(std::string(h->GetTitle()) + ";" +
                           h->GetXaxis()->GetTitle()  + ";" +
                           h->GetYaxis()->GetTitle());
        if (h->GetXaxis()->GetLabels() != NULL) {
          writer.writeInt(h->GetXaxis()->GetNbins());
          for (int i = 0; i < h->GetXaxis()->GetNbins(); i++) {
            writer.writeString(h->GetXaxis()->GetBinLabel(i + 1));
          }
        } else {
          writer.writeInt(0);
        }
        writer.writeInt(h->GetXaxis()->GetNbins());
        writer.writeDouble(h->GetXaxis()->GetXmin());
        writer.writeDouble(h->GetXaxis()->GetXmax());
        if (class_name.Contains("TH1")) {
        } else if (class_name.Contains("TH2")) {
          if (h->GetYaxis()->GetLabels() != NULL) {
            writer.writeInt(h->GetYaxis()->GetNbins());
            for (int i = 0; i < h->GetYaxis()->GetNbins(); i++) {
              writer.writeString(h->GetYaxis()->GetBinLabel(i + 1));
            }
          } else {
            writer.writeInt(0);
          }
          writer.writeInt(h->GetYaxis()->GetNbins());
          writer.writeDouble(h->GetYaxis()->GetXmin());
          writer.writeDouble(h->GetYaxis()->GetXmax());
        }
        writer.writeInt(0x7FFF);
      }
      m_configured = true;
    }
    writer.writeInt(FLAG_UPDATE);
    writer.writeInt(hist.size());
    for (size_t i = 0; i < hist.size(); i++) {
      TH1* h = hist[i];
      std::string name = h->GetName();
      StringList str_v = StringUtil::split(name, '/');
      std::string dirname = "";
      if (str_v.size() > 1) {
        dirname = str_v[0];
        name = str_v[1];
      }
      TString class_name = h->ClassName();
      writer.writeString(name);
      writer.writeString(dirname);
      const int nbinsx = h->GetXaxis()->GetNbins();
      if (class_name.Contains("TH1")) {
        for (int nx = 0; nx < nbinsx; nx++) {
          writer.writeFloat(h->GetBinContent(nx + 1));
        }
      } else if (class_name.Contains("TH2")) {
        const int nbinsy = h->GetYaxis()->GetNbins();
        for (int ny = 0; ny < nbinsy; ny++) {
          for (int nx = 0; nx < nbinsx; nx++) {
            double d = h->GetBinContent(nx + 1, ny + 1);
            writer.writeFloat(d);
          }
        }
      }
      writer.writeInt(0x7FFF);
    }
    writer.writeInt(0x7FFF);
    count++;
  } catch (const IOException& e) {
    LogFile::warning("hist sender : %s", e.what());
    m_socket.close();
    return false;
  }
  return true;
}

