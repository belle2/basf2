#ifndef _Belle2_Environment_hh
#define _Belle2_Environment_hh

#include <string>

namespace Belle2 {

  class Environment {

  private:
    static Environment __env;

  private:
    Environment() throw();

  public:
    ~Environment() throw() {};

  public:
    static const std::string& getXMLEntry() { return _XML_ENTRY; }
    static const std::string& getXMLPath() { return _XML_PATH; }
    static const std::string& getModuleClass() { return _MODULE_CLASS; }
    static const std::string& getClassPath() { return _CLASS_PATH; }
    static const std::string& getDBHost() { return _DB_HOST; }
    static const std::string& getDBName() { return _DB_NAME; }
    static const std::string& getDBUser() { return _DB_USER; }
    static const std::string& getDBPassword() { return _DB_PASSWORD; }

    static void setXMLEntry(const std::string& v) { _XML_ENTRY = v; }
    static void setXMLPath(const std::string& v) { _XML_PATH = v; }
    static void setModuleClass(const std::string& v) { _MODULE_CLASS = v; }
    static void setClassPath(const std::string& v) { _CLASS_PATH = v; }
    static void setDBHost(const std::string& v) { _DB_HOST = v; }
    static void setDBName(const std::string& v) { _DB_NAME = v; }
    static void setDBUser(const std::string& v) { _DB_USER = v; }
    static void setDBPassword(const std::string& v) { _DB_PASSWORD = v; }

  private:
    static std::string _XML_ENTRY;
    static std::string _XML_PATH;
    static std::string _MODULE_CLASS;
    static std::string _CLASS_PATH;
    static std::string _DB_HOST;
    static std::string _DB_NAME;
    static std::string _DB_USER;
    static std::string _DB_PASSWORD;

  };

}

#endif
