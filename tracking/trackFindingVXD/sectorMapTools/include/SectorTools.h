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
#include <TVector3.h>

// includes - stl:
#include <vector>
#include <utility> // std::pair
#include <limits>       // std::numeric_limits

// includes - tf-related stuff:
// includes - general fw stuff:
#include <framework/core/FrameworkExceptions.h>


namespace Belle2 {


  /** SectorTools - contains often needed transformer-functions.
   **/
  class SectorTools {


  public:
    /** .first is uValue, .second is vValue (both defined between 0-1) */
    typedef std::pair<float, float> NormCoords;



    /** shortcut for unsigned int short */
    typedef unsigned short uShort;



    /** shortcut for unsigned int */
    typedef unsigned int uInt;



    /** exception shall be thrown if value is not between 0-1 and therefore not normalized */
    BELLE2_DEFINE_EXCEPTION(Out_of_bounds, "The normalized coordinates (sectorConfig or coordinates themselves) are not defined between 0-1!");



    /** calculates the sector ID for current hit.
     * for given configuration for u and v coordinates, it takes a hit in normalized coordinates (sensor-independent within sensor-boundaries) and calculates the sectorID.
     * if no secID is found, it returns unsigned short :: max
     */
    static uShort calcSecID(const std::vector<float>& uConfig, const std::vector<float>& vConfig, NormCoords coords) {
      // safety checks
      throwBoundary(float(coords.first));
      throwBoundary(float(coords.second));
      throwBoundaryVector(uConfig);
      throwBoundaryVector(vConfig);

      // needed for determining the sectorID of this sensor
      uShort uCount = 0,
             vCount = 0,
             secID = std::numeric_limits<uShort>::max();
      bool foundSectorU = false,
           foundSectorV = false;
      unsigned int uMax = uConfig.size() - 1,
                   vMax = vConfig.size() - 1;
//       B2INFO("uMax: " << uMax << ", vMax: " << vMax);

      // searching for sector-slot in u-direction:
      for (; uCount < uMax; ++uCount) {
        if (coords.first > uConfig.at(uCount) and coords.first < uConfig.at(uCount + 1)) {
          foundSectorU = true;
          break;
        }
      }

      // searching for sector-slot in v-direction:
      for (; vCount < vMax; ++vCount) {
        if (coords.second > vConfig.at(vCount) and coords.second < vConfig.at(vCount + 1)) {
          foundSectorV = true;
          break;
        }
      }

//       B2INFO("foundSectorU: " << foundSectorU << ", foundSectorV: " << foundSectorV);
//       B2INFO("uCount: " << uCount << ", vCount: " << vCount);

      // on a sensor represented as a matrix, the uCount carries the lineID and the vCount the rowID
      if (foundSectorU == true and foundSectorV == true) {
        secID = vCount + 1 + uCount * vMax;
      }
//       B2INFO("secID: " << secID );

      return secID;
    }



    /** calculates the normalized coordinates for a point in given sectorID.
     *
     * first parameter is a vector containing the cuts for the sectors in u-direction (in consecutive and increasing order, have to be normalized).
     * second parameter is a vector containing the cuts for the sectors in v-direction (in consecutive and increasing order, have to be normalized).
     * third parameter is the sectorID in which the normalized point is calculated
     * fourth parameter is the point for which the normalized sector point shall be calculated.
     *  WARNING: this value is normalized not within the boundaries of the sensor but within the boundaries of the sector!
     *   Example: if coords are (0.5,0.5), the coordinate of the middle of the sector is calculated, not the coordinate of the middle of the sensor.
     *   Example2: the sector-edges can be retrieved by using e.g.: (0,0), (0,1), (1,0) and (1,1)
     */
    static NormCoords calcNormalizedSectorPoint(const std::vector<float>& uConfig, const std::vector<float>& vConfig, uShort secID, NormCoords coords) {
      // safety checks
      throwBoundary(float(coords.first));
      throwBoundary(float(coords.second));
      throwBoundaryVector(uConfig);
      throwBoundaryVector(vConfig);

//       B2INFO ("SECID: " << secID);

      uInt vMax = vConfig.size() - 1 ;
//       // reconstructing indices for the u and v cuts by using the sectorID:
      uInt vIndex = (secID - 1) % vMax ;
      uInt uIndex = (secID - 1 - vIndex) / vMax;

//        B2INFO ("vIndex: " << vIndex << ", uIndex: " << uIndex);


      float uPosInSector = (uConfig.at(uIndex + 1) - uConfig.at(uIndex)) * coords.first;
      throwBoundary(uPosInSector);
      float vPosInSector = (vConfig.at(vIndex + 1) - vConfig.at(vIndex)) * coords.second;
      throwBoundary(vPosInSector);

      return std::make_pair(uConfig.at(uIndex) + uPosInSector, vConfig.at(vIndex) + vPosInSector);
    }

  protected:
    /** returns true if value is between 0-1, false if not */
    static void throwBoundary(float value) {
      if (value > 1 or value < 0) {

        // TO DO: Activate
//   throw Out_of_bounds;
      }
    }


    /** returns true if value is between 0-1, false if not */
    static void throwBoundaryVector(const std::vector<float>& vec) {
      for (float value : vec) throwBoundary(value);
    }



  };
} //Belle2 namespace