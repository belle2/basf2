/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <arich/dbobjects/ARICHReconstructionPar.h>

#include <iostream>
#include <vector>

using namespace std;
using namespace Belle2;


void ARICHReconstructionPar::initializeDefault()
{

  double parsBkg[10] = {0, 1, 8.764830e+00, -4.815370e+00, 3.268570e-06, 8.518560e+00, -9.609710e+00, 9.769060e-06, 1.095350e-02, -1.322410e-02};
  m_bkgPDF = new TF1("bkgPDF",
                     "0*[0] + (1-[1])*(exp([2]+[3]*x)*[4]) + [1]*(exp([5]+[6]*x)*[7]+x*[8]+x*x*[9])");
  m_bkgPDF->SetParameters(parsBkg);

  double parsRes[4] = {1.12147e-01, 3.81701e+00, 0.0093, -6.14984e-04};
  m_thcResolution = new TF1("thcResolution", "([p0]*exp(-[p1]*x)+[p2]+[p3]*x-((x>3)?[p3]*(x-3):0))*1.35");
  m_thcResolution->SetParameters(parsRes);

  double parsBkgPhi[8] = {0, 0, 6.68258e-02, 6.50048e+00, -1.04876e-01, -7.06652e-03, 1.34111e-01, 1.83823e-01};
  m_bkgPhiPDF = new TF2("m_bkgPhiPDF",
                        "[0]*0 + [1]*0 + 1/( ([2]*exp(y*[3]) + [4])*cos(x/2)*cos(x/2)*cos(x/2)*cos(x/2) + sqrt([5] + [6]*y + [7]*y*y ) )");
  m_bkgPhiPDF->SetParameters(parsBkgPhi);

  m_pars = {0.098304138, 5.973590e-02, 21, 0.88, 3.7, 0.357, 0.46, 1, 0.12, 0.12, 0.02, 1.5, 0.0288};
  m_flatBkgPerPad = 0.000892;

  m_aerogelFOM = {13.200797, 15.186757};
}

double ARICHReconstructionPar::getBackgroundPerPad(double th_cer, const std::vector<double>& pars) const
{

  int ipar = 0;
  for (auto par : pars) {
    m_bkgPDF->SetParameter(ipar, par);
    ipar++;
  }
  return m_bkgPDF->Eval(th_cer) + m_flatBkgPerPad;
}

double ARICHReconstructionPar::getPhiCorrectedBackgroundPerPad(double fi_cer_trk, double th_cer,
    const std::vector<double>& pars) const
{

  int ipar = 0;
  for (auto par : pars) {
    m_bkgPhiPDF->SetParameter(ipar, par);
    ipar++;
  }
  return m_flatBkgPerPad * m_bkgPhiPDF->Eval(fi_cer_trk, th_cer);
}

double ARICHReconstructionPar::getExpectedBackgroundHits(const std::vector<double>& pars, double minThc, double maxThc) const
{

  int ipar = 0;
  for (auto par : pars) {
    m_bkgPDF->SetParameter(ipar, par);
    ipar++;
  }

  // parameters of fit of  pol3 to the number of nPads per ring with given theta and width 5mrad
  const double surf[4] = { -2.19669e-02, 3.59010e+01, -2.77441e+01, 1.43564e+02};
  double step = 0.005;
  double thc = minThc + step / 2.;
  double bkg = 0;
  while (thc < maxThc) {
    bkg += (surf[0] + surf[1] * thc + surf[2] * pow(thc, 2) + surf[3] * pow(thc, 3)) * getBackgroundPerPad(thc, pars);
    thc += step;
  }
  return bkg;

}

double ARICHReconstructionPar::getNPadsInRing(double maxThc, double minThc, double trackTh) const
{

  double s1 = sqrt(tan(maxThc)) * pow((tan(maxThc + trackTh) + tan(maxThc - trackTh)) / 2, 3. / 2.);
  double s2 = 0;
  if (minThc > 0) s2 = sqrt(tan(minThc)) * pow((tan(minThc + trackTh) + tan(minThc - trackTh)) / 2, 3. / 2.);
  return 3.1416 * 0.18 * 0.18 * (s1 - s2) * 0.6 / 0.005 / 0.005; // pi*dist^2*s*avg_geo_acc/pad_size
}

void ARICHReconstructionPar::print() const
{

  cout << endl << "-----bkg PDF-----" << endl;
  m_bkgPDF->Print();
  int Npar = m_bkgPDF->GetNpar();
  std::vector<double> bkgPars(Npar, 0);
  m_bkgPDF->GetParameters(bkgPars.data());
  for (int i = 0; i < Npar; i++)cout << Form("bkg Pars %d = %e", i, bkgPars[i]) << endl;

  cout << endl << "-----bkg phi corr PDF-----" << endl;
  m_bkgPhiPDF->Print();
  int Npar2 = m_bkgPhiPDF->GetNpar();
  std::vector<double> bkgPhiCorPars(Npar2, 0);
  m_bkgPhiPDF->GetParameters(bkgPhiCorPars.data());
  for (int i = 0; i < Npar2; i++)cout << Form("bkg Pars %d = %e", i, bkgPhiCorPars[i]) << endl;

  cout << endl << "-----flat backgroud per pad-----"  << endl;
  cout << " flat background per pad is " << m_flatBkgPerPad << endl;

  cout << endl << "----additional parameters (for wide gaus, quartz etc.)-----"  << endl;
  for (int i = 0; i < (int)m_pars.size(); i++) {
    printf("m_pars[%d] = %e\n", i, m_pars[i]);
  }

  cout << endl << "----Aerogel FOM-----"  << endl;
  for (int i = 0; i < (int)m_aerogelFOM.size(); i++) {
    printf("m_aerogelFOM[%d] = %f\n", i, m_aerogelFOM[i]);
  }

  cout << endl << "----- Resolution PDF-----" << endl;
  m_thcResolution->Print();
  double resPars[4];
  m_thcResolution->GetParameters(resPars);
  for (int i = 0; i < 4; i++)cout << Form("resolution Pars %d = %e", i, resPars[i]) << endl;

}

