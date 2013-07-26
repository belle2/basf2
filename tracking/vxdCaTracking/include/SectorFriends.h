/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
/// NOTE:These Classes are needed by dataobjects, but shall not be compatible to the datastore for performance reasons,
/// therefore not inheriting from T-object (compare to VXDID)
#ifndef SECTORFRIENDS_H
#define SECTORFRIENDS_H

#include <vector>
#include <algorithm>

#include "FilterID.h"
#include "Cutoff.h"

namespace Belle2 {
  namespace Tracking {

    //  class Cutoff;
    /** Class SectorFriends
    * is needed by the VXDSegmentCell-class. Carries information about the compatibility of two sectors and the cutoffs relevant for them as well */
    class SectorFriends {
    public:
      /** constructor */
      SectorFriends(unsigned int myName, unsigned int secName):
        m_friendName(myName),
        m_sectorName(secName) { m_filters.assign(FilterID::numFilters, NULL); }

      /** setters */
      void addValuePair(int aFilter, std::pair<double, double> values); /**< adds a pair of cutoffs of defined type */

      /** getters */
      std::pair<double, double> exportFilters(int aFilter); /**< exports filters stored in member */
      Cutoff* getCutOff(int aFilter); /**< returns chosen type of cutoff */
      void getSupportedCutoffs(std::vector<int>& supportedCutoffs); /**< returns list of cutoffs supported by current sector-friend-combination */

    protected:
      std::vector<Cutoff*> m_filters; /**< a vector carrying all cutoffs stored in current sector-friend-combination */
      unsigned int m_friendName; /**< the name of the sector representing this friend. Info is encoded as an int, if you want to have human readable values, you have to convert it using FullSecId */
      unsigned int m_sectorName; /**< the name of the sector having this friend. Info is encoded as an int, if you want to have human readable values, you have to convert it using FullSecId */
    };
  } // Tracking namespace
} //Belle2 namespace
#endif
