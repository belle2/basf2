#include "xml/NodeLoader.h"

#include "base/StringUtil.h"
#include "base/Debugger.h"

#include <iostream>

using namespace Belle2;

void NodeLoader::load(const std::string& entry)
{
  _entry = entry;
  std::string path = _dir + "/" + entry + ".xml";
  _file_path_v.push_back(path);
  XMLElement* root = _parser.parse(path);
  _system.setName(root->getAttribute("name"));
  _system.setRevision(atoi(root->getAttribute("revision").c_str()));
  std::vector<XMLElement*> el_v = root->getElements();
  for (size_t i = 0; i < el_v.size(); i++) {
    XMLElement* el = el_v[i];
    if (Belle2::tolower(el->getTag()) == "group") {
      NodeGroup* group = new NodeGroup();
      group->setName(el->getAttribute("name"));
      group->setCols(atoi(el->getAttribute("cols").c_str()));
      group->setRows(atoi(el->getAttribute("rows").c_str()));
      _system.addNodeGroup(group);
      std::vector<XMLElement*> elc_v = el->getElements();
      for (size_t ic = 0; ic < elc_v.size(); ic++) {
        XMLElement* elc = elc_v[ic];
        const std::string name = elc->getAttribute("name");
        const std::string tag =  Belle2::tolower(elc->getTag());
        if (tag == "node") {
          DataObject* obj = NULL;
          if (elc->hasAttribute("data_class")) {
            std::string class_name = elc->getAttribute("data_class");
            obj = _loader.load(class_name);
            if (_loader.getClassList().find(class_name) == _loader.getClassList().end()) {
              _loader.getClassList().insert(std::map<std::string, DataObject*>::value_type(class_name, obj));
            }
            push(obj);
          }
          NSMNode* node = new NSMNode(name, obj);
          //node->setVersion(_system.getRevision());
          node->setUsed(!(el->getAttribute("used") == "false"));
          node->setSynchronize((el->getAttribute("synchronize") == "true"));
          node->setIndex(_system.getNodes().size());
          node->setName(name);
          _system.addNode(node);
          group->addNode(node);
        }
      }
    }
  }
  for (std::map<std::string, std::string>::iterator it = _loader.getPathList().begin();
       it != _loader.getPathList().end(); it++) {
    std::string path = it->second;
    std::cout << path << std::endl;
    _file_path_v.push_back(path);
  }
}

void NodeLoader::push(DataObject* obj)
{
  if (obj == NULL) return;
  std::string class_name = obj->getClassName();
  if (_obj_v_m.find(class_name) == _obj_v_m.end()) {
    std::vector<DataObject*> obj_v;
    _obj_v_m.insert(std::map<std::string, std::vector<DataObject*> >::value_type(class_name, obj_v));
  }
  _obj_v_m[class_name].push_back(obj);
  for (std::map<std::string, DataObject*>::iterator it = obj->getObjects().begin();
       it != obj->getObjects().end(); it++) {
    push(it->second);
  }
}
