/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <arich/dbobjects/ARICHReconstructionPar.h>

#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <iostream>
#include <iomanip>

using namespace std;
using namespace Belle2;


void ARICHReconstructionPar::initializeDefault()
{

  double parsBkg[8] = {1, 0, 0.0062, 5.0, 0.10364, 20.0596, 0.002103, -0.0031283};
  m_bkgPDF = new TF1("bkgPDF",
                     "((1-[1])*[2]*exp(-[3]*x) + [1]*([4]*exp(-[5]*x)+[6]+[7]*x))*(1-1/1.5/1.5/[0]/[0])*TMath::Floor(1.5*[0])");
  m_bkgPDF->SetParameters(parsBkg);

  double parsRes[4] = {1.12147e-01, 3.81701e+00, 0.0093, -6.14984e-04};
  m_thcResolution = new TF1("thcResolution", "[0]*exp(-[1]*x)+[2]+[3]*x");
  m_thcResolution->SetParameters(parsRes);

  m_pars = {0.18, 0.13};
  m_flatBkgPerPad = 0.0014;

  m_aerogelFOM = {11.3, 13.0};
}

double ARICHReconstructionPar::getBackgroundPerPad(double th_cer, std::vector<double>& pars)
{

  int ipar = 0;
  for (auto par : pars) {
    m_bkgPDF->SetParameter(ipar, par);
    ipar++;
  }
  return m_bkgPDF->Eval(th_cer) + m_flatBkgPerPad;
}

double ARICHReconstructionPar::getExpectedBackgroundHits(std::vector<double>& pars, double minThc, double maxThc)
{

  int ipar = 0;
  for (auto par : pars) {
    m_bkgPDF->SetParameter(ipar, par);
    ipar++;
  }

  // parameters of fit of  pol3 to the number of nPads per ring with given theta and width 5mrad
  double surf[4] = { -2.19669e-02, 3.59010e+01, -2.77441e+01, 1.43564e+02};
  double step = 0.005;
  double thc = minThc + step / 2.;
  double bkg = 0;
  while (thc < maxThc) {
    bkg += (surf[0] + surf[1] * thc + surf[2] * pow(thc, 2) + surf[3] * pow(thc, 3)) * getBackgroundPerPad(thc, pars);
    thc += step;
  }
  return bkg;

}

double ARICHReconstructionPar::getNPadsInRing(double maxThc, double minThc, double trackTh)
{

  double s1 = sqrt(tan(maxThc)) * pow((tan(maxThc + trackTh) + tan(maxThc - trackTh)) / 2, 3. / 2.);
  double s2 = 0;
  if (minThc > 0) s2 = sqrt(tan(minThc)) * pow((tan(minThc + trackTh) + tan(minThc - trackTh)) / 2, 3. / 2.);
  return 3.1416 * 0.18 * 0.18 * (s1 - s2) * 0.6 / 0.005 / 0.005; // pi*dist^2*s*avg_geo_acc/pad_size
}

void ARICHReconstructionPar::print() const
{
}
