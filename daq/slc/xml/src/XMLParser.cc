#include "daq/slc/xml/XMLParser.h"

using namespace Belle2;

XMLParser::XMLParser()
{
  //  LIBXML_TEST_VERSION;
}

XMLElement* XMLParser::parse(const std::string& path)
{
  xmlDoc* doc = xmlReadFile(path.c_str(), NULL, 0);
  XMLElement* el = new XMLElement();
  parseXML(el, xmlDocGetRootElement(doc));
  return el;
}

void XMLParser::parseXML(XMLElement* el, xmlNode* node)
{
  for (xmlNode* cur_node = node; cur_node; cur_node = cur_node->next) {
    if (cur_node->type == XML_ELEMENT_NODE) {
      XMLElement* el_tmp = el;
      if (cur_node != node) {
        el_tmp = new XMLElement();
        el->addElement(el_tmp);
      }
      el_tmp->setTag((const char*)cur_node->name);
      for (xmlAttr* cur_attr = cur_node->properties; cur_attr; cur_attr = cur_attr->next) {
        xmlChar* value = xmlNodeListGetString(cur_node->doc, cur_attr->children, 1);
        el_tmp->addAttribute((const char*)cur_attr->name, (const char*)value);
        xmlFree(value);
      }
      parseXML(el_tmp, cur_node->children);
    }
  }
}
