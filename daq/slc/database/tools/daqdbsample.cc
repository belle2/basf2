#include <daq/slc/psql/PostgreSQLInterface.h>
#include <daq/slc/database/DBObjectLoader.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>

#include <iostream>

using namespace Belle2;

int main(int argc, const char** argv)
{
  if (argc < 3) {
    LogFile::debug("usage: %s <tablename> <configname>", argv[0]);
    return 1;
  }
  const std::string tablename = argv[1];
  const std::string configname = argv[2];

  // Settings for postgreSQL database
  ConfigFile config("slowcontrol");
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));

  // Loads parameters in DAQ config. table
  DBObject obj = DBObjectLoader::load(db, tablename, configname);
  std::cout << "Printing out confinguration " << configname << "@" << tablename << std::endl;
  obj.print();
  std::cout << std::endl;

  // Checks exsistence of config entry
  std::cout << "Getting values in " << configname << "@" << tablename << std::endl;
  if (!obj.hasObject("mydata")) {
    std::cerr << "No object \"mydata\" is available" << std::endl;
    return 1;
  }

  // Shows a list of parameters
  DBField::NameList names = obj.getFieldNames();
  for (DBField::NameList::iterator it = names.begin();
       it != names.end(); it++) {
    std::cout << *it << std::endl;
  }

  // Reads values in the configuration
  const DBObject cobj(obj("mydata"));
  if (cobj.getProperty("ival").getType() == DBField::INT)
    std::cout << "mydata.ival = " << cobj.getInt("ival") << std::endl;
  if (cobj.getProperty("bval").getType() == DBField::BOOL)
    std::cout << "mydata.bval = " << cobj.getBool("bval") << std::endl;
  if (cobj.getProperty("fval").getType() == DBField::FLOAT)
    std::cout << "mydata.fval = " << cobj.getFloat("fval") << std::endl;
  if (cobj.getProperty("dval").getType() == DBField::DOUBLE)
    std::cout << "mydata.dval = " << cobj.getDouble("dval") << std::endl;

  return 0;
}
