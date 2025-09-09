/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <array>
#define BOOST_MULTI_ARRAY_NO_GENERATORS
#include "boost/multi_array.hpp"

namespace Belle2 {

  /** TS-ID to 1/32 phi-sector mapping is stored in a 2D array */
  typedef boost::multi_array<unsigned short, 2> c2array;
  typedef c2array::index c2index; /**< index of TS-ID to 1/32 phi-sector mapping 2D array */

  /** The Hough space is a 3D array
   * (omega, phi, cot)
   */
  typedef boost::multi_array<unsigned short, 3> c3array;
  typedef c3array::index c3index; /**< index of Hough space 3D array */

  /** Store hit patterns in a 5D array
   * (hitid, prio, omega, phi, cot)
   */
  typedef boost::multi_array<unsigned short, 5> c5array;
  typedef c5array::index c5index; /**< index of store hit pattern 5D array */

  /** The cell index of one Hough space bin */
  typedef std::array<c3index, 3> cell_index;
}
