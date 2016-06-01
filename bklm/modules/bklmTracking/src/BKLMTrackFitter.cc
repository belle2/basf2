/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui GUAN                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cfloat>

#include "bklm/modules/bklmTracking/BKLMTrackFitter.h"
#include <bklm/geometry/GeometryPar.h>
#include <bklm/geometry/Module.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace CLHEP;
using namespace Belle2;
using namespace Belle2::bklm;

//! Hep3Vector indices
enum { VX = 0, VY = 1, VZ = 2 };

//! fit vector and  Error matrix indices
enum { AY = 0, BY = 1, AZ = 2, BZ = 3 };

//! a Matrix indices
enum { MY = 0, MZ = 1 };

//! Constructor
BKLMTrackFitter::BKLMTrackFitter():
  m_Valid(false),
  m_Good(false),
  m_Chi2(0.0),
  m_NumHit(0),
  m_GeoPar(NULL)
{
  HepVector     iniPar(4, 0);
  HepSymMatrix  iniErr(4, 0);
  m_GlobalPar = iniPar;
  m_SectorPar = iniPar;
  m_GlobalErr = iniErr;
  m_SectorErr = iniErr;
}

//! Destructor
BKLMTrackFitter::~BKLMTrackFitter()
{
}

//! do fit and returns chi square of the fit.
double BKLMTrackFitter::fit(std::list<BKLMHit2d* >& listHitSector)
{

  // We can only do fit if there are at least two hits
  if (listHitSector.size() < 2) {
    m_Valid = false;
    m_Good  = false;
    return (false);
  }

  HepVector     eta(2, 0);             //  ( a, b ) in y = a + bx
  HepSymMatrix  error(2, 0);
  HepVector     gloEta(2, 0);          //  ( a, b ) in y = a + bx in global system
  HepSymMatrix  gloError(2, 0);
  m_Chi2 = 0;

  // Create temporary vector and matrix... so size can be set.
  HepVector     sectorPar(4, 0);
  HepSymMatrix  sectorErr(4, 0);
  HepVector     globalPar(4, 0);
  HepSymMatrix  globalErr(4, 0);

  m_Chi2  = fit1dSectorTrack(listHitSector, eta, error, VY, VX);
  sectorPar.sub(1, eta);
  sectorErr.sub(1, error);

  m_Chi2 += fit1dSectorTrack(listHitSector, eta, error, VZ, VX);
  sectorPar.sub(3, eta);
  sectorErr.sub(3, error);

  //transfer to the global system, choose two abitrary points on track within in the sector jpionts on this track
  const Belle2::bklm::Module* refMod = m_GeoPar->findModule((*listHitSector.begin())->isForward(),
                                                            (*listHitSector.begin())->getSector(), 1);

  Hep3Vector p1(0, 0, 0); Hep3Vector p2(0, 0, 0);
  double x1 = 5; // sector localX
  double x2 = 10;
  double y1 = sectorPar[0] + sectorPar[1] * x1;
  double y2 = sectorPar[0] + sectorPar[1] * x2;
  double z1 = sectorPar[2] + sectorPar[3] * x1;
  double z2 = sectorPar[2] + sectorPar[3] * x2;
  p1.setX(x1); p1.setY(y1); p1.setZ(z1);
  p2.setX(x2); p2.setY(y2); p2.setZ(z2);
  Hep3Vector gl1 = refMod->localToGlobal(p1);
  Hep3Vector gl2 = refMod->localToGlobal(p2);

  //transfer the trackParameters to global system
  if (gl2[0] != gl1[0]) {
    globalPar[1] = (gl2[1] - gl1[1]) / (gl2[0] - gl1[0]);
    globalPar[0] = gl1[1] - globalPar[1] * gl1[0];
    globalPar[3] = (gl2[2] - gl1[2]) / (gl2[0] - gl1[0]);
    globalPar[2] = gl1[2] - globalPar[3] * gl1[0];
    globalErr = sectorErr;
  } else {
    globalPar[1] = DBL_MAX;
    globalPar[0] = DBL_MAX;
    globalPar[3] = DBL_MAX;
    globalPar[2] = DBL_MAX;
    globalErr = sectorErr; //?
  }

  m_Chi2 /= 2.0;

  m_SectorPar = sectorPar;
  m_SectorErr = sectorErr;
  m_GlobalPar = globalPar;
  m_GlobalErr = globalErr;

  m_Valid = true;
  m_Good  = false;
  m_NumHit =  listHitSector.size();
  if (m_Chi2 <= 20.0) {
    m_Good = true;
  }

  return (m_Chi2);

}

