#include <daq/slc/database/PostgreSQLInterface.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/PThread.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Date.h>

#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <sstream>

#include <TFile.h>
#include <TTree.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 5) {
    LogFile::debug("Usage : %s hvnode root_filename start_date end_date (date=\"YYYY-MM-dd hh:mm:ss\")", argv[0]);
    return 1;
  }
  ConfigFile config("slowcontrol", "hv");
  const char* nodename = argv[1];
  const char* filename = argv[2];
  const char* start_date = argv[3];
  const char* end_date = argv[4];
  DBInterface* db = new PostgreSQLInterface(config.get("database.host"),
                                            config.get("database.dbname"),
                                            config.get("database.user"),
                                            config.get("database.password"),
                                            config.getInt("database.port"));
  db->connect();
  db->execute("select l.record_time, extract(epoch from l.record_time) "
              "as recor_time_unix, c.crate, c.slot, c.channel, c.turnon, "
              "s.* from loggernames() as l, \"loggerinfo:hv_status.channel_status:1\" "
              "as s, \"configinfo:hvconfig.channel:1\" as c where l.\"table\"= "
              "'hv_status.channel_status' and l.node = '%s' and l.record_time > '%s' and "
              "l.record_time < '%s' and l.id = s.loggerid and c.index = s.index "
              "order by l.record_time desc, c.index", nodename, start_date, end_date);
  DBRecordList records = db->loadRecords();
  db->close();
  TFile* file = new TFile(filename, "recreate");
  TTree* tree = new TTree("hv_status", "HV Channel Status");
  Int_t record_time;
  Int_t index, crate, slot, channel, state;
  Bool_t turnon;
  Float_t voltage_mon, current_mon;
  tree->Branch("record_time", &record_time, "record_time/I");
  tree->Branch("index", &index, "index/I");
  tree->Branch("crate", &crate, "crate/I");
  tree->Branch("slot", &slot, "slot/I");
  tree->Branch("channel", &channel, "channel/I");
  tree->Branch("turnon", &turnon, "turnon/B");
  tree->Branch("state", &state, "state/I");
  tree->Branch("voltage_mon", &voltage_mon, "voltage_mon/F");
  tree->Branch("current_mon", &current_mon, "current_mon/F");
  for (size_t i = 0; i < records.size(); i++) {
    DBRecord& record(records[i]);
    record_time = record.getInt("recor_time_unix");
    index = record.getInt("index");
    crate = record.getInt("crate");
    slot = record.getInt("slot");
    channel = record.getInt("channel");
    turnon = record.getBool("turnon");
    state = record.getInt("state");
    voltage_mon = record.getInt("voltage_mon");
    current_mon = record.getInt("current_mon");
    tree->Fill();
  }
  tree->Write();
  file->Close();
  return 0;
}

