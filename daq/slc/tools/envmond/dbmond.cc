#include <nsm/NSMNodeDaemon.h>
#include <nsm/NSMData.h>
#include <nsm/NSMCommunicator.h>

#include <database/MySQLInterface.h>

#include <xml/XMLParser.h>
#include <base/Date.h>

#include <base/StringUtil.h>
#include <base/ConfigFile.h>

#include <iostream>
#include <unistd.h>
#include <cstdlib>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 1) {
    std::cout << "Usage : ./dbmond "
              << "[<dataname:formatname:rev>...]" << std::endl;
    return 1;
  }
  ConfigFile config("slc_config");
  DBInterface* db = new MySQLInterface();
  db->init();
  db->connect(config.get("HV_DATABASE_HOST"), config.get("HV_DATABASE_NAME"),
              config.get("HV_DATABASE_USER"), config.get("HV_DATABASE_PASS"),
              config.getInt("HV_DATABASE_PORT"));

  XMLParser parser;
  XMLElement* el = parser.parse(config.get("HV_MONITOR_XML_PATH") + "/" +
                                config.get("HV_MONITOR_XML_ENTRY") + ".xml");
  std::vector<XMLElement*> el_v = el->getElements();
  std::vector<NSMData*> data_v;
  for (size_t i = 0; i < el_v.size(); i++) {
    XMLElement* elc = el_v[i];
    std::string nodename = elc->getAttribute("name");
    std::vector<XMLElement*> elc_v = elc->getElements();
    for (size_t i = 0; i < elc_v.size(); i++) {
      XMLElement* elcc = elc_v[i];
      std::string dataname = elcc->getAttribute("name");
      std::string format = elcc->getAttribute("format");
      std::string monitor_class = elcc->getAttribute("monitor_class");
      int revision = atoi(elcc->getAttribute("revision").c_str());
      data_v.push_back(new NSMData(dataname, format, revision));
    }
  }

  for (size_t i = 0; i < data_v.size(); i++) {
    NSMData* data = data_v[i];
    data->parse();
    try {
      db->execute(Belle2::form("select unix_timestamp(record_time) %sfrom %s_rev%d;",
                               data->toSQLNames().c_str(),
                               data->getName().c_str(), data->getRevision()));
      DBRecordList& ret(db->loadRecords());
      for (size_t i = 0; i < ret.size(); i++) {
        data->setSQLValues(ret[i].getFieldNames(), ret[i].getFieldValues());
        std::cout << data->getName() << ","
                  << ret[i].getFieldValueInt("record_time") << ","
                  << (int)data->getByte("status") << ","
                  << data->getInt32("voltage_mon") << ","
                  << data->getInt32("current_mon") << std::endl;
      }
    } catch (const DBHandlerException& e) {}
  }

  return 0;
}
