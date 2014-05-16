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
    typedef std::pair<double, double> NormCoords;



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
    static uShort calcSecID(const std::vector<double>& uConfig, const std::vector<double>& vConfig, NormCoords coords) {
      // safety checks
      throwBoundary(coords.first);
      throwBoundary(coords.second);
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
        if (coords.first >= uConfig.at(uCount) and coords.first <= uConfig.at(uCount + 1)) {
          foundSectorU = true;
          break;
        }
      }

      // searching for sector-slot in v-direction:
      for (; vCount < vMax; ++vCount) {
        if (coords.second >= vConfig.at(vCount) and coords.second <= vConfig.at(vCount + 1)) {
          foundSectorV = true;
          break;
        }
      }

//       B2INFO("foundSectorU: " << foundSectorU << ", foundSectorV: " << foundSectorV);
//       B2INFO("uCount: " << uCount << ", vCount: " << vCount);

      // on a sensor represented as a matrix, the uCount carries the lineID and the vCount the rowID
      if (foundSectorU == true and foundSectorV == true) {
        secID = vCount + uCount * vMax; // 0-based
//  secID = vCount + 1 + uCount * vMax; // 1-based
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
     *   Example: if coords are (0.5,0.5), the coordinate of the middle of the _sector_ is calculated, not the coordinate of the middle of the _sensor_.
     *   Example2: the sector-edges can be retrieved by using e.g.: (0,0), (0,1), (1,0) and (1,1)
     *  the return value is in normalized Coordinates of the _sensor_ again
     */
    static NormCoords calcNormalizedSectorPoint(const std::vector<double>& uConfig, const std::vector<double>& vConfig, uShort secID, NormCoords coords) {
      // safety checks
      throwBoundary(coords.first);
      throwBoundary(coords.second);
      throwBoundaryVector(uConfig);
      throwBoundaryVector(vConfig);

      /**
      uCount
      |
      V coding: secID(uIndex, vIndex):
      vCount-> ----------------------------------------------
      | 0(0,0) | 1(0,1) | 2(0,2) | 3(0,3) | 4(0,4) |
      ----------------------------------------------
      | 5(1,0) | 6(1,1) | 7(1,2) | 8(1,3) | 9(1,4) |
      ----------------------------------------------
      |10(2,0) |11(2,1) |12(2,2) |13(2,3) |14(2,4) |
      ----------------------------------------------

      case vConfig { 0, .2, .4, .6, .8, 1} = 6 entries
      case uConfig { 0, .33, .67, 1} = 4 entries
      therefore:
      vMax = vConfig - 1 = 5,
      uMax = uConfig - 1 = 3;

      0-based-secID:
      secID = vCount + uCount*vMax.
      example 13 = 3 + 2*5, 6 = 1 + 1*5;

      vCount = (secID % vMax).
      example 3 = (13 % 5) = 3

      uCount = (secID - vCount)/vMax;
      */
      uInt vMax = 0;
      if (uInt(vConfig.size()) > 0) { vMax = vConfig.size() - 1 ; }

//       // reconstructing indices for the u and v cuts by using the sectorID:
      uInt vIndex = (secID) % vMax ; // 1-based
      uInt uIndex = (secID - vIndex) / vMax;

      B2DEBUG(100, "SECID: " << secID << ", vIndex: " << vIndex << ", uIndex: " << uIndex << ", vMax:" << vMax);
      B2DEBUG(100, "uConfig.at(uIndex + 1): " << uConfig.at(uIndex + 1) << ", uConfig.at(uIndex): " << uConfig.at(uIndex));
      B2DEBUG(100, "vConfig.at(vIndex + 1): " << vConfig.at(vIndex + 1) << ", vConfig.at(vIndex): " << vConfig.at(vIndex));

      double uPosInSector = (uConfig.at(uIndex + 1) - uConfig.at(uIndex)) * coords.first;
      throwBoundary(uPosInSector);
      double vPosInSector = (vConfig.at(vIndex + 1) - vConfig.at(vIndex)) * coords.second;
      throwBoundary(vPosInSector);

      return std::make_pair(uConfig.at(uIndex) + uPosInSector, vConfig.at(vIndex) + vPosInSector);
    }

  protected:
    /** returns true if value is between 0-1, false if not */
    static void throwBoundary(double value) {
      if (value > 1 or value < 0) {
        throw Out_of_bounds();
      }
    }


    /** returns true if value is between 0-1, false if not */
    static void throwBoundaryVector(const std::vector<double>& vec) {
      for (double value : vec) throwBoundary(value);
    }



  };
} //Belle2 namespace