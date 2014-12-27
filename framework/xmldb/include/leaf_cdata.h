#pragma once

#include "framework/xmldb/macros.h"
#include "framework/xmldb/leaf.h"

#include <cstdint>

namespace Belle2 {
  namespace xmldb {
    class LeafCdata : public Leaf {
    public:
      // called when loading from XML
      LeafCdata(::xmlNodePtr _xmlnode, const Tree* _owner);

      // called when loading from DB
      LeafCdata(::xmlNodePtr _xmlnode, const Tree* _owner, int32_t _cdata_id,
                int64_t _size);

      virtual DataType type() const;

      virtual std::string textRepresentation();

      uint8_t* getData() MUST_USE_RESULT;
      int64_t getSize() const MUST_USE_RESULT;

      bool isLoaded() const MUST_USE_RESULT;
      bool load();

    protected:
      static const int32_t cdata_id_invalid = -1;

      int32_t cdata_id_;
      bool is_loaded_;
      int64_t size_;
    };  // class LeafCdata

  }  // namespace xmldb
}  // namespace Belle2

