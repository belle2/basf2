/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at)     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingVXD/filterTools/SelectionVariableType.h>

using namespace Belle2;

/** setting static dictionary translating type -> string */
std::map<Belle2::SelectionVariableType::Type, std::string> Belle2::SelectionVariableType::s_fromTypeToString = {
  {Belle2::SelectionVariableType::UnknownType, std::string("UnknownType")},
  {Belle2::SelectionVariableType::SelectionVariable, std::string("SelectionVariable")},
  {Belle2::SelectionVariableType::Distance3DSquared, std::string("Distance3DSquared")},
  {Belle2::SelectionVariableType::Distance2DXYSquared, std::string("Distance2DXYSquared")},
  {Belle2::SelectionVariableType::Distance1DZ, std::string("Distance1DZ")},
  {Belle2::SelectionVariableType::SlopeRZ, std::string("SlopeRZ")},
  {Belle2::SelectionVariableType::Distance3DNormed, std::string("Distance3DNormed")},
  {Belle2::SelectionVariableType::Angle3DSimpleHighOccupancy, std::string("Angle3DSimpleHighOccupancy")},
  {Belle2::SelectionVariableType::Angle3DFullHighOccupancy, std::string("Angle3DFullHighOccupancy")},
  {Belle2::SelectionVariableType::CosAngleXYHighOccupancy, std::string("CosAngleXYHighOccupancy")},
  {Belle2::SelectionVariableType::AngleXYFullHighOccupancy, std::string("AngleXYFullHighOccupancy")},
  {Belle2::SelectionVariableType::AngleRZSimpleHighOccupancy, std::string("AngleRZSimpleHighOccupancy")},
  {Belle2::SelectionVariableType::AngleRZFullHighOccupancy, std::string("AngleRZFullHighOccupancy")},
  {Belle2::SelectionVariableType::CircleDist2IPHighOccupancy, std::string("CircleDist2IPHighOccupancy")},
  {Belle2::SelectionVariableType::DeltaSlopeRZHighOccupancy, std::string("DeltaSlopeRZHighOccupancy")},
  {Belle2::SelectionVariableType::DeltaSlopeZoverSHighOccupancy, std::string("DeltaSlopeZoverSHighOccupancy")},
  {Belle2::SelectionVariableType::DeltaSoverZHighOccupancy, std::string("DeltaSoverZHighOccupancy")},
  {Belle2::SelectionVariableType::HelixParameterFitHighOccupancy, std::string("HelixParameterFitHighOccupancy")},
  {Belle2::SelectionVariableType::PtHighOccupancy, std::string("PtHighOccupancy")},
  {Belle2::SelectionVariableType::CircleRadiusHighOccupancy, std::string("CircleRadiusHighOccupancy")},
  {Belle2::SelectionVariableType::Angle3DSimple, std::string("Angle3DSimple")},
  {Belle2::SelectionVariableType::Angle3DFull, std::string("Angle3DFull")},
  {Belle2::SelectionVariableType::CosAngleXY, std::string("CosAngleXY")},
  {Belle2::SelectionVariableType::AngleXYFull, std::string("AngleXYFull")},
  {Belle2::SelectionVariableType::AngleRZSimple, std::string("AngleRZSimple")},
  {Belle2::SelectionVariableType::AngleRZFull, std::string("AngleRZFull")},
  {Belle2::SelectionVariableType::CircleDist2IP, std::string("CircleDist2IP")},
  {Belle2::SelectionVariableType::DeltaSlopeRZ, std::string("DeltaSlopeRZ")},
  {Belle2::SelectionVariableType::DeltaSlopeZoverS, std::string("DeltaSlopeZoverS")},
  {Belle2::SelectionVariableType::DeltaSoverZ, std::string("DeltaSoverZ")},
  {Belle2::SelectionVariableType::HelixParameterFit, std::string("HelixParameterFit")},
  {Belle2::SelectionVariableType::Pt, std::string("Pt")},
  {Belle2::SelectionVariableType::CircleRadius, std::string("CircleRadius")},
  {Belle2::SelectionVariableType::DeltaPtHighOccupancy, std::string("DeltaPtHighOccupancy")},
  {Belle2::SelectionVariableType::DeltaDistCircleCenterHighOccupancy, std::string("DeltaDistCircleCenterHighOccupancy")},
  {Belle2::SelectionVariableType::DeltaDistCircleCenterHighOccupancy, std::string("DeltaCircleRadiusHighOccupancy")},
  {Belle2::SelectionVariableType::DeltaPt, std::string("DeltaPt")},
  {Belle2::SelectionVariableType::DeltaDistCircleCenter, std::string("DeltaDistCircleCenter")},
  {Belle2::SelectionVariableType::DeltaDistCircleCenter, std::string("DeltaCircleRadius")},
  {Belle2::SelectionVariableType::NTypes, std::string("NTypes")}
};


