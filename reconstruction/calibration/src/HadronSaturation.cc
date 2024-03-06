/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/calibration/HadronSaturation.h>
using namespace Belle2;

static HadronSaturation* HC_obj;

HadronSaturation::HadronSaturation() :
  m_flag(0),
  m_cosbins(8),
  m_alpha(0.044286100),
  m_gamma(0.0019308200),
  m_delta(0.020),
  m_power(1.3205300),
  m_ratio(1.0000)
{
  m_dedx.clear();
  m_dedxerror.clear();
  m_betagamma.clear();
  m_costheta.clear();
  HC_obj = this;
}

HadronSaturation::HadronSaturation(double alpha, double gamma, double delta, double power, double ratio) :
  m_flag(0),
  m_cosbins(8),
  m_alpha(alpha),
  m_gamma(gamma),
  m_delta(delta),
  m_power(power),
  m_ratio(ratio)
{
  m_dedx.clear();
  m_dedxerror.clear();
  m_betagamma.clear();
  m_costheta.clear();
}

void
HadronSaturation::setParameters(double par[])
{

  m_alpha = par[0];
  m_gamma = par[1];
  m_delta = par[2];
  m_power = par[3];
  m_ratio = par[4];
  std::cout << "\t" << std::string(100, '-') << std::endl;
  std::cout << "\t HadronSaturation: parameters detail " << std::endl;
  std::cout << "\t 1. m_alpha ---> \t" << m_alpha << std::endl;
  std::cout << "\t 2. m_gamma ---> \t" << m_gamma << std::endl;
  std::cout << "\t 3. m_delta ---> \t" << m_delta << std::endl;
  std::cout << "\t 4. m_power ---> \t" << m_power << std::endl;
  std::cout << "\t 5. m_ratio ---> \t" << m_ratio << std::endl;
  std::cout << "\t" << std::string(100, '-') << std::endl;

}

void
HadronSaturation::setParameters()
{

  if (!m_DBHadronCor || m_DBHadronCor->getSize() == 0) {
    B2WARNING("No hadron correction parameters!");
    std::cout << "\t HadronSaturation: parameters detail " << std::endl;
    std::cout << "\t 1. m_alpha ---> \t" << m_alpha << std::endl;
    std::cout << "\t 2. m_gamma ---> \t" << m_gamma << std::endl;
    std::cout << "\t 3. m_delta ---> \t" << m_delta << std::endl;
    std::cout << "\t 4. m_power ---> \t" << m_power << std::endl;
    std::cout << "\t 5. m_ratio ---> \t" << m_ratio << std::endl;
  } else {
    for (int i = 0; i < 5; ++i) {
      m_alpha = 0.165016;
      m_gamma = 0.02078;
      m_delta = 0.02;
      m_power = 1.18098;
      m_ratio = 1;
      // m_alpha = m_DBHadronCor->getHadronPar(0);
      // m_gamma = m_DBHadronCor->getHadronPar(1);
      // m_delta = m_DBHadronCor->getHadronPar(2);
      // m_power = m_DBHadronCor->getHadronPar(3);
      // m_ratio = m_DBHadronCor->getHadronPar(4);
    }
  }
}

