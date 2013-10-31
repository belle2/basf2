#include <iostream>
#include <sstream>
#include <cstdlib>

#include <base/StringUtil.h>

#include <xml/XMLParser.h>
#include <xml/NodeLoader.h>

#include <database/MySQLInterface.h>

#include <cstdlib>

using namespace Belle2;

int main(int argc, char** argv)
{
  const std::string dir = getenv("B2SC_XML_PATH");
  const std::string entry = getenv("B2SC_XML_ENTRY");

  NodeLoader loader(dir);
  loader.setVersion(0);
  loader.load(entry);

  NodeSystem& nodesystem(loader.getSystem());
  std::map<std::string, std::vector<FEEModule*> >& module_v_m(nodesystem.getModuleLists());

  Belle2::MySQLInterface db;
  db.init();
  db.connect(getenv("B2SC_DB_HOST"),
             getenv("B2SC_DB_NAME"),
             getenv("B2SC_DB_USER"),
             getenv("B2SC_DB_PASS"),
             atoi(getenv("B2SC_DB_PORT")));

  std::stringstream ss;
  ss << "drop table run_config;";
  std::cout << ss.str() << std::endl;
  try {
    db.execute(ss.str());
  } catch (const std::exception&) {}
  ss.str("");
  ss << "drop table run_status;";
  std::cout << ss.str() << std::endl;
  try {
    db.execute(ss.str());
  } catch (const std::exception&) {}
  ss.str("");
  ss << "drop table host_conf;";
  std::cout << ss.str() << std::endl;
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
  std::cout << ss.str() << std::endl;
  try {
    db.execute(ss.str()); ss.str("");
  } catch (const std::exception&) {}
  ss.str("");
  ss << "drop table ftsw_conf;";
  std::cout << ss.str() << std::endl;
  try {
    db.execute(ss.str()); ss.str("");
  } catch (const std::exception&) {}
  ss.str("");
  ss << "drop table ro_node_conf;";
  std::cout << ss.str() << std::endl;
  try {
    db.execute(ss.str()); ss.str("");
  } catch (const std::exception&) {}
  ss.str("");
  ss << "drop table hslb_conf;";
  std::cout << ss.str() << std::endl;
  try {
    db.execute(ss.str()); ss.str("");
  } catch (const std::exception&) {}
  for (std::map<std::string, std::vector<FEEModule*> >::iterator it = module_v_m.begin();
       it != module_v_m.end(); it++) {
    ss << "drop table " << it->first << "_conf;";
    std::cout << ss.str() << std::endl;
    try {
      db.execute(ss.str()); ss.str("");
    } catch (const std::exception&) {}
    ss.str("");
  }
  ss << "drop table version_control;";
  std::cout << ss.str() << std::endl;
  try {
    db.execute(ss.str()); ss.str("");
  } catch (const std::exception&) {}
  ss.str("");
  db.close();
  std::cout << "done" << std::endl;
  return 0;
}
