#include <daq/slc/pyb2daq/DAQDBObject.h>

#include <daq/slc/psql/PostgreSQLInterface.h>
#include <daq/slc/database/DBObjectLoader.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

using namespace Belle2;
using namespace boost::python;

DAQDBObject::DAQDBObject(const char* tablename, const char* configname)
{
  ConfigFile config("slowcontrol");
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));
  bool showall = true;
  *this = DBObjectLoader::load(db, tablename, configname, showall);
  db.close();
}

void DAQDBObject::create(const char* tablename)
{
  ConfigFile config("slowcontrol");
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));
  DBObjectLoader::createDB(db, tablename, m_obj);
  db.close();
}

boost::python::list DAQDBObject::getDBList(const char* tablename,
                                           const char* prefix)
{
  ConfigFile config("slowcontrol");
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));
  StringList names = DBObjectLoader::getDBlist(db, tablename, prefix);
  db.close();
  boost::python::list py_list;

  for (StringList::iterator it = names.begin();
       it != names.end(); it++) {
    py_list.append(*it);
  }
  return py_list;
}

boost::python::list DAQDBObject::getFieldNames() const throw()
{
  DBField::NameList names = m_obj.getFieldNames();
  boost::python::list py_list;

  for (DBField::NameList::iterator it = names.begin();
       it != names.end(); it++) {
    py_list.append(*it);
  }
  return py_list;
}

boost::python::list DAQDBObject::getNameList(bool showall) const throw()
{
  StringList names = m_obj.getNameList(showall);
  boost::python::list py_list;

  for (StringList::iterator it = names.begin();
       it != names.end(); it++) {
    py_list.append(*it);
  }
  return py_list;
}
