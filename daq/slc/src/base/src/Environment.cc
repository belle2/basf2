#include "Environment.h"

#include <cstdlib>

using namespace Belle2;

Environment Environment::__env;

std::string Environment::_XML_ENTRY = "";
std::string Environment::_XML_PATH = "";
std::string Environment::_MODULE_CLASS = "";
std::string Environment::_CLASS_PATH = "";
std::string Environment::_DB_HOST = "";
std::string Environment::_DB_NAME = "";
std::string Environment::_DB_USER = "";
std::string Environment::_DB_PASSWORD = "";

Environment::Environment() throw()
{
  std::string* env_v[] = {
    &_XML_ENTRY, &_XML_PATH, &_MODULE_CLASS, &_CLASS_PATH,
    &_DB_HOST, &_DB_NAME, &_DB_USER, &_DB_PASSWORD
  };

  const char* label[] = {
    "_XML_ENTRY", "_XML_PATH", "_MODULE_CLASS", "_CLASS_PATH",
    "_DB_HOST", "_DB_NAME", "_DB_USER", "_DB_PASSWORD"
  };
  int nlabel = sizeof(label) / sizeof(const char*);
  for (int n = 0; n < nlabel; n++) {
    char* env = getenv(label[n]);
    if (env != NULL)(*env_v[n]) = env;
  }
}
