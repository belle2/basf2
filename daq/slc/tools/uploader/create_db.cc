#include <iostream>
#include <sstream>
#include <cstdlib>

#include <util/StringUtil.hh>

#include <xml/XMLParser.hh>
#include <xml/NodeLoader.hh>

#include <db/DBNodeSystemConfigurator.hh>
#include <db/MySQLInterface.hh>

#include <cstdlib>

using namespace B2DAQ;

int main(int argc, char** argv)
{
  const std::string dir = getenv("B2SC_XML_PATH");
  const std::string entry = getenv("B2SC_XML_ENTRY");

  NodeLoader loader(dir);
  loader.setVersion(0);
  loader.load(entry);
  NodeSystem& node_system(loader.getSystem());

  MySQLInterface db;
  db.init();
  db.connect(getenv("B2SC_DB_HOST"),
             getenv("B2SC_DB_NAME"),
             getenv("B2SC_DB_USER"),
             getenv("B2SC_DB_PASS"),
             atoi(getenv("B2SC_DB_PORT")));
  DBNodeSystemConfigurator config(&db, &node_system);
  config.createTables();
  config.writeTables();

  std::cout << "done" << std::endl;
  db.close();
  return 0;
}
