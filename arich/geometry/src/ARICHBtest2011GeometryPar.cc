/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj, Rok Pestotnik                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <arich/geometry/ARICHBtest2011GeometryPar.h>

#include <cmath>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <TGraph.h>

using namespace std;
using namespace boost;

namespace Belle2 {
  namespace arich {

    ARICHBtest2011GeometryPar* ARICHBtest2011GeometryPar::p_B4ARICHBtest2011GeometryParDB = 0;

    ARICHBtest2011GeometryPar* ARICHBtest2011GeometryPar::Instance()
    {
      if (!p_B4ARICHBtest2011GeometryParDB) {
        p_B4ARICHBtest2011GeometryParDB = new ARICHBtest2011GeometryPar();
      }
      return p_B4ARICHBtest2011GeometryParDB;
    }

    ARICHBtest2011GeometryPar::ARICHBtest2011GeometryPar()
    {
      clear();
    }

    ARICHBtest2011GeometryPar::~ARICHBtest2011GeometryPar()
    {
    }


    void ARICHBtest2011GeometryPar::setMwpc(ARICHTracking* m)
    {
      m_mwpc = m;

    }


    ARICHTracking* ARICHBtest2011GeometryPar::getMwpc()
    {
      return m_mwpc;
    }

    void ARICHBtest2011GeometryPar::setTrackingShift(TVector3& v)
    {
      m_trackingShift = v;
      B2INFO("ARICHBtest2011GeometryPar::setTrackingShift=" << m_trackingShift.x() << ","  << m_trackingShift.y() << ","  <<  m_trackingShift.z());

    }


    TVector3 ARICHBtest2011GeometryPar::getTrackingShift()
    {
      return m_trackingShift;
    }

    void ARICHBtest2011GeometryPar::setRotationCenter(TVector3& v)
    {
      m_rotationCenter = v;
      B2INFO("ARICHBtest2011GeometryPar::setRotationCenter=" << m_rotationCenter.x() << ","  << m_rotationCenter.y() << ","  <<  m_rotationCenter.z());

    }

    TVector3 ARICHBtest2011GeometryPar::getRotationCenter()
    {
      return m_rotationCenter;
    }

    void ARICHBtest2011GeometryPar::setFrameRotation(double v)
    {
      m_frameRotation = TRotation();
      m_frameRotation.RotateY(v);
      B2INFO("ARICHBtest2011GeometryPar::setFrameRotation=" << v);

    }

    TRotation ARICHBtest2011GeometryPar::getFrameRotation()
    {
      return m_frameRotation;
    }

    void ARICHBtest2011GeometryPar::setAverageAgel(bool v)
    {
      m_averageAgel = v;
    }

    bool ARICHBtest2011GeometryPar::getAverageAgel()
    {
      return m_averageAgel;
    }



    void ARICHBtest2011GeometryPar::clear(void)
    {



      m_init = false;
      m_simple = false;
      m_trackingShift = TVector3();
      m_frameRotation   = TRotation();
      m_rotationCenter  = TVector3();


    }


    void ARICHBtest2011GeometryPar::Print(void) const
    {

    }



  } // namespace arich
} // namespace Belle2
