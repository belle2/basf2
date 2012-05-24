//
// $Id: TSvdBuilder.cc 9944 2006-11-29 07:36:07Z katayama $
//
// $Log$
// Revision 1.5  2003/12/25 12:03:35  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.4  2002/01/03 11:04:56  katayama
// HepGeom::Point3D<double> and other header files are cleaned
//
//

#include <cmath>

#include "tracking/modules/trasan/TSvdBuilder.h"
#include "tracking/modules/trasan/TSvdHit.h"


namespace Belle {

  unsigned
  TSvdBuilder::calTHelix3(double x, double y, double r, double q,
                          double& dRho, double& phi0, double& kappa)
  {
    double radius = q * std::fabs(r);
    if (radius == 0.)return 0;
    kappa = 222.376063 / radius;

    // pivot = (0,0,0)
    dRho = -radius + std::sqrt(x * x + y * y);
    double t_dRho = -radius - std::sqrt(x * x + y * y);
    if (std::fabs(t_dRho) < std::fabs(dRho))dRho = t_dRho;

    double cosPhi0 = x / (dRho + radius);
    double sinPhi0 = y / (dRho + radius);
    if (cosPhi0 >  1.)cosPhi0 =  1.;
    if (cosPhi0 < -1.)cosPhi0 = -1.;

    phi0 = std::acos(cosPhi0);
    if (sinPhi0 < 0.)phi0 = 2.*M_PI - phi0;

    return 1;
  }


  unsigned
  TSvdBuilder::calZTanL(double dRho, double phi0, double kappa,
                        double& dZ, double& tanL, double& chisq,
                        AList<TSvdHit> hits,
                        double ipC)
  {
    double radius = 222.376063 / kappa;
    HepGeom::Point3D<double>  xt(dRho * std::cos(phi0), dRho * std::sin(phi0), 0.);
    HepGeom::Point3D<double>  xc(xt.x() + radius * std::cos(phi0), xt.y() + radius * std::sin(phi0), 0.);
    HepGeom::Point3D<double>  v0 = xt - xc;

    double sum = 0.;
    double sumX = 0., sumY = 0., sumX2 = 0., sumXY = 0., sumY2 = 0.;
    for (unsigned i = 0; i < (unsigned) hits.length(); ++i) {
      HepGeom::Point3D<double>  xsvd(hits[i]->position().x(),
                                     hits[i]->position().y(),
                                     0.);
      HepGeom::Point3D<double>  v1 = xsvd - xc;
      double vCrs = v0.x() * v1.y() - v0.y() * v1.x();
      double vDot = v0.x() * v1.x() + v0.y() * v1.y();
      double dPhi = std::atan2(vCrs, vDot);

      double x = -radius * dPhi;
      double y = hits[i]->position().z();
      sumX  += x;
      sumY  += y;
      sumX2 += x * x;
      sumXY += x * y;
      sumY2 += y * y;
      sum += 1.0;
      /* std::cout << i << ": " << hits[i]->dssd()
      << " " << hits[i]->position() << std::endl;
      std::cout << x << ", " << y << std::endl; */
    }
    sum += ipC; // Ip Constraint if ipC != 0.
    double det = sum * sumX2 - sumX * sumX;
    if (det == 0. || sum == 2.)return false;
    // y=a*x+b
    tanL = (sumXY * sum - sumX * sumY) / det; //a
    dZ = (sumX2 * sumY - sumX * sumXY) / det; //b

    chisq = 0.;
    for (unsigned i = 0; i < (unsigned) hits.length(); ++i) {
      HepGeom::Point3D<double>  xsvd(hits[i]->position().x(),
                                     hits[i]->position().y(),
                                     0.);
      HepGeom::Point3D<double>  v1 = xsvd - xc;
      double vCrs = v0.x() * v1.y() - v0.y() * v1.x();
      double vDot = v0.x() * v1.x() + v0.y() * v1.y();
      double dPhi = std::atan2(vCrs, vDot);

      double x = -radius * dPhi;
      double y = hits[i]->position().z();

      double chi = y - tanL * x - dZ;
      double chi2 = chi * chi;

      chisq += chi2;
    }
    chisq /= sum - 2.;
    return true;
  }


  unsigned
  TSvdBuilder::calTHelix(double x, double y, double r, double q,
                         double& dRho, double& phi0, double& kappa,
                         double& dZ, double& tanL, double& chisq,
                         AList<TSvdHit> hits,
                         double ipC)
  {
    if (!calTHelix3(x, y, r, q, dRho, phi0, kappa))return 0;
    return calZTanL(dRho, phi0, kappa, dZ, tanL, chisq, hits, ipC);
  }

} // namespace Belle

