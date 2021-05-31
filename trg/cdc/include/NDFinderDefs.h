/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sebastian Skambraks                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef NDFINDERDEFS_H
#define NDFINDERDEFS_H

#define BOOST_MULTI_ARRAY_NO_GENERATORS
#include "boost/multi_array.hpp"

namespace Belle2 {
  typedef unsigned short c5elem;
  typedef boost::multi_array<c5elem, 5> c5array;
  typedef c5array::index c5index;
  typedef unsigned short c3elem;
  typedef boost::multi_array<c3elem, 3> c3array;
  typedef c3array::index c3index;
  typedef unsigned short c4elem;
  typedef boost::multi_array<c4elem, 4> c4array;//Clusterize Houghmap
  typedef c4array::index c4index;
  typedef unsigned short c2elem;
  typedef boost::multi_array<c2elem, 2> c2array; //HitMod
  typedef c2array::index c2index;
  typedef std::vector<unsigned short> vecOne; //Clusterize oneAxis
  typedef std::vector<c4index> cell_index;
  typedef std::vector<std::vector<unsigned short>> vecTwo; //Clusterize
  typedef unsigned short ushort;
}
#endif
