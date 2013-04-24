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

#include <map>
#include <vector>
#include <algorithm>
#include <string>


#include "Cutoff.h"

namespace Belle2 {

//  class Cutoff;
  /** Class SectorFriends
   * is needed by the VXDSegmentCell-class. Carries information about the compatibility of two sectors and the cutoffs relevant for them as well */
  class SectorFriends {
  public:
    /** constructor */
    SectorFriends(std::string myName, std::string secName):
      m_friendName(myName),
      m_sectorName(secName) { }

    /** setters */
    void addValuePair(std::string cutOffType, std::pair<double, double> values); /**< adds a pair of cutoffs of defined type */

    /** getters */
    std::pair<double, double> exportFilters(std::string cutOffType); /**< exports filters stored in member map */
    Cutoff* getCutOff(std::string cutOffType); /**< returns chosen type of cutoff */
    const std::vector<std::string> getSupportedCutoffs(); /**< returns list of cutoffs supported by current sector-friend-combination */

  protected:
    std::map<std::string, Belle2::Cutoff*> m_filterMap; /**< a map carrying all cutoffs stored in current sector-friend-combination */
    std::string m_friendName; /**< the name of the sector representing this friend */
    std::string m_sectorName; /**< the name of the sector having this friend */
  };

} //Belle2 namespace
#endif
