/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once


#include <tracking/dataobjects/FilterID.h>

// 2-hit:
#include <tracking/vxdCaTracking/TwoHitFilters.h>
// 3-hit:
#include <tracking/vxdCaTracking/ThreeHitFilters.h>
// 4-hit:
#include <tracking/vxdCaTracking/FourHitFilters.h>

#include <framework/logging/Logger.h>

#include <string>

#include <TVector3.h>


namespace Belle2 {

  /** The factory serves as an interface between all x-hit-filters and a user only knowing their name (in string), but not their type */
  template<class PointType>
  class XHitFilterFactory {
  public:

    /** constructor where virtual IP has been passed */
    XHitFilterFactory(double x = 0, double y = 0, double z = 0, double mField = 1.5) :
      m_virtualIP(x, y, z)
    {
      m_threeHit.resetMagneticField(mField);
      m_fourHit.resetMagneticField(mField);
    }



    /** constructor where nothing has been passed */
    /*    XHitFilterFactory() : m_virtualIP(0, 0, 0) {}*/



    /** shortCut for better readability. */
    typedef FilterID::filterTypes XHitFilterType;

    /** typedef for more readable function-type - to be used for 2-hit-selectionVariables. */
    using TwoHitFunction = typename std::function<double(const PointType&, const PointType&)>;

    /** typedef for more readable function-type - to be used for 3-hit-selectionVariables. */
    using ThreeHitFunction = typename std::function<double(const PointType&, const PointType&, const PointType&)>;

    /** typedef for more readable function-type - to be used for 4-hit-selectionVariables. */
    using FourHitFunction = typename std::function<double(const PointType&, const PointType&, const PointType&, const PointType&)>;



    /** For given name of a variableType a function for the corresponding Filter is returned. */
    TwoHitFunction get2HitInterface(std::string variableName)
    {
      XHitFilterType varType = FilterID::getTypeEnum(variableName);
      return get2HitInterface(varType);
    }



    /** For given name of a variableType a function for the corresponding Filter is returned. */
    TwoHitFunction get2HitInterface(XHitFilterType variableType)
    {
      // prepare stuff for 2-hit-filters:
      auto prepareStuff = [ = ](const PointType & outerHit, const PointType & innerHit) -> void {
        TVector3 tOuterHit(outerHit.X(), outerHit.Y(), outerHit.Z());
        TVector3 tInnerHit(innerHit.X(), innerHit.Y(), innerHit.Z());
        m_twoHit.resetValues(tOuterHit, tInnerHit);
      };

      /// 2-hit:
      if (variableType == FilterID::distance3D) {
        return [ = ](const PointType & outerHit, const PointType & innerHit) -> double {
          prepareStuff(outerHit, innerHit);
          return m_twoHit.calcDist3D();
        };
      }

      if (variableType == FilterID::distanceXY) {
        return [ = ](const PointType & outerHit, const PointType & innerHit) -> double {
          prepareStuff(outerHit, innerHit);
          return m_twoHit.calcDistXY();
        };
      }

      if (variableType == FilterID::distanceZ) {
        return [ = ](const PointType & outerHit, const PointType & innerHit) -> double {
          prepareStuff(outerHit, innerHit);
          return m_twoHit.calcDistZ();
        };
      }

      if (variableType == FilterID::slopeRZ) {
        return [ = ](const PointType & outerHit, const PointType & innerHit) -> double {
          prepareStuff(outerHit, innerHit);
          return m_twoHit.calcSlopeRZ();
        };
      }

      if (variableType == FilterID::normedDistance3D) {
        return [ = ](const PointType & outerHit, const PointType & innerHit) -> double {
          prepareStuff(outerHit, innerHit);
          return m_twoHit.calcNormedDist3D();
        };
      }

      // prepare stuff for 2+1-hit-filters
      auto prepareStuffHiOc = [ = ](const PointType & outerHit, const PointType & innerHit) -> void {
        TVector3 tOuterHit(outerHit.X(), outerHit.Y(), outerHit.Z());
        TVector3 tInnerHit(innerHit.X(), innerHit.Y(), innerHit.Z());
        m_threeHit.resetValues(tOuterHit, tInnerHit, m_virtualIP);
      };


      /// 2+1-hit:
      if (variableType == FilterID::anglesHighOccupancy3D) {
        return [ = ](const PointType & outerHit, const PointType & innerHit) -> double {
          prepareStuffHiOc(outerHit, innerHit);
          return m_threeHit.calcAngle3D();
        };
      }

      if (variableType == FilterID::anglesHighOccupancyXY) {
        return [ = ](const PointType & outerHit, const PointType & innerHit) -> double {
          prepareStuffHiOc(outerHit, innerHit);
          return m_threeHit.calcAngleXY();
        };
      }

      if (variableType == FilterID::anglesHighOccupancyRZ) {
        return [ = ](const PointType & outerHit, const PointType & innerHit) -> double {
          prepareStuffHiOc(outerHit, innerHit);
          return m_threeHit.calcAngleRZ();
        };
      }

      if (variableType == FilterID::distanceHighOccupancy2IP) {
        return [ = ](const PointType & outerHit, const PointType & innerHit) -> double {
          prepareStuffHiOc(outerHit, innerHit);
          return m_threeHit.calcCircleDist2IP();
        };
      }

      if (variableType == FilterID::deltaSlopeHighOccupancyRZ) {
        return [ = ](const PointType & outerHit, const PointType & innerHit) -> double {
          prepareStuffHiOc(outerHit, innerHit);
          return m_threeHit.calcDeltaSlopeRZ();
        };
      }

      if (variableType == FilterID::pTHighOccupancy) {
        return [ = ](const PointType & outerHit, const PointType & innerHit) -> double {
          prepareStuffHiOc(outerHit, innerHit);
          return m_threeHit.calcPt();
        };
      }

      if (variableType == FilterID::helixParameterHighOccupancyFit) {
        return [ = ](const PointType & outerHit, const PointType & innerHit) -> double {
          prepareStuffHiOc(outerHit, innerHit);
          return m_threeHit.calcHelixParameterFit();
        };
      }



      B2ERROR(" XHitFilterAdapter-2Hit: given name (raw/full): " << variableType <<
              "/" << FilterID::getTypeName(variableType) <<
              " is not known, returning dummy function with 0.0 as a result instead!");

      return [&](const PointType&, const PointType&) -> double { return 0.0; };
    }



