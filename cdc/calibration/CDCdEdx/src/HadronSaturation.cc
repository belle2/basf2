/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <cdc/calibration/CDCdEdx/HadronSaturation.h>
using namespace Belle2;

static HadronSaturation* HC_obj;

HadronSaturation::HadronSaturation() :
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

HadronSaturation::HadronSaturation(double alpha, double gamma, double delta, double power, double ratio, int cosbins) :
  m_cosbins(cosbins),
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
  HC_obj = this;
}

void HadronSaturation::fillSample(TString infilename)
{

  B2INFO("\n\t HadronSaturation: filling sample events...\n");

  // clear the vectors to be filled
  clear();

  std::vector< TString > types;
  types.push_back("pion");
  types.push_back("kaon");
  types.push_back("proton");
  types.push_back("muon");

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
    }
  }
  satFile->Close();
}

void
HadronSaturation::printEvents(int firstevent = 0, int nevents = 50)
{

  if ((nevents + firstevent) > int(m_dedx.size())) nevents = m_dedx.size();

  if (firstevent < 0 || (nevents + firstevent) > int(m_dedx.size()))
    B2FATAL("HadronSaturation: trying to print events out of range (" << m_dedx.size() << ")");

  for (int i = firstevent; i < nevents; ++i)
    B2INFO("\t Event " << i << ":\t bg = " << m_betagamma[i] << "\t cos(theta) = " << m_costheta[i] << "\t dE/dx mean = " <<
           m_dedx[i] << "\t dE/dx error = " << m_dedxerror[i]);

}