void
HadronSaturation::fillSample(TString infilename)
{

  std::cout << "\n\t HadronSaturation: filling sample events...\n" << std::endl;

  // clear the vectors to be filled
  clear();

  std::vector< TString > types;
  types.push_back("pion");
  types.push_back("kaon");
  types.push_back("proton");
  types.push_back("muon");
  // types.push_back("electron");

  // fill the containers with a previously prepared sample
  TFile* satFile = new TFile(infilename);

  for (int i = 0; i < int(types.size()); ++i) {

    TTree* satTree = (TTree*)satFile->Get(types[i]);
    double satbg, satcosth, satdedx, satdedxerr;
    satTree->SetBranchAddress("bg", &satbg);
    satTree->SetBranchAddress("costh", &satcosth);
    satTree->SetBranchAddress("dedx", &satdedx);
    satTree->SetBranchAddress("dedxerr", &satdedxerr);

    // fill the vectors
    for (unsigned int j = 0; j < satTree->GetEntries(); ++j) {
      satTree->GetEvent(j);
      if (satdedxerr == 0) continue;
      m_dedx.push_back(satdedx);
      m_dedxerror.push_back(satdedxerr);
      m_betagamma.push_back(satbg);
      m_costheta.push_back(satcosth);
      // std::cout << "Particle: " << types[i] << "   dedx:  " << satdedx << "   betagamma:   " << satbg << "   costh:  " << satcosth << std::endl;
    }
  }
  // std::cout << "size of dedx" << m_dedx.size() << std::endl;
  // std::cout << "size of dedx err " << m_dedxerror.size() << std::endl;
  // std::cout << "size of betagamma" << m_betagamma.size() << std::endl;
  // std::cout << "size of costh" << m_costheta.size() << std::endl;
  satFile->Close();
}

void
HadronSaturation::printEvents(int firstevent = 0, int nevents = 50)
{

  if ((nevents + firstevent) > m_dedx.size())nevents = m_dedx.size();

  if (firstevent < 0 || (nevents + firstevent) > m_dedx.size()) {
    std::cout << "HadronSaturation: trying to print events out of range (" << m_dedx.size() << ")" << std::endl;
    exit(1);
  }

  for (int i = firstevent; i < nevents; ++i) {
    std::cout << "\t Event " << i << ":\t bg = " << m_betagamma[i] << "\t cos(theta) = " << m_costheta[i] << "\t dE/dx mean = " <<
              m_dedx[i] << "\t dE/dx error = " << m_dedxerror[i] << std::endl;
  }
}

double
HadronSaturation::D2I(double cosTheta, double D, double alpha, double gamma, double delta, double power, double ratio) const
{

  double absCosTheta   = fabs(cosTheta);
  double projection    = pow(absCosTheta, power) + delta;
  if (projection == 0) {
    std::cout << "\t HadronSaturation: D2I Something wrong with dE/dx hadron constants!" << std::endl;
    return D;
  }

  double chargeDensity = D / projection;
  double numerator     = 1 + alpha * chargeDensity;
  double denominator   = 1 + gamma * chargeDensity;
  if (denominator == 0) {
    std::cout << "\t HadronSaturation: D2I Something wrong with dE/dx hadron constants!" << std::endl;
    return D;
  }

  double I = D * ratio * numerator / denominator;

  return I;
}

double
HadronSaturation::I2D(double cosTheta, double I, double alpha, double gamma, double delta, double power, double ratio) const
{

  double absCosTheta = fabs(cosTheta);
  double projection  = pow(absCosTheta, power) + delta;

  if (projection == 0 || ratio == 0) {
    std::cout << "\t HadronSaturation: I2D Something wrong with dE/dx hadron constants!" << std::endl;
    return I;
  }

  double a =  alpha / projection;
  double b =  1 - gamma / projection * (I / ratio);
  double c = -1.0 * I / ratio;

  if (b == 0 && a == 0) {
    std::cout << "\t HadronSaturation: both a and b coefficiants for hadron correction are 0" << std::endl;
    return I;
  }

  double discr = b * b - 4.0 * a * c;
  if (discr < 0) {
    std::cout << "negative discriminant; return uncorrectecd value" << std::endl;
    return I;
  }

  double D = (a != 0) ? (-b + sqrt(discr)) / (2.0 * a) : -c / b;
  if (D < 0) {
    std::cout << "D is less 0! will try another solution" << std::endl;
    D = (a != 0) ? (-b - sqrt(discr)) / (2.0 * a) : -c / b;
    if (D < 0) {
      std::cout << "D is still less 0! just return uncorrectecd value" << std::endl;
      return I;
    }
  }

  return D;
}

