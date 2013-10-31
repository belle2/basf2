#include "NodeLoader.h"

#include "base/StringUtil.h"
#include "base/Debugger.h"

#include <iostream>

using namespace Belle2;

void NodeLoader::load(const std::string& entry)
{
  _entry = entry;
  XMLParser* parser = new XMLParser();
  _parser = parser;
  std::string path = _dir + "/" + entry + ".xml";
  _file_path_v.push_back(path);
  XMLElement* el_system = parser->parse(path);
  const std::string system_name = el_system->getAttribute("name");
  loadHosts(el_system);
  loadFTSWs(el_system);
  loadCOPPERNodes(el_system);
  loadNodes(el_system);
  std::vector<FEEModule*>& module_v(_system.getModules());
  for (size_t i = 0; i < module_v.size(); i++) {
    std::string type = module_v[i]->getType();
    if (type.size() == 0) continue;
    if (_system.hasModuleClass(type)) {
      _system.addModules(type, std::vector<FEEModule*>());
    }
    module_v[i]->setID(_system.getModules(type).size());
    _system.getModules(type).push_back(module_v[i]);
  }
}

void NodeLoader::loadHosts(XMLElement* el,
                           const std::string& parent_tag,
                           const std::string& tag)
{
  std::vector<Host*> host_v;
  if (el->hasAttribute(parent_tag)) {
    std::string network = el->getAttribute(parent_tag);
    std::string path = _dir + "/" + network + ".xml";
    _file_path_v.push_back(path);
    XMLElement* el_network = _parser->parse(path);
    int version = atoi(el_network->getAttribute("version").c_str());
    std::vector<XMLElement*> el_v = el_network->getElements();
    for (size_t i = 0; i < el_v.size(); i++) {
      if (el_v[i]->getTag() == tag) {
        Host* host = new Host();
        host->setVersion(version);
        host->setID(i);
        host->setName(el_v[i]->getAttribute("name"));
        host->setType(el_v[i]->getAttribute("type"));
        host->setProductID(atoi(el_v[i]->getAttribute("produt_id").c_str()));
        host->setLocation(el_v[i]->getAttribute("location"));
        _system.addHost(host);
        _host_m.insert(std::map<std::string, Host*>::value_type(host->getName(), host));
      }
    }
  }
}

void NodeLoader::loadFTSWs(XMLElement* el)
{
  std::vector<FTSW*> ftsw_v;
  if (el->hasAttribute("ftswset")) {
    std::string ftswset = el->getAttribute("ftswset");
    std::string path = _dir + "/" + ftswset + ".xml";
    _file_path_v.push_back(path);
    XMLElement* el_ftswset = _parser->parse(path);
    int version = 0;
    std::vector<XMLElement*> el_v = el_ftswset->getElements();
    int id = 0;
    for (size_t i = 0; i < el_v.size(); i++) {
      if (el_v[i]->getTag() == "ftsw") {
        FTSW* ftsw = new FTSW();
        ftsw->setVersion(version);
        ftsw->setID(id++);
        ftsw->setChannel(atoi(el_v[i]->getAttribute("channel").c_str()));
        ftsw->setProductID(atoi(el_v[i]->getAttribute("product_id").c_str()));
        ftsw->setLocation(el_v[i]->getAttribute("location"));
        ftsw->setFirmware(el_v[i]->getAttribute("firmware"));
        int mode = FTSW::TRIG_NORMAL;
        std::string mode_s = Belle2::toupper(el_v[i]->getAttribute("trigger_mode"));
        if (mode_s == "IN") mode = FTSW::TRIG_IN;
        else if (mode_s == "TLU") mode = FTSW::TRIG_TLU;
        else if (mode_s == "PULSE") mode = FTSW::TRIG_PULSE;
        else if (mode_s == "REVO") mode = FTSW::TRIG_REVO;
        else if (mode_s == "RANDOM") mode = FTSW::TRIG_RANDOM;
        else if (mode_s == "POSSION") mode = FTSW::TRIG_POSSION;
        else if (mode_s == "ONCE") mode = FTSW::TRIG_ONCE;
        else if (mode_s == "STOP") mode = FTSW::TRIG_STOP;
        ftsw->setTriggerMode(mode);
        ftsw->setTriggerMode(mode);
        ftsw->setDummyRate(atoi(el_v[i]->getAttribute("dummy_rate").c_str()));
        ftsw->setTriggerLimit(atoi(el_v[i]->getAttribute("trigger_limit").c_str()));
        _system.addFTSW(ftsw);
        _ftsw_m.insert(std::map<int, FTSW*>::value_type(ftsw->getChannel(), ftsw));
      }
    }
  }
}

