#ifndef _B2DAQ_XMLParser_hh
#define _B2DAQ_XMLParser_hh

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "XMLElement.hh"

namespace B2DAQ {

  class XMLParser {

  public:
    XMLParser();
    virtual ~XMLParser() throw() {}

  public:
    virtual XMLElement* parse(const std::string& path);

  private:
    void parseXML(XMLElement* el, xmlNode* node);
    
  };

}

#endif