double
HadronSaturation::myFunction(double alpha, double gamma, double delta, double power, double ratio)
{

  unsigned int nevt = m_dedx.size();
  double chisq = 0, dedxsum = 0;
  std::vector< double > vdedxavg;

  // std::cout<<"CHECKing flag = " << m_flag );
  // Compute the average value (across cos(theta)) for each bin of beta-gamma.
  // NOTE: the correction is not constrained to a certain value (1), it
  // changes as a function of beta gamma...
  double dedxcor = 0;
  for (unsigned int i = 0; i < nevt; i++) {

    if (m_flag == 0)
      dedxcor = D2I(m_costheta[i], I2D(m_costheta[i], 1.0, alpha, gamma, delta, power, ratio) * m_dedx[i], alpha, gamma, delta, power,
                    ratio);
    else
      dedxcor = D2I(m_costheta[i], m_dedx[i], alpha, gamma, delta, power, ratio);

    dedxsum += dedxcor;

    // std::cout<<i <<  ") " << m_dedx[i] << ", " << dedxcor );
    if ((i + 1) % m_cosbins == 0) {
      vdedxavg.push_back(dedxsum / m_cosbins);
      std::cout << "\t --> average: = " << dedxsum / m_cosbins << ", " << m_cosbins << "" << std::endl;
      dedxsum = 0;
    }
  }

  // Construct a chi^2 value for the difference between the average in a cosine bin
  // to the actual values
  for (unsigned int i = 0; i < nevt; i++) {
    if (m_flag == 0)
      dedxcor = D2I(m_costheta[i], I2D(m_costheta[i], 1.0, alpha, gamma, delta, power, ratio) * m_dedx[i], alpha, gamma, delta, power,
                    ratio);
    else
      dedxcor = D2I(m_costheta[i], m_dedx[i], alpha, gamma, delta, power, ratio);

    int j = (int)i / m_cosbins;
    chisq += pow((dedxcor - vdedxavg[j]) / m_dedxerror[i], 2);
    std::cout << "\t " << i << ") " << dedxcor << "/" << vdedxavg[j] << ", error was "
              << m_dedxerror[i] << " De = " << I2D(m_costheta[i], 1.0, alpha, gamma, delta, power, ratio) <<
              ": Final " << chisq << std::endl;
  }

  return chisq;
}

void
HadronSaturation::minuitFunction(int& nDim, double* gout, double& result, double* par, int flag)
{
  result = HC_obj->myFunction(par[0], par[1], par[2], par[3], par[4]);
}

