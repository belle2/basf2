/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SecMapVector_H
#define SecMapVector_H

#include "VXDTFRawSecMap.h"

// stl:
#include <vector>
#include <string>
#include <utility> // std::pair

// root
#include <TObject.h>

namespace Belle2 {
  /** \addtogroup dataobjects
   * @{
   */

  /** A simple TObject-wrapper for std::vector of VXDTFRawSecMap
   */
  class SecMapVector: public TObject {
  public:
    typedef std::pair< std::string, Belle2::VXDTFRawSecMap> MapPack; /**< stores Map (.second) and Name of it (.first) */

    /** Default constructor for the ROOT IO. */
    SecMapVector() {}

    /** getter - allows access to vector (if the standard members can not help you) */
    std::vector< Belle2::SecMapVector::MapPack>& getFullVector() { return m_sectorMapVector; }

    /** clear all maps */
    void clear() { m_sectorMapVector.clear(); }

    /** add new map to container */
    void push_back(Belle2::SecMapVector::MapPack& newMapPack) {
      m_sectorMapVector.push_back(newMapPack);
    }

    /** returns size of stored RawSecMap */
    int size() { return m_sectorMapVector.size(); }

  protected:
    std::vector< Belle2::SecMapVector::MapPack> m_sectorMapVector; /**< contains full information of all sectorMaps */

    ClassDef(SecMapVector, 1)
  };

  /** @}*/
} //Belle2 namespace
#endif
