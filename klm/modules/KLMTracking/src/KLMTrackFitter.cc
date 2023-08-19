/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/modules/KLMTracking/KLMTrackFitter.h>

/* KLM headers. */
#include <klm/bklm/geometry/Module.h>
#include <klm/bklm/geometry/GeometryPar.h>
#include <klm/eklm/geometry/GeometryData.h>


/* Basf2 headers. */
#include <framework/logging/Logger.h>

/* CLHEP headers. */
#include <CLHEP/Matrix/DiagMatrix.h>
#include <CLHEP/Matrix/Matrix.h>

/* C++ headers. */
#include <cfloat>

using namespace CLHEP;
using namespace Belle2;
using namespace Belle2::KLM;

//! Hep3Vector indices
enum { VX = 0, VY = 1, VZ = 2 };

//! fit vector and  Error matrix indices
enum { AY = 0, BY = 1, AZ = 2, BZ = 3 };

//! a Matrix indices
enum { MY = 0, MZ = 1 };

//! Constructor
KLMTrackFitter::KLMTrackFitter():
// Description: "For the use of standalone KLM tracking, this module is the fitter component.";
  m_Valid(false),
  m_Good(false),
  m_Chi2(0.0),
  m_NumHit(0),
  m_GlobalPar(4, 0),
  m_GlobalErr(4, 0),
  m_GeoPar(nullptr)
{
}

