/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

// includes - rootStuff:
#include <TObject.h>

// includes - stl:
#include <vector>

// includes - tf-related stuff
// includes - general fw stuff


namespace Belle2 {

  /** forward declaration of a friendship */
  class SectorFriendship;


  /** FilterBase is the baseClass for filters applied on (chains of) spacepoints.
   *
   * - is used as the baseClass for a Strategy-pattern of Filters.
   * - these filters use very basic internal filters (like distance3D) to filter spacePoint-combinations
   **/
  class FilterBase : public TObject {
  public:
    /** is currently a counter which counts number of passed filterTests */
    typedef unsigned int CompatibilityValue;
    /** is currently a table of CompatibilityValues which allows to check which combination of spacepoints/segments are allowed to be combined */
    typedef std::vector<std::vector<CompatibilityValue> > CompatibilityTable;

    /** constructor */
    FilterBase(float minValue = 0, float maxValue = 0):
      m_minCutoff(minValue),
      m_maxCutoff(maxValue) {}

    /** called for each compatible sector-sector-combination inhabiting spacePoints */
    virtual void checkSpacePoints(const SectorFriendship* thisFriendship, CompatibilityTable& compatibilityTable) = 0;

  protected:


    /** if value is lower than cutoff, the value will be neglected */
    float m_minCutoff;

    /** if value is higher than cutoff, the value will be neglected */
    float m_maxCutoff;

    ClassDef(FilterBase, 1)
  };
} //Belle2 namespace
