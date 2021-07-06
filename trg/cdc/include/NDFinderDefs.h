/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef NDFINDERDEFS_H
#define NDFINDERDEFS_H

#define BOOST_MULTI_ARRAY_NO_GENERATORS
#include "boost/multi_array.hpp"

namespace Belle2 {

  /** Store hit patterns in a 5D array
   * (hitid, prio, omega, phi, theta)*/
  typedef unsigned short c5elem;
  typedef boost::multi_array<c5elem, 5> c5array;
  typedef c5array::index c5index;

  /** The Hough space is a 3D array
   * (omega, phi, theta)*/
  typedef unsigned short c3elem;
  typedef boost::multi_array<c3elem, 3> c3array;
  typedef c3array::index c3index;

  /** TS-Id to 1/32 phi-sector mapping is stored in a 2D array */
  typedef unsigned short c2elem;
  typedef boost::multi_array<c2elem, 2> c2array; //HitMod
  typedef c2array::index c2index;
  typedef std::vector<c3index> cell_index;
  typedef unsigned short ushort;

  /** Default binning in a (7/32) phi-sector */
  struct ndbinning {
    c5elem omega = 40;
    c5elem phi = 84;
    c5elem theta = 9;
    c5elem hitid; // 41 axial, 32 stereo
    c5elem prio = 3;
  };
}
#endif