void NodeLoader::loadNodes(XMLElement* el)
{
  NSMNode* node = NULL;
  const std::string name = el->getAttribute("name");
  const std::string tag = el->getTag();
  if (tag == "copper_node") {
    COPPERNode* copper = new COPPERNode();
    node = copper;
    copper->setID(_system.getCOPPERNodes().size());
    _system.addCOPPERNode(copper);
    std::string hostname = el->getAttribute("host");
    if (_host_m.find(hostname) != _host_m.end()) {
      copper->setHost(_host_m[hostname]);
    } else {
      copper->setHost(NULL);
    }
    std::string module_class = el->getAttribute("module_class");
    copper->setModuleClass(module_class);
    if (_copper_m.find(module_class) != _copper_m.end()) {
      for (size_t slot = 0; slot < 4; slot++) {
        HSLB* hslb = _copper_m[module_class]->getHSLB(slot);
        if (hslb != NULL) {
          HSLB* hslb_new = new HSLB();
          (*hslb_new) = (*hslb);
          hslb_new->setID(_system.getHSLBs().size());
          hslb_new->setVersion(_version);
          _system.addHSLB(hslb_new);
          copper->setHSLB(slot, hslb_new);
          if (hslb->getFEEModule() != NULL) {
            FEEModule* module = new FEEModule();
            (*module) = *(hslb->getFEEModule());
            module->setVersion(_version);
            hslb_new->setFEEModule(module);
            _system.addModule(module);
          }
        }
      }
    }
    copper->getSender()->setScript(el->getAttribute("script"));
    const int port = (el->hasAttribute("port")) ? atoi(el->getAttribute("port").c_str()) : -1;
    copper->getSender()->setPort(port);
    const int event_size = (el->hasAttribute("event_size")) ? atoi(el->getAttribute("event_size").c_str()) : -1;
    copper->getSender()->setEventSize(event_size);
    copper->getSender()->setID(copper->getID());
  } else if (tag == "ttd_node") {
    TTDNode* ttd = new TTDNode();
    node = ttd;
    ttd->setID(_system.getTTDNodes().size());
    _system.addTTDNode(ttd);
    std::string hostname = el->getAttribute("host");
    if (_host_m.find(hostname) != _host_m.end()) {
      ttd->setHost(_host_m[hostname]);
    } else {
      ttd->setHost(NULL);
    }
    std::vector<std::string> ftsw_v = Belle2::split(el->getAttribute("ftsw"), ',');
    for (size_t ich = 0; ich < ftsw_v.size(); ich++) {
      int channel = atoi(ftsw_v[ich].c_str());
      if (_ftsw_m.find(channel) != _ftsw_m.end()) {
        ttd->addFTSW(_ftsw_m[channel]);
      }
    }
  } else if (tag == "ro_node") {
    RONode* recv = new RONode();
    node = recv;
    recv->setID(_system.getRONodes().size());
    recv->setScript(el->getAttribute("script"));
    _system.addRONode(recv);
    std::string hostname = el->getAttribute("host");
    if (_host_m.find(hostname) != _host_m.end()) {
      recv->setHost(_host_m[hostname]);
    } else {
      recv->setHost(NULL);
    }
    recv->clearSenders();
    std::vector<std::string> hostname_v = Belle2::split(el->getAttribute("senders"), ',');
    for (size_t j = 0; j < hostname_v.size(); j++) {
      recv->addSender(hostname_v[j]);
    }
  } else if (el->getElements().size() > 0) {
    if (tag == "group") {
      NodeGroup* group = new NodeGroup();
      group->setName(name);
      group->setCols(atoi(el->getAttribute("cols").c_str()));
      group->setRows(atoi(el->getAttribute("rows").c_str()));
      _system.addNodeGroup(group);
      _group = group;
    }
    std::vector<XMLElement*> el_v = el->getElements();
    for (size_t i = 0; i < el_v.size(); i++) {
      loadNodes(el_v[i]);
    }
  }

  if (node != NULL) {
    node->setVersion(_system.getVersion());
    node->setUsed(!(el->getAttribute("used") == "false"));
    node->setSynchronize((el->getAttribute("synchronize") == "true"));
    node->setIndex(_system.getNodes().size());
    node->setName(name);
    _system.addNode(node);
    if (_group != NULL) {
      _group->addNode(node);
    }
  }
}

