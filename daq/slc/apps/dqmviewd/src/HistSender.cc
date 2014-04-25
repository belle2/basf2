#include "daq/slc/apps/dqmviewd/HistSender.h"
#include "daq/slc/apps/dqmviewd/DQMViewMaster.h"

#include <daq/slc/system/ZipDeflater.h>
#include <daq/slc/system/BufferedWriter.h>
#include <daq/slc/system/StreamSizeCounter.h>
#include <daq/slc/system/TCPSocketWriter.h>
#include <daq/slc/system/TCPSocketReader.h>
#include <daq/slc/system/LogFile.h>

#include <cstdlib>
#include <vector>

using namespace Belle2;

const int HistSender::FLAG_LIST = 1;
const int HistSender::FLAG_CONFIG = 2;
const int HistSender::FLAG_UPDATE = 3;

HistSender::~HistSender()
{
  m_socket.close();
  LogFile::debug("GUI disconnected.");
}

void HistSender::run()
{
  TCPSocketWriter socket_writer(m_socket);
  TCPSocketReader socket_reader(m_socket);

  while (true) {
    std::vector<DQMFileReader>& reader_v(m_master.getReaders());
    size_t nready = 0;
    m_master.lock();
    for (size_t i = 0; i < reader_v.size(); i++) {
      if (reader_v[i].isReady()) nready++;
    }
    try {
      socket_writer.writeInt(FLAG_LIST);
      socket_writer.writeInt((int)nready);
      for (size_t i = 0; i < reader_v.size(); i++) {
        if (reader_v[i].isReady())
          socket_writer.writeString(reader_v[i].getName());
      }
      socket_writer.writeInt(0x7FFF);
    } catch (const IOException& e) {
      m_master.unlock();
      return;
    }
    m_master.unlock();

    std::vector<bool> monitored_v;
    int npacks = 0;
    std::map<int, int> id_m;
    try {
      for (size_t i = 0; i < reader_v.size(); i++) {
        DQMFileReader& reader(reader_v[i]);
        m_master.lock();
        bool ready = reader.isReady();
        m_master.unlock();
        if (ready) {
          bool monitored = (bool)socket_reader.readChar();
          monitored_v.push_back(monitored);
          id_m.insert(std::map<int, int>::value_type(i, npacks));
          if (monitored) npacks++;
        } else {
          monitored_v.push_back(false);
        }
      }
    } catch (const IOException& e) {
      return;
    }
    if (npacks == 0) {
      LogFile::debug("HistoSender: No package selected.");
      m_socket.close();
      return;
    }
    try {
      socket_writer.writeInt(FLAG_CONFIG);
      socket_writer.writeInt(npacks);
      m_master.lock();
      for (size_t i = 0; i < reader_v.size(); i++) {
        if (monitored_v[i]) {
          DQMFileReader& reader(reader_v[i]);
          socket_writer.writeString(reader.getName());
          socket_writer.writeInt(reader.getHists().size());
          for (TH1Map::iterator it = reader.getHists().begin();
               it != reader.getHists().end(); it++) {
            TH1* h = (TH1*)it->second;
            std::string name = it->first;
            TString class_name = h->ClassName();
            socket_writer.writeString(class_name.Data());
            socket_writer.writeString(name);
            socket_writer.writeString(std::string(h->GetTitle()) + ";" +
                                      h->GetXaxis()->GetTitle()  + ";" +
                                      h->GetYaxis()->GetTitle());
            socket_writer.writeInt(h->GetXaxis()->GetNbins());
            socket_writer.writeDouble(h->GetXaxis()->GetXmin());
            socket_writer.writeDouble(h->GetXaxis()->GetXmax());
            if (class_name.Contains("TH1")) {
            } else if (class_name.Contains("TH2")) {
              socket_writer.writeInt(h->GetYaxis()->GetNbins());
              socket_writer.writeDouble(h->GetYaxis()->GetXmin());
              socket_writer.writeDouble(h->GetYaxis()->GetXmax());
            }
            socket_writer.writeInt(0x7FFF);
          }
          socket_writer.writeInt(0x7FFF);
        }
      }
      socket_writer.writeInt(0x7FFF);
      m_master.unlock();
    } catch (const IOException& e) {
      m_master.unlock();
      m_socket.close();
      return;
    }

    std::vector<int> updateid_v;
    int buf_size = 0;
    m_master.lock();
    for (size_t i = 0; i < reader_v.size(); i++) {
      updateid_v.push_back(reader_v[i].getUpdateId());
      if (monitored_v[i]) {
        StreamSizeCounter counter;
        sendContents(reader_v[i], counter);
        if (counter.count() > buf_size) buf_size = counter.count();
      }
    }
    m_master.unlock();
    ZipDeflater buf(buf_size, buf_size * 1.01 + 12);

    try {
      while (true) {
        //bool revised = false;
        m_master.lock();
        socket_writer.writeInt(FLAG_UPDATE);
        socket_writer.writeInt(m_master.getExpNumber());
        socket_writer.writeInt(m_master.getRunNumber());
        socket_writer.writeInt(m_master.getState().getId());
        int ic = 0;
        for (size_t i = 0; i < reader_v.size(); i++) {
          if (monitored_v[i]) {
            if (reader_v[i].getUpdateId() != updateid_v[i]) {
              updateid_v[i] = reader_v[i].getUpdateId();
              buf.seekTo(0);
              socket_writer.writeInt(ic);
              sendContents(reader_v[i], buf);
              buf.deflate();
              buf.writeObject(socket_writer);
            }
            ic++;
          }
        }
        socket_writer.writeInt(-1);
        m_master.wait();
        m_master.unlock();
      }
    } catch (const IOException& e) {
      m_master.unlock();
      m_socket.close();
      return;
    }
  }
}

void HistSender::sendContents(DQMFileReader& reader, Writer& writer)
throw(IOException)
{
  //LogFile::debug("Sending package : %s", reader.getName().c_str());
  writer.writeString(reader.getName());
  writer.writeInt(reader.getHists().size());
  for (TH1Map::iterator it = reader.getHists().begin();
       it != reader.getHists().end(); it++) {
    std::string name = it->first;
    TH1* h = (TH1*)it->second;
    TString class_name = h->ClassName();
    writer.writeString(name);
    const int nbinsx = h->GetXaxis()->GetNbins();
    if (class_name.Contains("TH1")) {
      for (int nx = 0; nx < nbinsx; nx++) {
        writer.writeFloat(h->GetBinContent(nx + 1));
      }
    } else if (class_name.Contains("TH2")) {
      const int nbinsy = h->GetYaxis()->GetNbins();
      for (int ny = 0; ny < nbinsy; ny++) {
        for (int nx = 0; nx < nbinsx; nx++) {
          writer.writeFloat(h->GetBinContent(nx + 1, ny + 1));
        }
      }
    }
    writer.writeInt(0x7FFF);
  }
  writer.writeInt(0x7FFF);
}
