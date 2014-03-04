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

    template<class TaggedType>
    struct GetTagImpl {
      typedef typename TaggedType::Tag Tag;
    };

    template<class TaggedType>
    using GetTag = typename GetTagImpl<TaggedType>::Tag;

    /*
    template<class Tag, class HeadTag, class ... TailTags>
    struct GetIndexInVariadic:
      std::integral_constant< std::size_t, GetIndexInVariadic<Tag,TailTags...>::value + 1>
    {
    };

    template<class Tag, class... TailTags>
    struct GetIndexInVariadic <Tag, Tag, TailTags...>:
      std::integral_constant< std::size_t, 0>
    {
    };
    */

    template<class Tag, class TagsTuple>
    struct GetIndexInTuple;

    template<class Tag, class HeadTag, class ... TailTags>
    struct GetIndexInTuple<Tag, std::tuple<HeadTag, TailTags...> > :
        std::integral_constant < std::size_t, GetIndexInTuple<Tag, std::tuple<TailTags...> >::value + 1 > {
    };

    template<class Tag, class... TailTags>
    struct GetIndexInTuple <Tag, std::tuple<Tag, TailTags...> >:
        std::integral_constant< std::size_t, 0> {
    };


    template <class Tag, class TagContainer>
    class GetIndex;

    template <template<class ...> class TypeContainerTemplate, class Tag, class ... Tags>
    struct GetIndex<Tag,  TypeContainerTemplate<Tags...> > : GetIndexInTuple<Tag, std::tuple<Tags...> > {

    };



    template<class ... TaggedTypes>
    class TaggedTuple : public std::tuple<TaggedTypes... > {

    private:
      typedef std::tuple< TaggedTypes... > TaggedTypesTuple;
      typedef std::tuple< GetTag<TaggedTypes>... > TagsTuple;

    public:
      template<std::size_t index>
      using GetTypeAtIndex = typename std::tuple_element< index, std::tuple< TaggedTypes... > >::type;

      template<class Tag>
      using GetTypeAtTag = GetTypeAtIndex< GetIndex<Tag, TagsTuple>::value >;

      template<class Tag>
      const GetTypeAtTag<Tag>&
      get() const
      { std::get<GetIndex< Tag, TagsTuple>::value >(*this); }

      template<class Tag>
      GetTypeAtTag<Tag>&
      get()
      { return std::get<GetIndex< Tag, TagsTuple>::value >(*this); }

    };




  } // end namespace CDCLocalTracking
} // namespace Belle2
#endif // TAGGEDTUPLE_H