    /** For given name of a variableType a function for the corresponding Filter is returned. */
    ThreeHitFunction get3HitInterface(std::string variableName)
    {
      XHitFilterType varType = FilterID::getTypeEnum(variableName);
      return get3HitInterface(varType);
    }



    /** For given name of a variableType a function for the corresponding Filter is returned. */
    ThreeHitFunction get3HitInterface(XHitFilterType variableType)
    {
      // prepare stuff for 3-hit-filters
      auto prepareStuff3Hit = [ = ](const PointType & outerHit, const PointType & centerHit, const PointType & innerHit) -> void {
        TVector3 tOuterHit(outerHit.X(), outerHit.Y(), outerHit.Z());
        TVector3 tCenterHit(centerHit.X(), centerHit.Y(), centerHit.Z());
        TVector3 tInnerHit(innerHit.X(), innerHit.Y(), innerHit.Z());
        m_threeHit.resetValues(tOuterHit, tCenterHit, tInnerHit);
      };

      /// 3-hit:
      if (variableType == FilterID::angles3D) {
        return [ = ](const PointType & outerHit, const PointType & centerHit, const PointType & innerHit) -> double {
          prepareStuff3Hit(outerHit, centerHit, innerHit);
          return m_threeHit.calcAngle3D();
        };
      }

      if (variableType == FilterID::anglesRZ) {
        return [ = ](const PointType & outerHit, const PointType & centerHit, const PointType & innerHit) -> double {
          prepareStuff3Hit(outerHit, centerHit, innerHit);
          return m_threeHit.calcAngleRZ();
        };
      }

      if (variableType == FilterID::anglesXY) {
        return [ = ](const PointType & outerHit, const PointType & centerHit, const PointType & innerHit) -> double {
          prepareStuff3Hit(outerHit, centerHit, innerHit);
          return m_threeHit.calcAngleXY();
        };
      }

      if (variableType == FilterID::deltaSlopeRZ) {
        return [ = ](const PointType & outerHit, const PointType & centerHit, const PointType & innerHit) -> double {
          prepareStuff3Hit(outerHit, centerHit, innerHit);
          return m_threeHit.calcDeltaSlopeRZ();
        };
      }

      if (variableType == FilterID::pT) {
        return [ = ](const PointType & outerHit, const PointType & centerHit, const PointType & innerHit) -> double {
          prepareStuff3Hit(outerHit, centerHit, innerHit);
          return m_threeHit.calcPt();
        };
      }

      if (variableType == FilterID::distance2IP) {
        return [ = ](const PointType & outerHit, const PointType & centerHit, const PointType & innerHit) -> double {
          prepareStuff3Hit(outerHit, centerHit, innerHit);
          return m_threeHit.calcCircleDist2IP();
        };
      }

      if (variableType == FilterID::helixParameterFit) {
        return [ = ](const PointType & outerHit, const PointType & centerHit, const PointType & innerHit) -> double {
          prepareStuff3Hit(outerHit, centerHit, innerHit);
          return m_threeHit.calcHelixParameterFit();
        };
      }

      if (variableType == FilterID::deltaSOverZ) {
        return [ = ](const PointType & outerHit, const PointType & centerHit, const PointType & innerHit) -> double {
          prepareStuff3Hit(outerHit, centerHit, innerHit);
          return m_threeHit.calcDeltaSOverZ();
        };
      }

      if (variableType == FilterID::deltaSlopeZOverS) {
        return [ = ](const PointType & outerHit, const PointType & centerHit, const PointType & innerHit) -> double {
          prepareStuff3Hit(outerHit, centerHit, innerHit);
          return m_threeHit.calcDeltaSlopeZOverS();
        };
      }


      // prepare stuff for 3+1-hit-filters
      auto prepareStuff4Hit = [ = ](const PointType & outerHit, const PointType & centerHit,
      const PointType & innerHit) -> void {
        TVector3 tOuterHit(outerHit.X(), outerHit.Y(), outerHit.Z());
        TVector3 tCenterHit(centerHit.X(), centerHit.Y(), centerHit.Z());
        TVector3 tInnerHit(innerHit.X(), innerHit.Y(), innerHit.Z());
        m_fourHit.resetValues(tOuterHit, tCenterHit, tInnerHit, m_virtualIP);
      };

      ///3+1hit:
      if (variableType == FilterID::deltapTHighOccupancy) {
        return [ = ](const PointType & outerHit, const PointType & centerHit, const PointType & innerHit) -> double {
          prepareStuff4Hit(outerHit, centerHit, innerHit);
          return m_fourHit.calcDeltapT();
        };
      }

      if (variableType == FilterID::deltaDistanceHighOccupancy2IP) {
        return [ = ](const PointType & outerHit, const PointType & centerHit, const PointType & innerHit) -> double {
          prepareStuff4Hit(outerHit, centerHit, innerHit);
          return m_fourHit.calcDeltaDistCircleCenter();
        };
      }


      B2ERROR(" XHitFilterAdapter-3Hit: given name (raw/full): " << variableType <<
              "/" << FilterID::getTypeName(variableType) <<
              " is not known, returning dummy function with 0.0 as a result instead!");

      return [&](const PointType&, const PointType&, const PointType&) -> double { return 0.0; };
    }



