#ifndef _Belle2_XMLParser_hh
#define _Belle2_XMLParser_hh

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "XMLElement.h"

namespace Belle2 {

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
