#pragma once

#include "framework/xmldb/macros.h"
#include "framework/xmldb/leaf.h"

#include <cstdint>

namespace Belle2 {
  namespace xmldb {
    /** A leaf node containing binary data. */
    class LeafCdata : public Leaf {
    public:
      /** Called when loading from an XML file. */
      LeafCdata(::xmlNodePtr _xmlnode, const Tree* _owner);

      /** Called when loading from the database. */
      LeafCdata(::xmlNodePtr _xmlnode, const Tree* _owner, int32_t _cdata_id,
                int64_t _size);

      /** Returns the data type, Cdata.
       *  @return Leaf::Cdata.
       */
      virtual DataType type() const;

      /** @return A textual representation of the data. */
      virtual std::string textRepresentation();

      /** Return a pointer to the data.
       *  Ownership of the pointer remains with libxml2.
       *  If the data hasn't been loaded from the database before, this is
       *  done automatically by this function.
       *  @return a Pointer to the data block.
       */
      uint8_t* getData() MUST_USE_RESULT;
      /** @return The size of the data block. */
      int64_t getSize() const MUST_USE_RESULT;

      /** @return true, iff the data is already available in memory. */
      bool isLoaded() const MUST_USE_RESULT;
      /** Ensure that the data is available in memory, linked from the
       *  libxml2 tree.
       *  @return true, iff the data could be loaded.
       */
      bool load();

    protected:
      static const int32_t cdata_id_invalid = -1;

      int32_t cdata_id_;
      bool is_loaded_;
      int64_t size_;
    };  // class LeafCdata

  }  // namespace xmldb
}  // namespace Belle2

