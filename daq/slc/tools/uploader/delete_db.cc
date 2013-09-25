#include <iostream>
#include <sstream>
#include <cstdlib>

#include <util/StringUtil.hh>

#include <xml/XMLParser.hh>
#include <xml/NodeLoader.hh>

#include <db/MySQLInterface.hh>

#include <cstdlib>

using namespace B2DAQ;

int main(int argc, char** argv)
{
  if (argc < 3) {
    std::cerr << "Usage : ./gui "
              << "<directory path to XML files> "
              << "<entry XML file name> "
              << std::endl;
    return 1;
  }

  const std::string dir = argv[1];
  const std::string entry = argv[2];

  NodeLoader loader(dir);
  loader.setVersion(0);
  loader.load(entry);

  NodeSystem& nodesystem(loader.getSystem());
  std::map<std::string, std::vector<FEEModule*> >& module_v_m(nodesystem.getModuleLists());

  B2DAQ::MySQLInterface db;
  db.init();
  db.connect(getenv("B2SC_DB_HOST"),
             getenv("B2SC_DB_NAME"),
             getenv("B2SC_DB_USER"),
             getenv("B2SC_DB_PASS"),
             atoi(getenv("B2SC_DB_PORT")));

  std::stringstream ss;
  ss << "drop table host_conf;";
  try {
    db.execute(ss.str());
  } catch (const std::exception&) {}
  ss.str("");
  ss << "drop table copper_node_conf;";
  try {
    db.execute(ss.str()); ss.str("");
  } catch (const std::exception&) {}
  ss.str("");
  ss << "drop table ttd_node_conf;";
  try {
    db.execute(ss.str()); ss.str("");
  } catch (const std::exception&) {}
  ss.str("");
  ss << "drop table ftsw_conf;";
  try {
    db.execute(ss.str()); ss.str("");
  } catch (const std::exception&) {}
  ss.str("");
  ss << "drop table reciever_node_conf;";
  try {
    db.execute(ss.str()); ss.str("");
  } catch (const std::exception&) {}
  ss.str("");
  ss << "drop table hslb_conf;";
  try {
    db.execute(ss.str()); ss.str("");
  } catch (const std::exception&) {}
  for (std::map<std::string, std::vector<FEEModule*> >::iterator it = module_v_m.begin();
       it != module_v_m.end(); it++) {
    ss << "drop table " << it->first << "_conf;";
    try {
      db.execute(ss.str()); ss.str("");
    } catch (const std::exception&) {}
    ss.str("");
  }
  ss << "drop table version_control;";
  try {
    db.execute(ss.str()); ss.str("");
  } catch (const std::exception&) {}
  ss.str("");
  db.close();
  return 0;
}
