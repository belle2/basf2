#include "framework/xmldb/leaf_cdata.h"
#include "framework/xmldb/node.h"
#include "framework/xmldb/tree.h"

#include <cassert>

#include <libxml/xpath.h>

#if !defined(LIBXML_XPATH_ENABLED)
#error libXML compiled without XPath support.
#endif

namespace Belle2 {
  namespace xmldb {

    Tree::Tree() : doc_(nullptr), configNs_(nullptr)
    {
      id_.id = -1;
      LIBXML_TEST_VERSION
    } // Tree constructor

    Tree::~Tree()
    {
      for (const auto & i : node_data_map) {
        delete i.second;
      }
      if (doc_) {
        ::xmlFreeDoc(doc_);
        ::xmlCleanupParser();
      } // if
    } // Tree destructor

    void Tree::attachNodeData(::xmlNodePtr _node, Node* _data)
    {
      assert(_node);
      node_data_map[_node] = _data;
    }

    ::xmlNodePtr Tree::findNode(const std::string& _xpath) const
    {
      const ::xmlXPathContextPtr xpathCtx = ::xmlXPathNewContext(doc_);
      const ::xmlXPathObjectPtr xpathObj = ::xmlXPathEvalExpression(
                                             reinterpret_cast<const ::xmlChar*>(_xpath.c_str()), xpathCtx);
      const ::xmlNodeSetPtr nodes = xpathObj->nodesetval;
      // here, we want exactly one matching node.
      ::xmlNodePtr ret = nullptr;
      if (nodes->nodeNr == 1) {
        if (nodes->nodeTab[0]->type == XML_ELEMENT_NODE) {
          ret = nodes->nodeTab[0];
        }
      }
      ::xmlXPathFreeObject(xpathObj);
      ::xmlXPathFreeContext(xpathCtx);
      return ret;
    } // Tree::findNode

    bool Tree::loadCdata(const std::string& _xpath)
    {
      ::xmlNodePtr xmlnode = findNode(_xpath);
      Node* n = getNodeData(xmlnode);
      LeafCdata* cd = dynamic_cast<LeafCdata*>(n);
      if (!cd) return false;
      return cd->load();
    } // Tree::loadCdata

    Node* Tree::getNodeData(::xmlNodePtr _node) const
    {
      assert(_node);
      node_data_map_t::const_iterator i = node_data_map.find(_node);
      assert(i != node_data_map.end());
      return i->second;
    } // Tree::getNodeData

    ::xmlNodePtr Tree::rootNode() const
    {
      return doc_->children;
    } // Tree::rootNode

    bool Tree::isLeaf(::xmlNodePtr n)
    {
      if (n->children == nullptr) return false;
      if (n->children->next != nullptr) return false;
      if (!::xmlNodeIsText(n->children) && n->children->type != XML_CDATA_SECTION_NODE) return false;
      return true;
    }

    bool Tree::isTreeChild(::xmlNodePtr n)
    {
      if (isLeaf(n))
        return false;
      return isTrueChild(n);
    }

    bool Tree::isTrueChild(::xmlNodePtr n)
    {
      if (::xmlNodeIsText(n)) return false;
      if (n->type == XML_COMMENT_NODE) return false;
      if ((n->type == XML_XINCLUDE_START) || (n->type == XML_XINCLUDE_END)) return false;
      return true;
    }
  } // namespace xmldb
} // namespace Belle2

