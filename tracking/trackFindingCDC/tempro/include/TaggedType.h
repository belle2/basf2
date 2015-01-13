/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef TAGGEDTYPE_H
#define TAGGEDTYPE_H

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Simple struct attaching a tag to a type
    template<class Type_, class Tag_ = Type_>
    class TaggedType {
    public:
      typedef Type_ Type;
      typedef Tag_ Tag;
    };

    /// Metafunction to retrieve the Tag from a TaggedType.
    /** Retrieves the Tag of a type, which is a dedicated member type named Tag.*/
    template<class TaggedType_>
    struct GetTagImpl {
      /// Tag type unpacked from the TaggedType.
      typedef typename TaggedType_::Tag result;
    };

    /// Metafunction to retrieve the Tag from a TaggedType.
    /** Defines a shorthand for  GetTagImpl<>::result; */
    template<class TaggedType_>
    using GetTag = typename GetTagImpl<TaggedType_>::result;

    /// Metafunction to retrieve the Type from a TaggedType.
    /** Retrieves the Type of a TaggedType, which is a dedicated member type named Type.*/
    template<class TaggedType_>
    struct GetTypeImpl {
      /// Tag type unpacked from the TaggedType.
      typedef typename TaggedType_::Type result;
    };

    /// Metafunction to retrieve the Type from a TaggedType.
    /** Defines a shorthand for  GetTypeImpl<>::result; */
    template<class TaggedType_>
    using GetType = typename GetTypeImpl<TaggedType_>::result;

  } // end namespace CDCLocalTracking
} // namespace Belle2
#endif // TAGGEDTUPLE_H
