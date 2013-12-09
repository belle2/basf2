#include "daq/slc/xml/ObjectLoader.h"

#include "daq/slc/base/StringUtil.h"
#include "daq/slc/base/Debugger.h"

#include <iostream>
#include <cstdlib>

using namespace Belle2;

DataObject* ObjectLoader::load(const std::string& entry, DataObject* obj)
{
  std::string path = _dir + "/" + entry + ".xml";
  _path_m.insert(std::map<std::string, std::string>::value_type(entry, path));
  XMLElement* el = _parser.parse(path);
  const std::string data_class = el->getAttribute("class");
  const std::string base_class = el->getAttribute("extends");
  const int revision = atoi(el->getAttribute("revision").c_str());
  if (obj == NULL) {
    obj = new DataObject(data_class, base_class);
    //obj->setName(el->getAttribute("name"));
    obj->setRevision(revision);
  }
  if (base_class.size() > 0) {
    load(base_class, obj);
  }
  std::vector<XMLElement*> el_v = el->getElements();
  for (size_t i = 0; i < el_v.size(); i++) {
    XMLElement* elc = el_v[i];
    const std::string name = elc->getAttribute("name");
    if (name.size() == 0) continue;
    if (elc->getTag() == "enum") {
      const std::string value = elc->getAttribute("value");
      std::map<std::string, int> enum_m;
      std::vector<std::string> options_v = Belle2::split(elc->getAttribute("options"), ',');
      for (size_t j = 0; j < options_v.size(); j++) {
        std::vector<std::string> str_v = Belle2::split(options_v[j], ':');
        enum_m.insert(std::map<std::string, int>::value_type(str_v[0], atoi(str_v[1].c_str())));
      }
      obj->addEnum(name, enum_m, value);
    } else if (elc->getTag() != "object") {
      DataObject::ParamInfo info = { DataObject::TEXT, 0, NULL };
      if (elc->getTag().find("array") != std::string::npos) {
        info.length = atoi(elc->getAttribute("length").c_str());
      }
      const std::string value = elc->getAttribute("value");
      if (elc->getTag().find("bool") != std::string::npos) {
        info.type = DataObject::BOOL;
      } else if (elc->getTag().find("ulong") != std::string::npos) {
        info.type = DataObject::ULONG;
      } else if (elc->getTag().find("uint") != std::string::npos) {
        info.type = DataObject::UINT;
      } else if (elc->getTag().find("ushort") != std::string::npos) {
        info.type = DataObject::USHORT;
      } else if (elc->getTag().find("uchar") != std::string::npos) {
        info.type = DataObject::UCHAR;
      } else if (elc->getTag().find("long") != std::string::npos) {
        info.type = DataObject::LONG;
      } else if (elc->getTag().find("int") != std::string::npos) {
        info.type = DataObject::INT;
      } else if (elc->getTag().find("short") != std::string::npos) {
        info.type = DataObject::SHORT;
      } else if (elc->getTag().find("char") != std::string::npos) {
        info.type = DataObject::CHAR;
      } else if (elc->getTag().find("double") != std::string::npos) {
        info.type = DataObject::DOUBLE;
      } else if (elc->getTag().find("float") != std::string::npos) {
        info.type = DataObject::FLOAT;
      } else if (elc->getTag().find("text") != std::string::npos) {
        info.type = DataObject::TEXT;
        if (info.length == 0) info.length = 64;
      }
      obj->add(name, NULL, info.type, info.length);
      obj->setValue(name, value, info.length);
    } else {
      std::string class_name = elc->getAttribute("class");
      DataObject* objc = NULL;
      if (_class_m.find(class_name) == _class_m.end()) {
        objc = load(class_name);
        _class_m.insert(std::map<std::string, DataObject*>::value_type(class_name, objc));
      } else {
        objc = new DataObject(_class_m[class_name]);
      }
      for (DataObject::ParamInfoMap::iterator it = objc->getParams().begin();
           it != objc->getParams().end(); it++) {
        std::string pname = it->first;
        if (elc->hasAttribute(pname)) {
          objc->setValue(pname, elc->getAttribute(pname));
        }
      }
      obj->addObject(name, objc);
    }
  }
  return obj;
}