double HadronSaturation::myFunction(double alpha, double gamma, double delta, double power, double ratio)
{

  unsigned int nevt = m_dedx.size();
  double chisq = 0, dedxsum = 0;
  std::vector< double > vdedxavg;

  // Compute the average value (across cos(theta)) for each bin of beta-gamma.
  // NOTE: the correction is not constrained to a certain value (1), it
  // changes as a function of beta gamma...
  CDCDedxHadSat hadsat;

  double dedxcor = 0;
  for (unsigned int i = 0; i < nevt; i++) {

    dedxcor = hadsat.D2I(m_costheta[i], hadsat.I2D(m_costheta[i], 1.0, alpha, gamma, delta, power, ratio) * m_dedx[i], alpha, gamma,
                         delta, power, ratio);

    dedxsum += dedxcor;

    if ((i + 1) % m_cosbins == 0) {
      vdedxavg.push_back(dedxsum / m_cosbins);
      B2INFO("\t --> average: = " << dedxsum / m_cosbins << ", " << m_cosbins << "");
      dedxsum = 0;
    }
  }

  // Construct a chi^2 value for the difference between the average in a cosine bin
  // to the actual values
  for (unsigned int i = 0; i < nevt; i++) {
    dedxcor = hadsat.D2I(m_costheta[i], hadsat.I2D(m_costheta[i], 1.0, alpha, gamma, delta, power, ratio) * m_dedx[i], alpha, gamma,
                         delta, power, ratio);

    int j = (int)i / m_cosbins;
    chisq += pow((dedxcor - vdedxavg[j]) / m_dedxerror[i], 2);
    B2INFO("\t " << i << ") " << dedxcor << "/" << vdedxavg[j] << ", error was "
           << m_dedxerror[i] << " De = " << hadsat.I2D(m_costheta[i], 1.0, alpha, gamma, delta, power, ratio) <<
           ": Final " << chisq);
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

  B2INFO("\t Performing the hadron saturation fit...");

  // Construct the fitter
  TFitter* minimizer = new TFitter(5);
  minimizer->SetFCN(HadronSaturation::minuitFunction);

  minimizer->SetParameter(0, "alpha", m_alpha, gRandom->Rndm() * 0.001, -5.0, 5.0);
  minimizer->SetParameter(1, "gamma", m_gamma, gRandom->Rndm() * 0.001, -5.0, 5.0);
  minimizer->SetParameter(2, "delta", m_delta, gRandom->Rndm() * 0.001, 0, 0.50);
  minimizer->SetParameter(3, "power", m_power, gRandom->Rndm() * 0.01, 0.05, 2.5);
  minimizer->SetParameter(4, "ratio", m_ratio, gRandom->Rndm() * 0.01, 0.5, 2);
  minimizer->FixParameter(2);
  // minimizer->FixParameter(3);
  minimizer->FixParameter(4);

  // Set minuit fitting strategy
  double arg[10];
  arg[0] = 2;
  double strategy(2.);
  minimizer->ExecuteCommand("SET STRAT", &strategy, 1);

  double up(1.);
  minimizer->ExecuteCommand("SET ERR", &up, 1);

  // Suppress much of the output of MINUIT
  arg[0] = -1;
  minimizer->ExecuteCommand("SET PRINT", arg, 1);

  double eps_machine(std::numeric_limits<double>::epsilon());
  minimizer->ExecuteCommand("SET EPS", &eps_machine, 1);

  // Minimize with MIGRAD
  arg[0] = 10000;

  double fitpar[5], fiterr[5];

  for (int i = 0; i < 30; ++i) {

    minimizer->SetParameter(0, "alpha", m_alpha, gRandom->Rndm() * 0.001, -5.0, 5.0);
    minimizer->SetParameter(1, "gamma", m_gamma, gRandom->Rndm() * 0.001, -5.0, 5.0);
    minimizer->SetParameter(2, "delta", m_delta, gRandom->Rndm() * 0.001, 0, 0.50);
    minimizer->SetParameter(3, "power", m_power, gRandom->Rndm() * 0.01, 0.05, 2.5);
    minimizer->SetParameter(4, "ratio", m_ratio, gRandom->Rndm() * 0.01, 0.5, 2);
    minimizer->FixParameter(2);
    // minimizer->FixParameter(3);
    minimizer->FixParameter(4);

    double maxcalls(5000.), tolerance(0.1);
    double arglist[] = {maxcalls, tolerance};
    unsigned int nargs(2);
    int status = minimizer->ExecuteCommand("MIGRAD", arglist, nargs);
    status = minimizer->ExecuteCommand("MIGRAD", arglist, nargs);
    status = minimizer->ExecuteCommand("HESSE", arglist, nargs);

    // int status = minimizer->ExecuteCommand("MIGRAD", arg, 1);
    // status = minimizer->ExecuteCommand("MIGRAD", arg, 1);
    minimizer->PrintResults(1, 0);
    B2INFO("\t iter = " << i << ": Fit status: " << status);

    int counter = 0;
    while (status != 0 && counter < 5) {
      minimizer->SetParameter(0, "alpha", m_alpha, gRandom->Rndm() * 0.001, -5.0, 5.0);
      minimizer->SetParameter(1, "gamma", m_gamma, gRandom->Rndm() * 0.001, -5.0, 5.0);
      minimizer->SetParameter(2, "delta", m_delta, gRandom->Rndm() * 0.001, 0, 0.50);
      minimizer->SetParameter(3, "power", m_power, gRandom->Rndm() * 0.01, 0.05, 2.5);
      minimizer->SetParameter(4, "ratio", m_ratio, gRandom->Rndm() * 0.01, 0.5, 2);
      minimizer->FixParameter(2);
      // minimizer->FixParameter(3);
      minimizer->FixParameter(4);
      counter++;
      status = minimizer->ExecuteCommand("MIGRAD", arglist, nargs);
      status = minimizer->ExecuteCommand("HESSE", arglist, nargs);
      B2INFO("\t re-Fit iter: " << counter  << ", status code: " << status);
    }

    if (status != 0) {
      B2INFO("\t HadronSaturation::ERROR - BAD FIT!");
      return;
    }
    if (status == 0) break;
  }

  for (int par = 0; par < 5; ++par) {
    fitpar[par] = minimizer->GetParameter(par);
    fiterr[par] = minimizer->GetParError(par);
  }

  B2INFO("\t Final Result: HadronSaturation: fit results");
  B2INFO("\t alpha (" << m_alpha << "): " << fitpar[0] << " +- " << fiterr[0]);
  B2INFO("\t gamma (" << m_gamma << "): " << fitpar[1] << " +- " << fiterr[1]);
  B2INFO("\t delta (" << m_delta << "): " << fitpar[2] << " +- " << fiterr[2]);
  B2INFO("\t power (" << m_power << "): " << fitpar[3] << " +- " << fiterr[3]);
  B2INFO("\t ratio (" << m_ratio << "): " << fitpar[4] << " +- " << fiterr[4]);

  // function value, estimated distance to minimum, errdef
  // variable parameters, number of parameters
  double chi2, edm, errdef;
  int nvpar, nparx;
  minimizer->GetStats(chi2, edm, errdef, nvpar, nparx);
  B2INFO("\t\t Fit chi^2: " << chi2);

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