/** setting static dictionary translating string -> type */
std::map<std::string, Belle2::SelectionVariableType::Type> Belle2::SelectionVariableType::s_fromStringToType = {
  {std::string("UnknownType"), Belle2::SelectionVariableType::UnknownType},
  {std::string("SelectionVariable"), Belle2::SelectionVariableType::SelectionVariable},
  {std::string("Distance3DSquared"), Belle2::SelectionVariableType::Distance3DSquared},
  {std::string("Distance2DXYSquared"), Belle2::SelectionVariableType::Distance2DXYSquared},
  {std::string("Distance1DZ"), Belle2::SelectionVariableType::Distance1DZ},
  {std::string("SlopeRZ"), Belle2::SelectionVariableType::SlopeRZ},
  {std::string("Distance3DNormed"), Belle2::SelectionVariableType::Distance3DNormed},
  {std::string("Angle3DSimpleHighOccupancy"), Belle2::SelectionVariableType::Angle3DSimpleHighOccupancy},
  {std::string("Angle3DFullHighOccupancy"), Belle2::SelectionVariableType::Angle3DFullHighOccupancy},
  {std::string("CosAngleXYHighOccupancy"), Belle2::SelectionVariableType::CosAngleXYHighOccupancy},
  {std::string("AngleXYFullHighOccupancy"), Belle2::SelectionVariableType::AngleXYFullHighOccupancy},
  {std::string("AngleRZSimpleHighOccupancy"), Belle2::SelectionVariableType::AngleRZSimpleHighOccupancy},
  {std::string("AngleRZFullHighOccupancy"), Belle2::SelectionVariableType::AngleRZFullHighOccupancy},
  {std::string("CircleDist2IPHighOccupancy"), Belle2::SelectionVariableType::CircleDist2IPHighOccupancy},
  {std::string("DeltaSlopeRZHighOccupancy"), Belle2::SelectionVariableType::DeltaSlopeRZHighOccupancy},
  {std::string("DeltaSlopeZoverSHighOccupancy"), Belle2::SelectionVariableType::DeltaSlopeZoverSHighOccupancy},
  {std::string("DeltaSoverZHighOccupancy"), Belle2::SelectionVariableType::DeltaSoverZHighOccupancy},
  {std::string("HelixParameterFitHighOccupancy"), Belle2::SelectionVariableType::HelixParameterFitHighOccupancy},
  {std::string("PtHighOccupancy"), Belle2::SelectionVariableType::PtHighOccupancy},
  {std::string("CircleRadiusHighOccupancy"), Belle2::SelectionVariableType::CircleRadiusHighOccupancy},
  {std::string("Angle3DSimple"), Belle2::SelectionVariableType::Angle3DSimple},
  {std::string("Angle3DFull"), Belle2::SelectionVariableType::Angle3DFull},
  {std::string("CosAngleXY"), Belle2::SelectionVariableType::CosAngleXY},
  {std::string("AngleXYFull"), Belle2::SelectionVariableType::AngleXYFull},
  {std::string("AngleRZSimple"), Belle2::SelectionVariableType::AngleRZSimple},
  {std::string("AngleRZFull"), Belle2::SelectionVariableType::AngleRZFull},
  {std::string("CircleDist2IP"), Belle2::SelectionVariableType::CircleDist2IP},
  {std::string("DeltaSlopeRZ"), Belle2::SelectionVariableType::DeltaSlopeRZ},
  {std::string("DeltaSlopeZoverS"), Belle2::SelectionVariableType::DeltaSlopeZoverS},
  {std::string("DeltaSoverZ"), Belle2::SelectionVariableType::DeltaSoverZ},
  {std::string("HelixParameterFit"), Belle2::SelectionVariableType::HelixParameterFit},
  {std::string("Pt"), Belle2::SelectionVariableType::Pt},
  {std::string("CircleRadius"), Belle2::SelectionVariableType::CircleRadius},
  {std::string("DeltaPtHighOccupancy"), Belle2::SelectionVariableType::DeltaPtHighOccupancy},
  {std::string("DeltaDistCircleCenterHighOccupancy"), Belle2::SelectionVariableType::DeltaDistCircleCenterHighOccupancy},
  {std::string("DeltaCircleRadiusHighOccupancy"), Belle2::SelectionVariableType::DeltaCircleRadiusHighOccupancy},
  {std::string("DeltaPt"), Belle2::SelectionVariableType::DeltaPt},
  {std::string("DeltaDistCircleCenter"), Belle2::SelectionVariableType::DeltaDistCircleCenter},
  {std::string("DeltaCircleRadius"), Belle2::SelectionVariableType::DeltaCircleRadius},
  {std::string("NTypes"), Belle2::SelectionVariableType::NTypes}
};