//! Distance from track to a hit in the plane of the module
double BKLMTrackFitter::distanceToHit(BKLMHit2d* hit,
                                      double& error,
                                      double& sigma)
{

  double x, y, z, dy, dz;

  if (!m_Valid) {
    error = DBL_MAX;
    sigma = DBL_MAX;
    return DBL_MAX;
  }

  m_GeoPar = GeometryPar::instance();
  const Belle2::bklm::Module* refMod = m_GeoPar->findModule(hit->isForward(), hit->getSector(), 1);
  const Belle2::bklm::Module* corMod = m_GeoPar->findModule(hit->isForward(), hit->getSector(), hit->getLayer());

  CLHEP::Hep3Vector globalPos(hit->getGlobalPosition()[0], hit->getGlobalPosition()[1], hit->getGlobalPosition()[2]);

  //+++ local coordinates of the hit
  CLHEP::Hep3Vector local = refMod->globalToLocal(globalPos);

  x = local[0] ;

  y = m_SectorPar[ AY ] + x * m_SectorPar[ BY ];
  z = m_SectorPar[ AZ ] + x * m_SectorPar[ BZ ];

  dy = y - local[1];
  dz = z - local[2];

  double distance = sqrt(dy * dy + dz * dz);

  // Error is composed of four parts: error due to tracking, y and z;
  // and error in hit, y and z. We know the latter two, got to find
  // the first two. We could calculate this from simple equations or
  // using matrices. I choose the later because it is extendable.
  HepMatrix  errors(2, 2, 0);    // Matrix for errors
  HepMatrix  A(2, 4, 0);         // Matrix for derivatives

  // Derivatives of y (z) = a + bx with respect to a and b.
  A[ MY ][ AY ] = 1.0;
  A[ MY ][ BY ] =   x;
  A[ MZ ][ AZ ] = 1.0;
  A[ MZ ][ BZ ] =   x;

  errors = A * m_SectorErr * A.T();

  double hit_localPhiErr = corMod->getPhiStripWidth() / sqrt(12);
  double hit_localZErr = corMod->getZStripWidth() / sqrt(12);

  if (hit->inRPC()) {
    //+++ scale localErr based on measured-in-Belle resolution
    int nStrips = hit->getPhiStripMax() - hit->getPhiStripMin() + 1;
    double dn = nStrips - 1.5;
    double factor = std::pow((0.9 + 0.4 * dn * dn), 1.5) * 0.60;//measured-in-Belle resolution
    hit_localPhiErr = hit_localPhiErr * sqrt(factor);

    nStrips = hit->getZStripMax() - hit->getZStripMin() + 1;
    dn = nStrips - 1.5;
    factor = std::pow((0.9 + 0.4 * dn * dn), 1.5) * 0.55;//measured-in-Belle resolution
    hit_localZErr = hit_localZErr * sqrt(factor);
  }

  error = sqrt(errors[ MY ][ MY ] +
               errors[ MZ ][ MZ ] +
               pow(hit_localPhiErr, 2) +
               pow(hit_localZErr, 2));

  if (error != 0.0) {
    sigma = distance / error;
  } else {
    sigma = DBL_MAX;
  }

  return (distance);
}

