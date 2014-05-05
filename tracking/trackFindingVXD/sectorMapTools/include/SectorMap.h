/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

// includes - rootStuff:
#include <TObject.h>
#include <TVector3.h>

// includes - stl:
#include <string>
#include <vector>
#include <utility> // std::pair

// includes - tf-related stuff:
// #include <tracking/dataobjects/TrackFinderVXDTypedefs.h>

// includes - general fw stuff:


namespace Belle2 {


  /**SectorMap - carries metaData to attached sectors and related objects.
   *
   * - knows what is part of the current collection of Sectors
   **/
  class SectorMap : public TObject {
  public:
    typedef std::pair<unsigned int, double> SectorDistance; /**< stores distance to origin (.second) for sector (.first) */
    typedef std::vector< SectorDistance > SectorDistancesMap; /**< stores vector of SectorDistanceInfo */

    /** constructor */
    SectorMap() {}

  protected:

    /** Name of the sectorMap */
    std::string m_nameOfSecMap;


    /** Name of the detectorType (PXD, SVD, VXD) */
    std::string m_detectorType;


    /** allows defining the the config of the sectors in U direction value is valid for each sensor of chosen detector setup, minimum 2 values between 0.0 and 1.0 */
    std::vector<double> m_sectorConfigU;


    /** allows defining the the config of the sectors in V direction value is valid for each sensor of chosen detector setup, minimum 2 values between 0.0 and 1.0 */
    std::vector<double> m_sectorConfigV;


    /** defines the position of the assumed primary vertex */
    TVector3 m_origin;


    /** strength of magnetic field in Tesla, standard is 1.5T */
    double m_magneticFieldStrength;


    /** this variable is reserved for extra info which shall be stored in the container, e.g. date of production or other useful info for the user(it shall be formatted before storing it), this info will be displayed by the VXDTF on Info-level */
    std::string m_additionalInfo;


    /** stores the secID in .first and the value for the distances in .second */
    SectorDistancesMap m_dist2OriginMap;


    /** defines minimal accepted distance for sectors to the origin */
    double m_minDistance2origin;


    /** defines maximum accepted distance for sectors to the origin */
    double m_maxDistance2origin;


    /** if true, filterBadSectorCombis filters sectors by origin not by layerID */
    bool m_sortByDistance2origin;


    ClassDef(SectorMap, 1)
  };
} //Belle2 namespace