#ifndef _Belle2_NodeLoader_hh
#define _Belle2_NodeLoader_hh

#include "xml/XMLParser.h"
#include "xml/ObjectLoader.h"

#include "base/NodeSystem.h"

#include <string>
#include <vector>
#include <map>

namespace Belle2 {

  class NodeLoader {

  public:
    NodeLoader(const std::string& dir = ".")
      : _dir(dir), _loader(dir) {}
    ~NodeLoader() throw() {}

  public:
    void load(const std::string& entry);
    NodeSystem& getSystem() { return _system; }
    const std::string& getEntryLabel() const throw() { return _entry; }
    std::vector<std::string>& getFilePathList() { return _file_path_v; }
    std::map<std::string, std::vector<DataObject*> >& getObbjectSet() { return _obj_v_m; }

  protected:
    void push(DataObject* obj);

  private:
    std::string _dir;
    std::string _entry;
    XMLParser _parser;
    ObjectLoader _loader;
    NodeSystem _system;
    std::vector<std::string> _file_path_v;
    std::map<std::string, std::vector<DataObject*> > _obj_v_m;

  };

}

#endif
