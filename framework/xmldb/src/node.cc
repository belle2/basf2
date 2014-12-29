#include "framework/xmldb/leaf_int.h"
#include "framework/xmldb/node.h"
#include "framework/xmldb/tree.h"

#if LIBXML_VERSION < 20703
namespace {

  ::xmlNodePtr
  xmlFirstElementChild(::xmlNodePtr _parent)
  {
    if (!_parent)
      return (NULL);
    ::xmlNodePtr cur = NULL;
    switch (_parent->type) {
      case XML_ELEMENT_NODE:
      case XML_ENTITY_NODE:
      case XML_DOCUMENT_NODE:
      case XML_HTML_DOCUMENT_NODE:
        cur = _parent->children;
        break;
      default:
        return (NULL);
    }
    while (cur != NULL) {
      if (cur->type == XML_ELEMENT_NODE)
        return (cur);
      cur = cur->next;
    }
    return (NULL);
  }
}
#endif

namespace Belle2 {
  namespace xmldb {

    Node::Node(::xmlNodePtr _xmlnode, const Tree* _owner)
      : xmlnode_(_xmlnode), id_(-1), link_invalid(true),
        owner_(_owner), predecessor_(-1), last_changed_commit_(-1)
    {
    } // Node constructor

    Node::~Node()
    {
    } // Node destructor

    std::string Node::attributes() const
    {
      std::string ret;
      for (::xmlAttrPtr attr = xmlnode_->properties; attr; attr = attr->next) {
        // do not include dbctrl: attributes in the list
        if (attr->ns == owner_->configNs_) continue;
        ::xmlChar* val = ::xmlGetProp(xmlnode_, attr->name);
        if (ret.size()) ret.append(" ");
        ret = ret + reinterpret_cast<const char*>(attr->name) + "=\"";
        ret = ret + reinterpret_cast<const char*>(val) + "\"";
        ::xmlFree(val);
      } // for
      return ret;
    } // Node::attributes

    bool Node::isDirty() const
    {
      return link_invalid;
    } // Node::isDirty

    bool Node::isLeaf() const
    {
      return !xmlFirstElementChild(xmlnode_);
    } // Node::isLeaf

    void Node::invalidateData()
    {
      if (id_ != -1) {
        predecessor_ = id_;
      }
      id_ = -1;
      invalidateLink();
    } // Node::invalidateData

    void Node::invalidateLink()
    {
      link_invalid = true;
      LinkParent lp = DBParent();
      if (lp.node) lp.node->invalidateLink();
    } // Node::invalidateLink

    std::string Node::name() const
    {
      return reinterpret_cast<const char*>(xmlnode_->name);
    } // Node::name

    std::string Node::path() const
    {
      const Node* const p = parent();
      if (p) {
        return p->name() + "." + name();
      } else {
        return name();
      }
    } // Node::path

    Node* Node::parent() const
    {
      if (!xmlnode_->parent || xmlnode_->parent->type == XML_DOCUMENT_NODE) {
        return nullptr;
      }
      return owner_->getNodeData(xmlnode_->parent);
    } // Node::parent

    Node::LinkParent Node::DBParent() const
    {
      LinkParent p;
      const ::xmlNodePtr prev = xmlnode_->prev;
      if (prev != nullptr) {
        p.node = owner_->getNodeData(prev);
        p.type = LinkType::sibling;
      } else {
        if (!xmlnode_->parent || xmlnode_->parent->type == XML_DOCUMENT_NODE) {
          p.node = nullptr;
        } else {
          p.node = owner_->getNodeData(xmlnode_->parent);
        }
        p.type = LinkType::parent;
      }
      return p;
    }
  } // namespace xmldb
} // namespace Belle2

