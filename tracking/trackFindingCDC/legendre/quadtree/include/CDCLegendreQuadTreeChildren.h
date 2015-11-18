/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Viktor Trusov, Thomas Hauth                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

/*
 * Object which can store pointers to hits while processing FastHogh algorithm
 *
 * TODO: check if it's possible to store in each hit list of nodes in which we can meet it.
 *
 * Possible runtime improvements:
 * - use a specific allocator for the QuadTrees which can be wiped without calling
 *   each QuadTree constructor once the search is complete.
 */


#pragma once

#include <framework/logging/Logger.h>

#include <boost/math/constants/constants.hpp>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/utility.hpp>
#include <boost/multi_array.hpp>

#include <set>
#include <vector>
#include <array>
#include <algorithm>

#include <cstdlib>
#include <iomanip>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <memory>
#include <cmath>
#include <functional>

namespace Belle2 {
  namespace TrackFindingCDC {

    template<typename quadChild>
    class QuadChildrenTemplate {
    public:

//      typedef std::map<std::pair<size_t, size_t>, quadChild> ChildrenHolder;

      QuadChildrenTemplate()
      {
        // initialize to null
        for (size_t t_index = 0; t_index < m_sizeX; ++t_index) {
          for (size_t r_index = 0; r_index < m_sizeY; ++r_index) {
            m_children[t_index][r_index] = nullptr;
          }
        }
      };

      /*
        This method will apply a lambda function to every child of the quad tree
      */
      void apply(std::function<void(quadChild*)> lmd)
      {
        for (size_t t_index = 0; t_index < m_sizeX; ++t_index) {
          for (size_t r_index = 0; r_index < m_sizeY; ++r_index) {
            if (m_children[t_index][r_index]->getLevel() != 0) {
              lmd(m_children[t_index][r_index]);
            }
          }
        }
      };

      ~QuadChildrenTemplate()
      {
        this->apply([](quadChild * qt) { delete qt;});
      };

      inline void set(const size_t x, const size_t y, quadChild* qt)
      {
        m_children[x][y] = qt;
      }

      inline quadChild* get(const size_t x, const size_t y)
      {
        return m_children[x][y];
      }

      static constexpr size_t m_sizeX = 2;
      static constexpr size_t m_sizeY = 2;

    private:

      quadChild* m_children[m_sizeX][m_sizeY];
    };


  }
}
