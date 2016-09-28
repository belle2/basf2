/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at)     *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/sectorMap/filterFramework/SelectionVariable.h>
#include <math.h>

namespace Belle2 {

  /** This is the specialization for SpacePoints with returning floats, where value calculates the squared distance between two hits in 3D */
  template <typename PointType>
  class Distance3DSquared : public SelectionVariable< PointType, double > {
  public:

    /** calculates the squared distance between the hits (3D), returning unit: cm^2 for speed optimization */
    static double value(const PointType& outerHit, const PointType& innerHit)
    {
//    #include <iomanip>      // std::setprecision
//    B2INFO("dist3DSquareDEBUG: outer " << std::fixed << std::setprecision(18) << outerHit.X() << "/" << outerHit.Y() << "/" << outerHit.Z() << ": x/y/z, inner: " << innerHit.X() << "/" << innerHit.Y() << "/" << innerHit.Z() << ".   RESULT: " << (std::pow(outerHit.X()-innerHit.X(), 2) + std::pow(outerHit.Y()-innerHit.Y(), 2) + std::pow(outerHit.Z()-innerHit.Z(), 2)));
      return
        std::pow(outerHit.X() - innerHit.X() , 2) +
        std::pow(outerHit.Y() - innerHit.Y() , 2) +
        std::pow(outerHit.Z() - innerHit.Z() , 2) ;
    }
  };

}
