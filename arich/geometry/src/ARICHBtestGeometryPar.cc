/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/logging/Logger.h>

#include <arich/geometry/ARICHBtestGeometryPar.h>

#include <boost/format.hpp>
#include <boost/foreach.hpp>

using namespace std;
using namespace boost;

namespace Belle2 {

  ARICHBtestGeometryPar* ARICHBtestGeometryPar::p_B4ARICHBtestGeometryParDB = 0;

  ARICHBtestGeometryPar* ARICHBtestGeometryPar::Instance()
  {
    if (!p_B4ARICHBtestGeometryParDB) {
      p_B4ARICHBtestGeometryParDB = new ARICHBtestGeometryPar();
    }
    return p_B4ARICHBtestGeometryParDB;
  }

  ARICHBtestGeometryPar::ARICHBtestGeometryPar():
    m_mwpc(new ARICHTracking),
    m_trackingShift(TVector3()),
    m_rotationCenter(TVector3()),
    m_frameRotation(TRotation()),
    m_averageAgel(0),
    m_nPads(0),
    m_init(0),
    m_simple(0)
  {
    clear();
  }


  ARICHBtestGeometryPar::ARICHBtestGeometryPar(const ARICHBtestGeometryPar& arichPar) :
    m_trackingShift(arichPar.m_trackingShift),
    m_rotationCenter(arichPar.m_rotationCenter),
    m_frameRotation(arichPar.m_frameRotation),
    m_hapdmap(arichPar.m_hapdmap),
    m_hapdeid(arichPar.m_hapdeid)
  {
    m_averageAgel = arichPar.m_averageAgel;
    m_nPads = arichPar.m_nPads;
    m_init = arichPar.m_init;
    m_simple = arichPar.m_simple;
    m_mwpc = new ARICHTracking();
    *m_mwpc = *arichPar.m_mwpc;
    p_B4ARICHBtestGeometryParDB = this;
  }

  ARICHBtestGeometryPar::~ARICHBtestGeometryPar()
  {
  }


  void ARICHBtestGeometryPar::setMwpc(ARICHTracking* m)
  {
    m_mwpc = m;
  }


  ARICHTracking* ARICHBtestGeometryPar::getMwpc()
  {
    return m_mwpc;
  }

  std::pair<double, double> ARICHBtestGeometryPar::GetHapdChannelPosition(int id)
  {
    return m_hapdmap[id];
  }

  std::pair<int, int> ARICHBtestGeometryPar::GetHapdElectronicMap(int id)
  {
    return m_hapdeid[id];
  }

  int ARICHBtestGeometryPar::AddHapdChannelPositionPair(double x , double y)
  {
    m_hapdmap.push_back(std::make_pair(x, y));
    return m_hapdmap.size();
  }

  int ARICHBtestGeometryPar::AddHapdElectronicMapPair(int x, int y)
  {
    m_hapdeid.push_back(std::make_pair(x, y));
    return m_hapdeid.size();
  }

  void ARICHBtestGeometryPar::setTrackingShift(const TVector3& v)
  {
    m_trackingShift = v;
    B2INFO("ARICHBtestGeometryPar::setTrackingShift=" << m_trackingShift.x() << ","  << m_trackingShift.y() << ","  <<
           m_trackingShift.z());

  }


  TVector3 ARICHBtestGeometryPar::getTrackingShift()
  {
    return m_trackingShift;
  }

  void ARICHBtestGeometryPar::setRotationCenter(const TVector3& v)
  {
    m_rotationCenter = v;
    B2INFO("ARICHBtestGeometryPar::setRotationCenter=" << m_rotationCenter.x() << ","  << m_rotationCenter.y() << ","  <<
           m_rotationCenter.z());

  }

  TVector3 ARICHBtestGeometryPar::getRotationCenter()
  {
    return m_rotationCenter;
  }

  void ARICHBtestGeometryPar::setFrameRotation(double v)
  {
    m_frameRotation = TRotation();
    m_frameRotation.RotateY(v);
    B2INFO("ARICHBtestGeometryPar::setFrameRotation=" << v);

  }

  TRotation ARICHBtestGeometryPar::getFrameRotation()
  {
    return m_frameRotation;
  }

  void ARICHBtestGeometryPar::setAverageAgel(bool v)
  {
    m_averageAgel = v;
  }

  bool ARICHBtestGeometryPar::getAverageAgel()
  {
    return m_averageAgel;
  }



  void ARICHBtestGeometryPar::clear(void)
  {
    m_init = false;
    m_simple = false;
    m_trackingShift = TVector3();
    m_frameRotation   = TRotation();
    m_rotationCenter  = TVector3();
  }

  void ARICHBtestGeometryPar::setOffset(const TVector3& v)
  {
    m_offset = v;
    B2INFO("ARICHBtestGeometryPar::setOffset=" << m_offset.x() << ","  << m_offset.y() << ","  <<  m_offset.z());

  }

  TVector3 ARICHBtestGeometryPar::getOffset()
  {
    return m_offset;
  }


  void ARICHBtestGeometryPar::Print(void) const
  {

  }

} // namespace Belle2
