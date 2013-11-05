#include "xml/ObjectLoader.h"

#include "base/StringUtil.h"
#include "base/Debugger.h"

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
    obj->setRevision(revision);
  }
  if (base_class.size() > 0) {
    load(base_class, obj);
  }
  std::vector<XMLElement*> el_v = el->getElements();
  for (size_t i = 0; i < el_v.size(); i++) {
    XMLElement* elc = el_v[i];
    const std::string pname = elc->getAttribute("name");
    const std::string priority_s = Belle2::toupper(elc->getAttribute("priority"));
    DataObject::ParamPriority priority = DataObject::LOAD;
    if (priority_s == "BOOT") priority = DataObject::BOOT;
    else if (priority_s == "LOAD") priority = DataObject::LOAD;
    else if (priority_s == "TRIGFT") priority = DataObject::TRIGFT;
    if (pname.size() == 0) continue;
    if (elc->getTag() == "enum") {
      const std::string value = elc->getAttribute("value");
      std::map<std::string, int> enum_m;
      std::vector<std::string> options_v = Belle2::split(elc->getAttribute("options"), ',');
      for (size_t j = 0; j < options_v.size(); j++) {
        std::vector<std::string> str_v = Belle2::split(options_v[j], ':');
        enum_m.insert(std::map<std::string, int>::value_type(str_v[0], atoi(str_v[1].c_str())));
      }
      obj->addEnum(pname, enum_m, value, priority);
    } else if (elc->getTag() == "int") {
      const int value = atoi(elc->getAttribute("value").c_str());
      obj->addInt(pname, value, priority);
    } else if (elc->getTag() == "int_array") {
      const size_t length = atoi(elc->getAttribute("length").c_str());
      std::vector<std::string> str_v = Belle2::split(elc->getAttribute("value"), ',');
      for (size_t j = 0; j < length; j++) {
        const int value = atoi(((str_v.size() == length) ? str_v[j] : str_v[0]).c_str());
        obj->addInt(Belle2::form(pname + "_%d", j), value, priority);
      }
    } else if (elc->getTag() == "uint") {
      const int value = strtoul(elc->getAttribute("value").c_str(), 0, 0);
      obj->addInt(pname, value, priority);
    } else if (elc->getTag() == "uint_array") {
      const size_t length = atoi(elc->getAttribute("length").c_str());
      std::vector<std::string> str_v = Belle2::split(elc->getAttribute("value"), ',');
      for (size_t j = 0; j < length; j++) {
        const int value = strtoul(((str_v.size() == length) ? str_v[j] : str_v[0]).c_str(), 0, 0);
        obj->addInt(Belle2::form(pname + "_%d", j), value, priority);
      }
    } else if (elc->getTag() == "bool") {
      const bool value = elc->getAttribute("value") == "true";
      obj->addBoolean(pname, value, priority);
    } else if (elc->getTag() == "bool_array") {
      const size_t length = atoi(elc->getAttribute("length").c_str());
      std::vector<std::string> str_v = Belle2::split(elc->getAttribute("value"), ',');
      for (size_t j = 0; j < length; j++) {
        const bool value = ((str_v.size() == length) ? str_v[j] : str_v[0]) == "true";
        obj->addBoolean(Belle2::form(pname + "_%d", j), value, priority);
      }
    } else if (elc->getTag() == "text") {
      obj->addText(pname, elc->getAttribute("value"), priority);
    } else if (elc->getTag() == "object") {
      std::string class_name = elc->getAttribute("class");
      DataObject* objc = NULL;
      if (_class_m.find(class_name) == _class_m.end()) {
        objc = load(class_name);
        _class_m.insert(std::map<std::string, DataObject*>::value_type(class_name, objc));
      } else {
        objc = new DataObject(_class_m[class_name]);
      }
      for (size_t j = 0; j < objc->getParamNames().size(); j++) {
        std::string ppname = objc->getParamName(j);
        if (elc->hasAttribute(ppname)) {
          std::string value = elc->getAttribute(ppname);
          switch (objc->getParamType(j)) {
            case DataObject::BOOLEAN:
              objc->setBooleanValue(ppname, value == "true");
              break;
            case DataObject::INT:
              objc->setIntValue(ppname, atoi(value.c_str()));
              break;
            case DataObject::TEXT:
              objc->setTextValue(ppname, value);
              break;
            default:
              break;
          }
        }
      }
      obj->addObject(pname, objc, priority);
    }
  }
  return obj;
}

