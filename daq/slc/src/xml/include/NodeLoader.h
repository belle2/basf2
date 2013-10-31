#ifndef _Belle2_NodeLoader_hh
#define _Belle2_NodeLoader_hh

#include "XMLParser.h"

#include "base/NodeSystem.h"

#include <string>
#include <vector>
#include <map>

namespace Belle2 {

  class NodeLoader {

  public:
    NodeLoader(const std::string& dir = ".")
      : _dir(dir), _parser(NULL) { _group = NULL; }
    ~NodeLoader() throw() {}

  public:
    void load(const std::string& entry);
    void loadHosts(XMLElement* el,
                   const std::string& parent_tag = "hosts",
                   const std::string& tag = "host");
    void loadFTSWs(XMLElement* el);
    void loadNodes(XMLElement* el);
    void loadCOPPERNodes(XMLElement* el);
    void setVersion(int version) { _version = version; }
    int getVersion() { return _version; }
    NodeSystem& getSystem() { return _system; }
    std::map<std::string, COPPERNode*>& getCOPPERNodeList() { return _copper_m; }
    std::map<std::string, FEEModule*>& getModuleList() { return _module_m; }
    std::map<std::string, Host*>& getHostList() { return _host_m; }
    std::map<std::string, DataSender*>& getSenderList() { return _sender_m; }
    std::vector<std::string>& getFilePathList() { return _file_path_v; }
    const std::string& getEntryLabel() { return _entry; }

  protected:
    void search(std::map<std::string, XMLElement*>& el_m, XMLElement* el,
                const std::string& type);

  private:
    int _version;
    std::string _dir;
    std::string _entry;
    XMLParser* _parser;
    NodeSystem _system;
    NodeGroup* _group;
    std::vector<std::string> _copper_name_v;
    std::map<std::string, COPPERNode*> _copper_m;
    std::map<std::string, FEEModule*> _module_m;
    std::map<int, FTSW*> _ftsw_m;
    std::map<std::string, Host*> _host_m;
    std::map<std::string, DataSender*> _sender_m;
    std::vector<std::string> _file_path_v;

  };

}

#endif