void
HadronSaturation::fitSaturation()
{

  std::cout << "\t Performing the hadron saturation fit..." << std::endl;

  // Construct the fitter
  TFitter* minimizer = new TFitter(5);
  minimizer->SetFCN(HadronSaturation::minuitFunction);

  TRandom* rand = new TRandom();

  minimizer->SetParameter(0, "alpha", m_alpha, rand->Rndm() * 0.001, -5.0, 5.0);
  minimizer->SetParameter(1, "gamma", m_gamma, rand->Rndm() * 0.001, -5.0, 5.0);
  minimizer->SetParameter(2, "delta", m_delta, rand->Rndm() * 0.001, 0, 0.50);
  minimizer->SetParameter(3, "power", m_power, rand->Rndm() * 0.01, 0, 0);
  minimizer->SetParameter(4, "ratio", m_ratio, rand->Rndm() * 0.01, 0.5, 2);
  minimizer->FixParameter(2);
  // minimizer->FixParameter(3);
  minimizer->FixParameter(4);

  // Set minuit fitting strategy
  double arg[10];
  arg[0] = 2;
  minimizer->ExecuteCommand("SET STR", arg, 1);

  // Suppress much of the output of MINUIT
  arg[0] = -1;
  minimizer->ExecuteCommand("SET PRINT", arg, 1);

  // Minimize with MIGRAD
  arg[0] = 10000;

  double fitpar[5], fiterr[5];

  for (int i = 0; i < 1; ++i) {

    minimizer->SetParameter(0, "alpha", m_alpha, rand->Rndm() * 0.001, -1.0, 1.0);
    minimizer->SetParameter(1, "gamma", m_gamma, rand->Rndm() * 0.001, -2.0, 2.0);
    minimizer->SetParameter(2, "delta", m_delta, rand->Rndm() * 0.001, 0, 0.50);
    minimizer->SetParameter(3, "power", m_power, rand->Rndm() * 0.01, 0.5, 2.5);
    minimizer->SetParameter(4, "ratio", m_ratio, rand->Rndm() * 0.01, 0.5, 2);
    minimizer->FixParameter(2);
    // minimizer->FixParameter(3);
    minimizer->FixParameter(4);

    int status = minimizer->ExecuteCommand("MIGRAD", arg, 1);
    status = minimizer->ExecuteCommand("MIGRAD", arg, 1);
    minimizer->PrintResults(1, 0);
    std::cout << "\t iter = " << i << ": Fit status: " << status << std::endl;

    int counter = 0;
    while (status != 0 && counter < 5) {

      counter++;
      minimizer->SetParameter(0, "alpha", m_alpha, rand->Rndm() * 0.001, -1.0, 1.0);
      minimizer->SetParameter(1, "gamma", m_gamma, rand->Rndm() * 0.001, -2.0, 2.0);
      minimizer->SetParameter(2, "delta", m_delta, rand->Rndm() * 0.001, 0, 0.50);
      minimizer->SetParameter(3, "power", m_power, rand->Rndm() * 0.01, 0.5, 2.5);
      minimizer->SetParameter(4, "ratio", m_ratio, rand->Rndm() * 0.01, 0.5, 2);
      minimizer->FixParameter(2); //lets find alpha and gamma only
      // minimizer->FixParameter(3); //lets find alpha and gamma only
      minimizer->FixParameter(4); //lets find alpha and gamma only

      status = minimizer->ExecuteCommand("MIGRAD", arg, 1);
      std::cout << "\t re-Fit iter: " << counter  << ", status code: " << status << std::endl;
    }

    if (status != 0) {
      std::cout << "\t HadronSaturation::ERROR - BAD FIT!" << std::endl;
      return;
    }
  }

  for (int par = 0; par < 5; ++par) {
    fitpar[par] = minimizer->GetParameter(par);
    fiterr[par] = minimizer->GetParError(par);
  }

  std::cout << "\t Final Result: HadronSaturation: fit results" << std::endl;
  std::cout << "\t alpha (" << m_alpha << "): " << fitpar[0] << " +- " << fiterr[0] << std::endl;
  std::cout << "\t gamma (" << m_gamma << "): " << fitpar[1] << " +- " << fiterr[1] << std::endl;
  std::cout << "\t delta (" << m_delta << "): " << fitpar[2] << " +- " << fiterr[2] << std::endl;
  std::cout << "\t power (" << m_power << "): " << fitpar[3] << " +- " << fiterr[3] << std::endl;
  std::cout << "\t ratio (" << m_ratio << "): " << fitpar[4] << " +- " << fiterr[4] << std::endl;

  // function value, estimated distance to minimum, errdef
  // variable parameters, number of parameters
  double chi2, edm, errdef;
  int nvpar, nparx;
  minimizer->GetStats(chi2, edm, errdef, nvpar, nparx);
  std::cout << "\t\t Fit chi^2: " << chi2 << std::endl;

  std::ofstream parfile;
  parfile.open("sat-pars.fit.txt");

  parfile << fitpar[0] << std::endl;
  parfile << fitpar[1] << std::endl;
  parfile << fitpar[2] << std::endl;
  parfile << fitpar[3] << std::endl;
  parfile << fitpar[4] << std::endl;
  parfile.close();

  delete minimizer;
}

void HadronSaturation::clear()
{

  m_dedx.clear();
  m_dedxerror.clear();
  m_betagamma.clear();
  m_costheta.clear();
}