//! Destructor
KLMTrackFitter::~KLMTrackFitter()
{
}
//TODO: Fix me to be E and B KLM (let's do this last)
//! do fit and returns chi square of the fit.
double KLMTrackFitter::fit(std::list<KLMHit2d* >& listHitSector)
{

  // We can only do fit if there are at least two hits
  if (listHitSector.size() < 2) {
    m_Valid = false;
    m_Good  = false;
    return (false);
  }

  HepVector     eta(2, 0);             //  ( a, b ) in y = a + bx, used as inbetween
  HepSymMatrix  error(2, 0);
  HepVector     gloEta(2, 0);          //  ( a, b ) in y = a + bx in global system
  HepSymMatrix  gloError(2, 0);
  m_Chi2 = 0;

  // Create temporary vector and matrix... so size can be set.
  HepVector     globalPar(4, 0);
  HepSymMatrix  globalErr(4, 0);

  m_Chi2  = fit1dTrack(listHitSector, eta, error, VY, VX);
  globalPar.sub(1, eta);
  globalErr.sub(1, error);

  //m_Chi2  += fit1dTrack(listHitSector, eta, error, VY, VZ);
  m_Chi2  += fit1dTrack(listHitSector, eta, error, VZ, VX);
  globalPar.sub(3, eta);
  globalErr.sub(3, error);


  m_Chi2 /= 2.0;

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



//TODO: Fix me to be E and B KLM (please check)
//! Distance from track to a hit calculated in the global system
double KLMTrackFitter::globalDistanceToHit(KLMHit2d* hit,
                                           double& error,
                                           double& sigma)
{

  if (!m_Valid) {
    error = DBL_MAX;
    sigma = DBL_MAX;
    return DBL_MAX;
  }

  //in global fit, we have two functions y = a + b*x and y = c + d*z
  double x_mea = hit->getPositionX();
  double y_mea = hit->getPositionY();
  double z_mea = hit->getPositionZ();

  //there is some subdetector dependence so let's define this first.
  double x_pre, y_pre, z_pre, dx, dy, dz;

  // Error is composed of four parts: error due to tracking;
  // // and error in hit, y, x  or y, z. // TODO: Get rid of me
  // and error in hit, y(x)  or z(x).
  HepMatrix  errors(2, 2, 0);    // Matrix for errors
  HepMatrix  A(2, 4, 0);         // Matrix for derivatives


  //defining quanitites for hit errors
  double hit_xErr = 0; double hit_yErr = 0; double hit_zErr = 0;
  HepMatrix globalHitErr(3, 3, 0);

  //HepMatrix  B(2, 2, 0);         // Matrix for derivatives

  // Derivatives of x (z) = y/b - a/b with respect to y.
  //B[ MY ][ AY ] = 1./m_GlobalPar[BY];
  //B[ MZ ][ BY ] = 1./m_GlobalPar[BZ] ;
  //double errors_b[0] = B[ MY ][ AY ]*globalHitErr[1][1];
  //double errors_b[1] = B[ MZ ][ BY ]*globalHitErr[1][1];

  // Derivatives of x (z) = y/b - a/b with respect to a and b.
  // TODO: FIX ME
  //A[ MY ][ AY ] = -1. / m_GlobalPar[BY];
  //A[ MY ][ BY ] = -1 * (y_mea - m_GlobalPar[ AY ]) / (m_GlobalPar[ BY ] * m_GlobalPar[ BY ]);
  //A[ MZ ][ AZ ] = -1. / m_GlobalPar[ BZ ];
  //A[ MZ ][ BZ ] = -1 * (y_mea - m_GlobalPar[ AZ ]) / (m_GlobalPar[ BZ ] * m_GlobalPar[ BZ ]);
  A[ MY ][ AY ] = 1. ;
  A[ MY ][ BY ] = x_mea;
  A[ MZ ][ AZ ] = 1;
  A[ MZ ][ BZ ] = x_mea;


  //error from trackPar is inclueded, error from y_mea is not included
  errors = A * m_GlobalErr * A.T();

  if (hit->getSubdetector() == KLMElementNumbers::c_BKLM) {
    // defining terms relating to distance
    // TODO: REMOVE ME
    //x_pre = (y_mea - m_GlobalPar[ AY ]) / m_GlobalPar[ BY ]; //y_mea has uncertainties actually
    //y_pre = y_mea; //by definition
    //z_pre = (y_mea - m_GlobalPar[ AZ ]) / m_GlobalPar[ BZ ];
    x_pre = x_mea; //by definition
    y_pre = m_GlobalPar[ AY ] + m_GlobalPar[ BY ] * x_mea;
    z_pre =  m_GlobalPar[ AZ ] + m_GlobalPar[ BZ ] * x_mea;

    const bklm::GeometryPar* bklmGeo = m_GeoPar->BarrelInstance();
    //here get the resolustion of a hit, repeated several times, ugly. should we store this in KLMHit2d object ?
    const bklm::Module* corMod = bklmGeo->findModule(hit->getSection(), hit->getSector(), hit->getLayer());
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

    Hep3Vector globalOrigin = corMod->getGlobalOrigin();
    double sinphi = globalOrigin[1] / globalOrigin.mag();
    double cosphi = globalOrigin[0] / globalOrigin.mag();

    hit_xErr = hit_localPhiErr * sinphi;
    hit_yErr = hit_localPhiErr * cosphi;
    hit_zErr = hit_localZErr;


    globalHitErr[0][0] = pow(hit_localPhiErr * sinphi, 2); //x
    globalHitErr[0][1] = (hit_localPhiErr * sinphi) * (hit_localPhiErr * cosphi);
    globalHitErr[0][2] = 0;
    globalHitErr[1][1] = pow(hit_localPhiErr * cosphi, 2);;
    globalHitErr[1][0] = (hit_localPhiErr * sinphi) * (hit_localPhiErr * cosphi);
    globalHitErr[1][2] = 0;
    globalHitErr[2][2] = pow(hit_localZErr, 2);
    globalHitErr[2][0] = 0;
    globalHitErr[2][1] = 0;


  } // end of BKLM portion

  else if (hit->getSubdetector() == KLMElementNumbers::c_EKLM) {
    // distance related (choose z variable)
    // z coordinate should correspond to where track passes through EKLM-plane
    //TODO: FIX ME
    //y_pre =  m_GlobalPar[ AZ ] +  m_GlobalPar[ BZ ] * z_pre;
    //x_pre = (y_pre - m_GlobalPar[ AY ]) / m_GlobalPar[ BY ];
    //z_pre = z_mea; //by design
    z_pre = z_mea;
    x_pre = (z_mea - m_GlobalPar[ AZ ]) / m_GlobalPar[ BZ ];
    y_pre =  m_GlobalPar[ AY ] +  m_GlobalPar[ BY ] * x_pre;

    const EKLM::GeometryData* eklmGeo = m_GeoPar->EndcapInstance();

    //EKLM GeometryData is needed for strip widths

    //here get the resolustion of a hit, repeated several times, ugly. should we store this in KLMHit2d object ?
    hit_xErr = (eklmGeo->getStripGeometry()->getWidth()) * (Unit::cm / CLHEP::cm) * (hit->getXStripMax() - hit->getXStripMin() + 1) /
               sqrt(
                 12);
    hit_yErr = (eklmGeo->getStripGeometry()->getWidth()) * (Unit::cm / CLHEP::cm) * (hit->getYStripMax() - hit->getYStripMin() + 1) /
               sqrt(
                 12);
    hit_zErr = 0.; //KLMHit2d is always centred on the boundary between the x/y planes with ~0 uncertainty

    globalHitErr[0][0] = pow(hit_xErr, 2); //x
    globalHitErr[0][1] = 0.;
    globalHitErr[0][2] = 0.;
    globalHitErr[1][1] = pow(hit_yErr, 2); //y
    globalHitErr[1][0] = 0.;
    globalHitErr[1][2] = 0.;
    globalHitErr[2][2] = pow(hit_zErr, 2);; //z
    globalHitErr[2][0] = 0.;
    globalHitErr[2][1] = 0.;


  } // end of EKLM section

  else
    B2FATAL("In KLMTrackFitter globalDistanceToHit, hit is neither from E/B-KLM.");

  dx = x_pre - x_mea;
  dy = y_pre - y_mea;
  dz = z_pre - z_mea;
  double distance = sqrt(dx * dx + dy * dy +  dz * dz);

  // now that we have globalHitErr, compute error
  error = sqrt(errors[ MY ][ MY ] +
               errors[ MZ ][ MZ ] +
               //errors_b[0] + errors_b[1] + //error of prediced point due to error of inPos (y_mea)
               globalHitErr[0][0] +
               globalHitErr[1][1] + //y_mea error is correlated to the error of predicted point, but we didn't consider that part in errors
               globalHitErr[2][2]); //we ignore y_mea error?


  if (error != 0.0) {
    sigma = distance / error;
  } else {
    sigma = DBL_MAX;
  }

  return (distance);
}


//! do fit in global system, handle tracks that go thrugh multi-sectors
double KLMTrackFitter::fit1dTrack(std::list< KLMHit2d* > hitList,
                                  HepVector&  eta,
                                  HepSymMatrix&  error,
                                  int depDir,    int indDir)
{
// Fit d = a + bi, where d is dependent direction and i is independent
// // in global system we assume y = a + b*x and y = c + d*z  different from local fit
// in global system we assume y = a + b*x and z = c + dx  different from local fit
  HepMatrix globalHitErr(3, 3, 0);

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


  int n = 0;
  for (std::list< KLMHit2d* >::iterator iHit = hitList.begin(); iHit != hitList.end(); ++iHit) {

    KLMHit2d* hit = *iHit;

    CLHEP::Hep3Vector globalPos(hit->getPositionX(),  hit->getPositionY(),  hit->getPositionZ());


    if (hit->getSubdetector() == KLMElementNumbers::c_BKLM) {
      //const Belle2::bklm::Module* refMod = m_GeoPar->findModule(hit->getSection(), hit->getSector(), 1);
      const bklm::GeometryPar* bklmGeo = m_GeoPar->BarrelInstance();
      const bklm::Module* corMod = bklmGeo->findModule(hit->getSection(), hit->getSector(), hit->getLayer());

      //localHitPos = refMod->globalToLocal(globalPos);
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

      Hep3Vector globalOrigin = corMod->getGlobalOrigin();
      double sinphi = globalOrigin[1] / globalOrigin.mag();
      double cosphi = globalOrigin[0] / globalOrigin.mag();

      globalHitErr[0][0] = pow(hit_localPhiErr * sinphi, 2); // x
      globalHitErr[0][1] = (hit_localPhiErr * sinphi) * (hit_localPhiErr * cosphi);
      globalHitErr[0][2] = 0.;
      globalHitErr[1][1] = pow(hit_localPhiErr * cosphi, 2); // y
      globalHitErr[1][0] = (hit_localPhiErr * sinphi) * (hit_localPhiErr * cosphi);
      globalHitErr[1][2] = 0.;
      globalHitErr[2][2] = pow(hit_localZErr, 2); // z
      globalHitErr[2][0] = 0.;
      globalHitErr[2][1] = 0.;

      switch (indDir) {

        case VX:
          indPos = globalPos.x();
          break;

        case VY:
          indPos = globalPos.y();
          break;

        case VZ:
          indPos = globalPos.z();
          break;

        default:
          B2DEBUG(20, "error in bklm in-TrackFit: illegal direction");

      }

      switch (depDir) {

        case VX:
          depPos = globalPos.x();
          break;

        case VY:
          depPos = globalPos.y();
          break;

        case VZ:
          depPos = globalPos.z();
          break;

        default:
          B2DEBUG(20, "error in bklm dep-TrackFit: illegal direction");

      }
    } //end of BKLM section

    else if (hit->getSubdetector() == KLMElementNumbers::c_EKLM) {
      const EKLM::GeometryData* eklmGeo = m_GeoPar->EndcapInstance();

      //here get the resolustion of a hit, repeated several times, ugly. should we store this in KLMHit2d object ?
      double hit_xErr = (eklmGeo->getStripGeometry()->getWidth()) * (Unit::cm / CLHEP::cm) * (hit->getXStripMax() - hit->getXStripMin() +
                        1) / sqrt(
                          12);
      double hit_yErr = (eklmGeo->getStripGeometry()->getWidth()) * (Unit::cm / CLHEP::cm) * (hit->getYStripMax() - hit->getYStripMin() +
                        1) / sqrt(
                          12);
      double hit_zErr = 0; //KLMHit2d is always centred on the boundary between the x/y planes with ~0 uncertainty

      B2DEBUG(20, "KLMTrackFitter" << " Width: " << eklmGeo->getStripGeometry()->getWidth() << " Vec_x: " << hit_xErr * sqrt(
                12) << " Vec_y: " << hit_yErr * sqrt(12));

      globalHitErr[0][0] = pow(hit_xErr, 2); //x
      globalHitErr[0][1] = 0.;
      globalHitErr[0][2] = 0.;
      globalHitErr[1][1] = pow(hit_yErr, 2); //y
      globalHitErr[1][0] = 0.;
      globalHitErr[1][2] = 0.;
      globalHitErr[2][2] = pow(hit_zErr, 2); //z
      globalHitErr[2][0] = 0.;
      globalHitErr[2][1] = 0.;



      switch (indDir) {

        case VX:
          indPos = globalPos.x();
          break;

        case VY:
          indPos = globalPos.y();
          break;

        case VZ:
          indPos = globalPos.z();
          break;

        default:
          B2DEBUG(20, "error in EKLM in-TrackFit: illegal direction");

      }

      switch (depDir) {

        case VX:
          depPos = globalPos.x();
          break;

        case VY:
          depPos = globalPos.y();
          break;

        case VZ:
          depPos = globalPos.z();
          break;

        default:
          B2DEBUG(20, "error in EKLM dep-TrackFit: illegal direction");

      }
    } //end of EKLM section

    A[ n ][ 0 ] = 1;
    A[ n ][ 1 ] = indPos;

    y[ n ] = depPos;

    double error_raw = globalHitErr[indDir][indDir] + globalHitErr[depDir][depDir];
    //double correlate_ = 2.0*globalHitErr[indDir][depDir]; //?
    //double weight= error_raw - correlate_;
    double weight = error_raw;
    if (weight > 0) {
      V_y_inverse[ n ][ n ] = 1.0 / weight;
    } else {
      V_y_inverse[ n ][ n ] = DBL_MAX;
    }
    ++n;//n is the index of measured points/hits
  } // end of loop


  //HepMatrix AT = A.T();
  //HepMatrix tmp = AT*V_y_inverse;
  //V_A_inverse = tmp*A;
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
