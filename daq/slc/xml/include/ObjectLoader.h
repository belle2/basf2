#ifndef _Belle2_ObjectLoader_h
#define _Belle2_ObjectLoader_h

#include "daq/slc/xml/XMLParser.h"

#include "daq/slc/base/DataObject.h"

#include <string>
#include <vector>
#include <map>

namespace Belle2 {

  class ObjectLoader {

  public:
    ObjectLoader(const std::string& dir = ".")
      : _dir(dir) {}
    ~ObjectLoader() throw() {}

  public:
    DataObject* load(const std::string& entry, DataObject* obj = NULL);
    std::map<std::string, DataObject*>& getClassList() { return _class_m; }
    std::map<std::string, std::string>& getPathList() { return _path_m; }

  private:
    std::string _dir;
    XMLParser _parser;
    std::map<std::string, DataObject*> _class_m;
    std::map<std::string, std::string> _path_m;

  };

}

#endif
