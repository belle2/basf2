/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Oliver Frost                                             *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/hough/GenIndices.h>

#include <array>
#include <iterator>
#include <assert.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    template<std::size_t... factors>
    struct Product;

    template<>
    struct Product<> :
      std::integral_constant<std::size_t, 1> {;};

    template<std::size_t factor, std::size_t... factors>
    struct Product<factor, factors...> :
      std::integral_constant<std::size_t, factor* Product<factors...>::value > {;};


    template<std::size_t... divisions>
    class LinearBoxPartition {

    public:
      static const std::size_t s_nSubNodes = Product<divisions...>::value;
      static const std::size_t s_divisions[sizeof...(divisions)];

    public:
      template<class Node>
      class ChildrenStructure {

      private:
        template<std::size_t... Is>
        inline static Node makeSubNode(const Node& box,
                                       std::size_t globalISubNode,
                                       IndexSequence<Is...> /*coordinatesIndex*/)
        {
          std::array<std::size_t, sizeof...(divisions)> indices;
          for (size_t iIndex = 0 ; iIndex <  sizeof...(divisions); ++iIndex) {
            indices[iIndex] = globalISubNode % s_divisions[iIndex];
            globalISubNode /= s_divisions[iIndex];
          }
          assert(globalISubNode == 0);
          return Node(box.template getDivisionBounds<Is>(s_divisions[Is], indices[Is]) ...);
        }

        template<std::size_t... Is>
        inline static std::array<Node, s_nSubNodes>
        makeSubNodes(const Node& box,
                     IndexSequence<Is...> /*globalSubNodeIndex*/)
        {
          return {{ makeSubNode(box, Is, GenIndices<sizeof...(divisions)>())... }};
        }

      public:
        ChildrenStructure(const Node& box) :
          m_nodes(makeSubNodes(box, GenIndices<s_nSubNodes>()))
        {;}

      public:
        /// Begin iterator over the subnodes
        typename std::array<Node, s_nSubNodes>::iterator
        begin()
        { return std::begin(m_nodes); }

        /// End iterator over the subnodes
        typename std::array<Node, s_nSubNodes>::iterator
        end()
        { return std::end(m_nodes); }

        /// Get number of child nodes
        size_t size() const
        { return s_nSubNodes; }
      private:
        /// Memory of the subnodes.
        std::array<Node, s_nSubNodes> m_nodes;
      };
    };

    template<std::size_t... divisions>
    const std::size_t LinearBoxPartition<divisions...>::s_divisions[sizeof...(divisions)] =
    {divisions...};


  }
}
