/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef TAGGEDTUPLE_H
#define TAGGEDTUPLE_H

#include <tuple>
#include <tracking/cdcLocalTracking/tempro/TaggedType.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Metafunction to extract the tags of a variadic sequence of TaggedTuples returning a tuple of types.
    template<class ... TaggedTypes_>
    using GetTags = typename std::tuple< GetTag<TaggedTypes_>... >;

    /// Metafunction to extract the tags of a variadic sequence of TaggedTuples returning a tuple of types.
    template<class ... TaggedTypes_>
    using GetTypes = typename std::tuple< GetType<TaggedTypes_>... >;


    /// Looks up at, which index the given Type can be found in a tuple. Amounts to a type inheriting from std::integral_constant
    template<class Type, class Tuple>
    struct GetIndexInTuple {
    };

    /// Specialisation for the case that the first type in the tuple is not the Type asked for. Recursion case.
    template<class Type, class HeadType, class ... TailTypes>
    struct GetIndexInTuple<Type, std::tuple<HeadType, TailTypes...> > :
        std::integral_constant < std::size_t, GetIndexInTuple<Type, std::tuple<TailTypes...> >::value + 1 > {
    };

    /// Specialisation for the case that the first type in the tuple is equal to the Type asked for. Recursion end.
    template<class Type, class... TailTypes>
    struct GetIndexInTuple <Type, std::tuple<Type, TailTypes...> >:
        std::integral_constant< std::size_t, 0> {
    };

    /// Looks up at, which index the given Type can be found in any container type.
    template <class Type, class TypeContainer>
    struct GetIndex;

    /// Specialisation for variadic type container.
    template <template<class ...> class TypeContainerTemplate, class Type, class ... Types>
    struct GetIndex<Type,  TypeContainerTemplate<Types...> > :
        GetIndexInTuple<Type, std::tuple<Types...> > {
    };

    /// Substitute a variadic template with another.
    template<class OldWrapped_, template <class ...> class NewWrapper_>
    struct RewrapImpl;

    /// Specilisation deducing the OldWrapper and the contained types and reapplying them to the new wrapper.
    template <template<class ...> class OldWrapper_, template <class ...> class NewWrapper_, class ... Types_>
    struct RewrapImpl<OldWrapper_<Types_...>, NewWrapper_ > {
      typedef NewWrapper_<Types_...> type;
    };

    /// Substitute a variadic template with another.
    template<class OldWrapped_, template <class ...> class NewWrapper_>
    using Rewrap = typename RewrapImpl<OldWrapped_, NewWrapper_>::type;

    template<class Type, class... Types_>
    using First = Type;

    /// Tuple type supporting look up of the individual entries by a tag.
    template<class ... TaggedTypes_>
    class TaggedTuple : public std::tuple<GetType<TaggedTypes_>...> {

    public:
      /// Base class of the tagged tuple which is a tuple containing the bare types.
      typedef  std::tuple<GetType<TaggedTypes_>...> Base;

      /// The tuple type providing the storage for the values
      typedef std::tuple< TaggedTypes_... > TaggedTypes;

      /// Tuple type collecting the tags
      typedef std::tuple< GetTag<TaggedTypes_>... > Tags;

      /// Tuple type collecting the types
      typedef std::tuple< GetType<TaggedTypes_>... > Types;

    public:
      /// Metafunction to get the nth TaggedType
      template<std::size_t index>
      using GetTaggedTypeAtIndex = typename std::tuple_element< index, TaggedTypes>::type;

      /// Metafunction to get the nth Type
      template<std::size_t index>
      using GetTypeAtIndex = typename std::tuple_element< index, Types>::type;

      /// Metafunction to get the nth Tag
      template<std::size_t index>
      using GetTagAtIndex = typename std::tuple_element< index, Tags>::type;

      /// Metafunction to get the TaggedType for a given Tag
      template<class Tag>
      using GetTaggedTypeAtTag = GetTaggedTypeAtIndex< GetIndex<Tag, Tags>::value >;

      /// Metafunction to get the Type for a given Tag
      template<class Tag>
      using GetTypeAtTag = GetTypeAtIndex< GetIndex<Tag, Tags>::value >;

      /// Default constructor with no arguments
      TaggedTuple() : Base() {;}

      /// Constructor taking references forwarded to the base tuple.
      explicit TaggedTuple(const GetType<TaggedTypes_>& ... initialValues) : Base(initialValues...) {;}

      /// Constant getter function taking a tag and returning the value at this tag
      template<class Tag>
      const GetTypeAtTag<Tag>& get() const
      { return std::get<GetIndex<Tag, Tags>::value >(*this); }

      /// Getter function taking a tag and returning the value at this tag
      template<class Tag>
      GetTypeAtTag<Tag>& get()
      { return std::get<GetIndex<Tag, Tags>::value >(*this); }

    };

  } // end namespace CDCLocalTracking
} // namespace Belle2
#endif // TAGGEDTUPLE_H