void NodeLoader::loadCOPPERNodes(XMLElement* el)
{
  std::map<std::string, XMLElement*> el_m;
  search(el_m, el, "module_class");
  std::map<std::string, XMLElement*> elc_m;
  for (std::map<std::string, XMLElement*>::iterator it = el_m.begin();
       it != el_m.end(); it++) {
    search(elc_m, it->second, "type");
  }
  for (std::map<std::string, XMLElement*>::iterator it = elc_m.begin();
       it != elc_m.end(); it++) {
    std::vector<XMLElement*> el_reg_v = it->second->getElements();
    FEEModule* module = new FEEModule();
    module->setType(it->first);
    module->setVersion(_version);
    for (size_t i = 0; i < el_reg_v.size(); i++) {
      XMLElement* el_reg = el_reg_v[i];
      FEEModule::Register reg;
      reg.setName(el_reg->getAttribute("name"));
      reg.setAddress(strtoul(el_reg->getAttribute("address").c_str(), 0, 0));
      reg.setLength(atoi(el_reg->getAttribute("length").c_str()));
      reg.setSize(atoi(el_reg->getAttribute("size").c_str()));
      reg.setValues(atoi(el_reg->getAttribute("value").c_str()));
      module->addRegister(reg);
    }
    _module_m.insert(std::map<std::string, FEEModule*>::value_type(it->first, module));
  }
  for (std::map<std::string, XMLElement*>::iterator it = el_m.begin();
       it != el_m.end(); it++) {
    std::vector<XMLElement*> el_v = it->second->getElements();
    COPPERNode* copper = new COPPERNode();
    for (size_t i = 0; i < el_v.size(); i++) {
      if (el_v[i]->getTag() == "hslb") {
        HSLB* hslb = new HSLB();
        hslb->setFirmware(el_v[i]->getAttribute("firmware"));
        hslb->setUsed(el_v[i]->getAttribute("used") != "false");
        hslb->setFEEModule(_module_m[el_v[i]->getAttribute("type")]);
        copper->setHSLB((int)(el_v[i]->getAttribute("slot")[0] - 'a'), hslb);
      }
    }
    _copper_m.insert(std::map<std::string, COPPERNode*>::value_type(it->first, copper));
  }
}

void NodeLoader::search(std::map<std::string, XMLElement*>& el_m, XMLElement* el,
                        const std::string& type)
{
  std::vector<XMLElement*> el_v = el->getElements();
  for (size_t i = 0; i < el_v.size(); i++) {
    if (el_v[i]->hasAttribute(type)) {
      std::string value = el_v[i]->getAttribute(type);
      if (el_m.find(value) == el_m.end()) {
        std::string path = _dir + "/" + value + ".xml";
        if (value.size() > 0) {
          el_m.insert(std::map<std::string, XMLElement*>::value_type(value,
                                                                     _parser->parse(path)));
          _file_path_v.push_back(path);
        }
      }
    } else {
      search(el_m, el_v[i], type);
    }
  }
}
