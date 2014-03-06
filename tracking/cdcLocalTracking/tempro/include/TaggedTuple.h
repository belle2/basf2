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

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Metafunction to retrieve the Tag.
    /** Retrieves the Tag of a type, which is a dedicated member type named Tag.*/
    template<class TaggedType>
    struct GetTagImpl {
      typedef typename TaggedType::Tag type;
    };

    /// Metafunction to retrieve the Tag from a Tagged type.
    /** Defines a shorthand for  GetTagImpl<>::type; */
    template<class TaggedType>
    using GetTag = typename GetTagImpl<TaggedType>::type;

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


    /// Tuple type supporting look up of the individual entries by a tag.
    template<class ... TaggedTypes>
    class TaggedTuple : public std::tuple<TaggedTypes... > {

    public:
      /// Metafunction to extract the tag from a tagged type
      template<class TaggedType>
      using GetTag = typename GetTagImpl<TaggedType>::type;

    private:
      /// The tuple type providing the storage for the values
      typedef std::tuple< TaggedTypes... > TaggedTypesTuple;

      /// Tuple type collecting of the tags
      typedef std::tuple< GetTag<TaggedTypes>... > TagsTuple;

    public:
      /// Metafunction to get the nth type
      template<std::size_t index>
      using GetTypeAtIndex = typename std::tuple_element< index, std::tuple< TaggedTypes... > >::type;

      /// Metafunction to get the type for a given tag
      template<class Tag>
      using GetTypeAtTag = GetTypeAtIndex< GetIndex<Tag, TagsTuple>::value >;


      /// Constant getter function taking a tag and returning the value at this tag
      template<class Tag>
      const GetTypeAtTag<Tag>&
      get() const
      { return std::get<GetIndex< Tag, TagsTuple>::value >(*this); }

      /// Getter function taking a tag and returning the value at this tag
      template<class Tag>
      GetTypeAtTag<Tag>&
      get()
      { return std::get<GetIndex< Tag, TagsTuple>::value >(*this); }

    };

  } // end namespace CDCLocalTracking
} // namespace Belle2
#endif // TAGGEDTUPLE_H
