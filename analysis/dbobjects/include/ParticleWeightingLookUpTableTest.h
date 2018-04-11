/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ilya Komarov                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <TObject.h>
#include <map>
#include <vector>
#include <string>

namespace Belle2 {

  // Just pair of numners - min and max values of bin border
  typedef std::pair<double, double> BinLimits;

  // N-dim bin: pairs of bin limits with name of the axis variable.
  typedef std::map<std::string, BinLimits> NDBin;

  // Map of bin limits with bin ID
  typedef std::map<BinLimits, double> Nameless1DMap;

  // Map of 2D bins with bin ID
  typedef std::map<BinLimits, Nameless1DMap> Nameless2DMap;

  // Map of 3D bins with bin ID
  typedef std::map<BinLimits, Nameless2DMap> Nameless3DMap;

  // Vector of axis names paired with 3D map.
  typedef std::pair< std::vector<std::string>, Nameless3DMap> Named3DMap;

  typedef std::map<std::string, double> WeightInfo;

  typedef std::map<double, WeightInfo> WeightMap;
  class ParticleWeightingLookUpTableTest : public TObject {

    //std::pair<ParticleWeightingKeyMap, WeightMap> m_test;
    Named3DMap m_test;

    ClassDef(ParticleWeightingLookUpTableTest, 1);

  public:
    /**
    * Constructor
    */
    ParticleWeightingLookUpTableTest()
    {

    };
  };

} // Belle2 namespace


