#include "framework/xmldb/dbfactory.h"
#include "framework/xmldb/leaf_cdata.h"
#include "framework/xmldb/connection.h"
#include "framework/xmldb/reader_db.h"

#include <framework/logging/Logger.h>

#include <cassert>
#include <libxml/tree.h>
#include <memory>
#include <sstream>

namespace Belle2 {
  namespace xmldb {

    LeafCdata::LeafCdata(::xmlNodePtr _xmlnode, const Tree* _owner)
      : Leaf(_xmlnode, _owner), cdata_id_(cdata_id_invalid), is_loaded_(true)
    {
      assert(xmlnode_->children
             && xmlnode_->children->type == XML_CDATA_SECTION_NODE);
      size_ = ::xmlStrlen(xmlnode_->children->content);
    }

    LeafCdata::LeafCdata(::xmlNodePtr _xmlnode, const Tree* _owner,
                         int32_t _cdata_id, int64_t _size)
      : Leaf(_xmlnode, _owner), cdata_id_(_cdata_id), is_loaded_(false)
    {
      size_ = _size;
    }

    Leaf::DataType LeafCdata::type() const
    {
      return DataType::Cdata;
    }

    std::string LeafCdata::textRepresentation()
    {
      std::ostringstream ss;
      ss << "/CDATA " << getSize() << " bytes, ";
      if (!isLoaded()) ss << "not ";
      ss << "loaded/";
      return ss.str();
    }

    uint8_t* LeafCdata::getData()
    {
      load();
      // TODO: what about split CDATA?
      return xmlnode_->children->content;
    }

    int64_t LeafCdata::getSize() const
    {
      // TODO: what about split CDATA?
      return size_;
    }

    bool LeafCdata::isLoaded() const
    {
      return is_loaded_;
    }

    bool LeafCdata::load()
    {
      if (isLoaded()) return true;
      B2DEBUG(1000, "LOAD CDATA " << name());

      std::unique_ptr<Reader_DB> reader(new Reader_DB);
      std::vector<uint8_t> data = reader->readCdata(cdata_id_, getSize());

      const ::xmlNodePtr cdataPtr = ::xmlNewCDataBlock(NULL, data.data(),
                                                       data.size());
      ::xmlAddChild(xmlnode_, cdataPtr);
      is_loaded_ = true;
      return true;
    }
  } // namespace xmldb
} // namespace Belle2

