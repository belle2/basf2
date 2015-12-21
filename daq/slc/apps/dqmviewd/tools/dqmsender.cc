#include <daq/slc/system/TCPSocket.h>
#include <daq/slc/system/TCPServerSocket.h>
#include <daq/slc/system/TCPSocketWriter.h>
#include <daq/slc/system/Time.h>
#include <daq/slc/system/File.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/Date.h>

#include <daq/slc/runcontrol/RCState.h>

#include "daq/slc/apps/dqmviewd/ZipDeflater.h"

#include <daq/slc/system/BufferedWriter.h>
#include <daq/slc/system/StreamSizeCounter.h>

#include <TH1F.h>
#include <TH2F.h>
#include <TRandom.h>

#include <cstdio>

using namespace Belle2;

void sendContents(const char* packname, int nhists,
                  TH1** hist, Writer& writer) throw(IOException);

const int FLAG_LIST = 1;
const int FLAG_CONFIG = 2;
const int FLAG_UPDATE = 3;

int main(int argc, char** argv)
{
  if (argc < 2) {
    LogFile::debug("%s : port ", argv[0]);
    return 1;
  }

  const char* dirname = "TEST";
  const char* packname = "TEST";
  int npacks = 1;
  int nhists = 3;
  TH1** hist = new TH1* [3] {
    new TH1F("h_HitXPositionCh01",
    "Hit X position (Ch-01);X [mm];Entires", 1000, -20, 20),
    new TH1F("h_HitYPositionCh01",
    "Hit Y position (Ch-01);Y [mm];Entires", 1000, -30, 30),
    new TH2F("h_HitXYPositionCh01",
    "Hit X - Y (Ch-01);X [mm];Y [mm]",
    90, -20, 20, 90, -30, 30)
  };

  TCPServerSocket serversocket("0.0.0.0", atoi(argv[1]));
  serversocket.open();
  TCPSocket socket;
  while (true) {
    try {
      socket = serversocket.accept();
      LogFile::info("connected new ");
      TCPSocketWriter writer(socket);
      writer.writeInt(FLAG_LIST);
      writer.writeInt(npacks);
      for (int i = 0; i < npacks; i++) {
        writer.writeString(packname);
      }
      writer.writeInt(0x7FFF);

      writer.writeInt(FLAG_CONFIG);
      writer.writeInt(npacks);
      for (int i = 0; i < npacks; i++) {
        writer.writeString(packname);
        writer.writeInt(nhists);
        for (int n = 0; n < nhists; n++) {
          TH1* h = hist[n];
          std::string name = h->GetName();
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
        writer.writeInt(0x7FFF);
      }
      writer.writeInt(0x7FFF);

      int buf_size = 0;
      for (int i = 0; i < npacks; i++) {
        StreamSizeCounter counter;
        sendContents(packname, nhists, hist, counter);
        if (counter.count() > buf_size) buf_size = counter.count();
      }
      ZipDeflater buf(buf_size, buf_size * 1.01 + 12);

      int count = 0;
      while (true) {
        for (Int_t i = 0; i < 10000; i++) {
          Double_t x = gRandom->Gaus(0, 10);
          Double_t y = gRandom->Gaus(0, 10);
          hist[0]->Fill(x);
          hist[1]->Fill(y);
          hist[2]->Fill(x, y);
        }

        writer.writeInt(FLAG_UPDATE);
        writer.writeInt(1);//exp number
        writer.writeInt(2);//run number
        writer.writeInt(RCState::RUNNING_S.getId());
        int ic = 0;
        buf.seekTo(0);
        writer.writeInt(ic);
        sendContents(packname, nhists, hist, buf);
        buf.deflate();
        buf.writeObject(writer);
        ic++;
        writer.writeInt(-1);
        LogFile::info("updated");

        sleep(5);
        count++;
      }
    } catch (const IOException& e) {
      LogFile::error(e.what());
    }
  }
  return 0;
}

void sendContents(const char* packname, int nhists, TH1** hist, Writer& writer)
throw(IOException)
{
  writer.writeString(packname);
  writer.writeInt(nhists);
  for (int i = 0; i < nhists; i++) {
    TH1* h = hist[i];
    std::string name = h->GetName();
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
