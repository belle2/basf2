/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef STRUCTOFVECTORS_H
#define STRUCTOFVECTORS_H

#include <vector>
#include <tracking/cdcLocalTracking/tempro/TaggedType.h>
#include <tracking/cdcLocalTracking/tempro/TaggedTuple.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Wraps the Type of a TaggedTyp by a wrapper type and constructs a TaggedType with that Wrapper<Type> and the original Tag
    template<class TaggedType_, template<class> class Wrapper_>
    using WrapTaggedType = TaggedType<Wrapper_< GetType<TaggedType_>>, GetTag<TaggedType_>>;

    template<class Type_>
    using WithVector = std::vector<Type_>;

    template<class Type_>
    using WithVectorIterator = typename std::vector<Type_>::iterator;

    /// Wraps the Type of a TaggedTyp by a vector and constructs a TaggedType with that vector<Type> and the original Tag
    template<class TaggedType_>
    using TaggedVectorType = WrapTaggedType<TaggedType_, WithVector>;

    template<class TaggedType_>
    using TaggedVectorIteratorType = WrapTaggedType<TaggedType_, WithVectorIterator>;

    template<class ... TaggedTypes_>
    class StructOfVectors : public TaggedTuple<TaggedVectorType<TaggedTypes_>...> {
    };

    template<class ... TaggedTypes_>
    class StructOfVectorsIterator : public TaggedTuple<TaggedVectorType<TaggedTypes_>...> {

      /// The Base class type.
      typedef TaggedTuple<TaggedVectorType<TaggedTypes_>...> Base;

      /// Tuple type collecting the types
      typedef std::tuple< GetType<TaggedTypes_>... > ValueTypes;

      /// Metafunction to get the nth ValueType
      template<std::size_t index>
      using GetValueTypeAtIndex = typename std::tuple_element< index, ValueTypes>::type;

      /// Metafunction to get the TaggedType for a given Tag
      template<class Tag_>
      using GetValueTypeAtTag = GetValueTypeAtIndex< GetIndex<Tag_, typename Base::Tags>::value >;

      StructOfVectorsIterator(const WithVectorIterator< GetType<TaggedTypes_>>& ... iterators) : Base(iterators...) {;}


      template<class Tag_>
      GetValueTypeAtTag<Tag_>& getValue() {
        typename Base::GetTypeAtTag& singleIterator = Base::template get<Tag_>();
        return *singleIterator;
      }

      template<class Tag_>
      const GetValueTypeAtTag<Tag_>& getValue() const {
        typename Base::GetTypeAtTag& singleIterator = Base::template get<Tag_>();
        return *singleIterator;
      }

      void operator++() { evalVariadic(next<GetTag<TaggedTypes_>>()...); }

      bool operator==(const StructOfVectorsIterator<TaggedTypes_...>& other) const {
        // Only compare the first iterator, since all the others should be at the same position.
        typedef Rewrap<typename Base::Tags, First> FirstTag;
        return Base::template get<FirstTag>() == other.template get<FirstTag>();
      }

      bool operator!=(const StructOfVectorsIterator<TaggedTypes_...>& other) const {
        return not operator==(other);
      }

    private:
      template<class Tag_>
      void next() {
        typename Base::GetTypeAtTag& singleIterator = Base::template get<Tag_>();
        return ++singleIterator;
      }

    };
  } // end namespace CDCLocalTracking
} // namespace Belle2
#endif // TAGGEDTUPLE_H