    /** For given name of a variableType a function for the corresponding Filter is returned. */
    FourHitFunction get4HitInterface(std::string variableName)
    {
      XHitFilterType varType = FilterID::getTypeEnum(variableName);
      return get4HitInterface(varType);
    }



    /** For given name of a variableType a function for the corresponding Filter is returned. */
    FourHitFunction get4HitInterface(XHitFilterType variableType)
    {

      // prepare stuff for 4-hit-filters
      auto prepareStuff4Hit = [ = ](const PointType & outerHit, const PointType & outerCenterHit,
      const PointType & innerCenterHit, const PointType & innerHit) -> void {
        TVector3 tOuterHit(outerHit.X(), outerHit.Y(), outerHit.Z());
        TVector3 tOuterCenterHit(outerCenterHit.X(), outerCenterHit.Y(), outerCenterHit.Z());
        TVector3 tInnerCenterHit(innerCenterHit.X(), innerCenterHit.Y(), innerCenterHit.Z());
        TVector3 tInnerHit(innerHit.X(), innerHit.Y(), innerHit.Z());
        m_fourHit.resetValues(tOuterHit, tOuterCenterHit, tInnerCenterHit, tInnerHit);
      };

      /// 4-hit:
      if (variableType == FilterID::deltapT) {
        return [ = ](const PointType & outerHit, const PointType & outerCenterHit,
        const PointType & innerCenterHit , const PointType & innerHit) -> double {
          prepareStuff4Hit(outerHit, outerCenterHit, innerCenterHit, innerHit);
          return m_fourHit.calcDeltapT();
        };
      }

      if (variableType == FilterID::deltaDistance2IP) {
        return [ = ](const PointType & outerHit, const PointType & outerCenterHit,
        const PointType & innerCenterHit , const PointType & innerHit) -> double {
          prepareStuff4Hit(outerHit, outerCenterHit, innerCenterHit, innerHit);
          return m_fourHit.calcDeltaDistCircleCenter();
        };
      }


      B2ERROR(" XHitFilterAdapter-4Hit: given name (raw/full): " << variableType <<
              "/" << FilterID::getTypeName(variableType) <<
              " is not known, returning dummy function with 0.0 as a result instead!");

      return [&](const PointType&, const PointType&, const PointType&, const PointType&) -> double { return 0.0; };
    }



  protected:

    TwoHitFilters m_twoHit; /**< contains all 2-hit-filters. */
    ThreeHitFilters m_threeHit; /**< contains all 3-hit-filters. */
    FourHitFilters m_fourHit; /**< contains all 4-hit-filters. */

    TVector3 m_virtualIP; /**< contains global coordinates of virtual interaction point. */
  };
}