//! do fit in the y-x plane or z-x plane
double BKLMTrackFitter::fit1dSectorTrack(std::list< BKLMHit2d* > hitList,
                                         HepVector&  eta,
                                         HepSymMatrix&  error,
                                         int depDir,    int indDir)
{

// Fit d = a + bi, where d is dependent direction and i is independent

  std::list< BKLMHit2d* >::iterator s;

  Hep3Vector localHitPos;
  HepMatrix  localHitErr(3, 3, 0);

  double     indPos = 0;
  double     depPos = 0;

  // Matrix based solution

  int noPoints = hitList.size();

  // Derivative of y = a + bx, with respect to a and b evaluated at x.
  HepMatrix  A(noPoints, 2, 0);

  // Measured data points.
  HepVector  y(noPoints, 0);

  // Inverse of covariance (error) matrix, also known as the weight matrix.
  // In plain English: V_y_inverse_nn = 1 / (error of nth measurement)^2
  HepDiagMatrix V_y_inverse(noPoints, 0);

  // Error or correlation matrix for coefficients (2x2 matrices)
  HepSymMatrix  V_A, V_A_inverse;

  s = hitList.begin();
  BKLMHit2d* hit = *s;
  bool isForward = hit->isForward();
  int sector    = hit->getSector();

  m_GeoPar = GeometryPar::instance();
  const Belle2::bklm::Module* refMod = m_GeoPar->findModule((*hitList.begin())->isForward(), (*hitList.begin())->getSector(), 1);

  int n = 0;
  for (s = hitList.begin(); s != hitList.end(); ++s) {

    hit = *s;
    if (hit->isForward() != isForward || hit->getSector() != sector) {
      continue;
    }

    // m_GeoPar = GeometryPar::instance();
    //const Belle2::bklm::Module* refMod = m_GeoPar->findModule(hit->isForward(), hit->getSector(), 1);
    const Belle2::bklm::Module* corMod = m_GeoPar->findModule(hit->isForward(), hit->getSector(), hit->getLayer());

    CLHEP::Hep3Vector globalPos;
    globalPos[0] = hit->getGlobalPosition()[0];
    globalPos[1] = hit->getGlobalPosition()[1];
    globalPos[2] = hit->getGlobalPosition()[2];

    localHitPos = refMod->globalToLocal(globalPos);
    double hit_localPhiErr = corMod->getPhiStripWidth() / sqrt(12);
    double hit_localZErr = corMod->getZStripWidth() / sqrt(12);

    if (hit->inRPC()) {
      //+++ scale localErr based on measured-in-Belle resolution
      int nStrips = hit->getPhiStripMax() - hit->getPhiStripMin() + 1;
      double dn = nStrips - 1.5;
      double factor = std::pow((0.9 + 0.4 * dn * dn), 1.5) * 0.60;//measured-in-Belle resolution
      hit_localPhiErr = hit_localPhiErr * sqrt(factor);

      nStrips = hit->getZStripMax() - hit->getZStripMin() + 1;
      dn = nStrips - 1.5;
      factor = std::pow((0.9 + 0.4 * dn * dn), 1.5) * 0.55;//measured-in-Belle resolution
      hit_localZErr = hit_localZErr * sqrt(factor);
    }

    localHitErr[0][0] = 0.0; //x
    localHitErr[0][1] = 0;
    localHitErr[0][2] = 0;
    localHitErr[1][1] = hit_localPhiErr;
    localHitErr[1][0] = 0;
    localHitErr[1][2] = 0;
    localHitErr[2][2] = hit_localZErr;
    localHitErr[2][0] = 0;
    localHitErr[2][1] = 0;

    switch (indDir) {

      case VX:
        indPos = localHitPos.x();
        break;

      case VY:
        indPos = localHitPos.y();
        break;

      case VZ:
        indPos = localHitPos.z();
        break;

      default:
        B2DEBUG(1, "error in klm_trackSectorFit: illegal direction");

    }

    switch (depDir) {

      case VX:
        depPos = localHitPos.x();
        break;

      case VY:
        depPos = localHitPos.y();
        break;

      case VZ:
        depPos = localHitPos.z();
        break;

      default:
        B2DEBUG(1, "error in klm_trackSectorFit: illegal direction");

    }


    A[ n ][ 0 ] = 1;
    A[ n ][ 1 ] = indPos;

    y[ n ] = depPos;

    if (localHitErr[ depDir ][ depDir ] > 0.0) {
      V_y_inverse[ n ][ n ] = 1.0 / localHitErr[ depDir ][ depDir ];
    } else {
      V_y_inverse[ n ][ n ] = DBL_MAX;
    }
    ++n;//n is the index of measured points/hits
  }

  V_A_inverse = V_y_inverse.similarityT(A);

  int ierr = 0;
  V_A = V_A_inverse.inverse(ierr);

  eta   = V_A * A.T() * V_y_inverse * y;
  error = V_A;

// Calculate residuals:
  HepMatrix residual = y - A * eta;
  HepMatrix chisqr = residual.T() * V_y_inverse * residual;

  return (chisqr.trace());

}
