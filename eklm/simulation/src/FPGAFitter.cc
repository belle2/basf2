/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* System headers. */
#include <math.h>

/* External headers. */
#include <TFile.h>
#include <TH1F.h>
#include <TMinuit.h>

/* Belle2 headers. */
#include <eklm/simulation/FPGAFitter.h>
#include <framework/core/utilities.h>
#include <framework/logging/Logger.h>

static const char MemErr[] = "Memory allocation error.";

using namespace Belle2;

static int* famp;
static int n;

EKLM::FPGAFitter::FPGAFitter(int nPoints)
{
  int i;
  TFile* f;
  TH1F* h;
  m_nPoints = nPoints;
  try {
    f = new TFile(FileSystem::findFile("/data/eklm/FPGA.root").c_str());
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  if (f->IsZombie())
    B2FATAL("Cannot open FPGA data file.");
  h = (TH1F*)f->Get("FitShape");
  if (h->GetNbinsX() != m_nPoints)
    B2FATAL("Numbers of digitization points in FPGA data file and program are "
            "diffetent.");
  try {
    m_sig = new float[m_nPoints];
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  for (i = 0; i < m_nPoints; i++)
    m_sig[i] = h->GetBinContent(i + 1);
  f->Close();
  delete f;
}

EKLM::FPGAFitter::~FPGAFitter()
{
}

static double SignalShapeFitFunction(double x, double* par)
{
  double u = 0;
  if (x > par[0] && x < par[0] + par[1])
    u = par[3] / par[1] * (x - par[0]);
  else if (x > par[0] + par[1])
    u = par[3] * exp(-par[2] * (x - par[0] - par[1]));
  return u + par[4];
}

static void fcn(int& npar, double* grad, double& fval, double* par, int iflag)
{
  (void)npar;
  (void)grad;
  (void)iflag;
  int i;
  double sig;
  fval = 0.0;
  for (i = 0; i < n; i++) {
    sig = SignalShapeFitFunction(i, par);
    if (famp[i] < 0.5)
      fval = fval + 2.0 * sig;
    else
      fval = fval + 2.0 * (sig - famp[i] * (1.0 - log(famp[i] / sig)));
  }
}

enum EKLM::FPGAFitStatus EKLM::FPGAFitter::fit(int* amp, float* fit,
                                               struct FPGAFitParams* par)
{
  const int thr = 100;
  int i;
  int sum;
  int firstSig;
  int lastSig;
  double mpar[5];
  sum = 0;
  firstSig = -1;
  lastSig = m_nPoints - 1;
  /**
   * Calculate integral above threshold and simultaneously find
   * first and last point above threshold.
   */
  for (i = 0; i < m_nPoints; i++) {
    if (amp[i] > thr) {
      sum = sum + amp[i];
      if (firstSig == -1)
        firstSig = i;
      lastSig = i;
    }
  }
  if (firstSig == -1)
    return c_FPGANoSignal;
  mpar[0] = firstSig > 1 ? firstSig : 1;
  mpar[1] = 2;
  mpar[2] = 2. / (lastSig - firstSig + 1);
  mpar[3] = sum / (lastSig - firstSig + 1);
  mpar[4] = 0.;
  /**
   * TODO: Minuit is used here.
   *       It must be replaced by actual FPGA algorithm.
   */
  double args[1];
  TMinuit* mn = new TMinuit(5);
  famp = amp;
  n = m_nPoints;
  mn->SetFCN(fcn);
  int ierflg;
  mn->SetPrintLevel(-1);
  for (i = 0; i <= 1; i++)
    mn->mnparm(i, "", mpar[i], fabs(mpar[i]) / 10., 0.0, m_nPoints, ierflg);
  for (i = 2; i <= 3; i++)
    mn->mnparm(i, "", mpar[i], fabs(mpar[i]) / 10., 0.0, 0.0, ierflg);
  mn->mnparm(4, "", mpar[4], 1.0, 0.0, 0.0, ierflg);
  args[0] = 1000;
  if (par->bgAmplitude == 0.0)
    mn->mnfixp(4, ierflg);
  mn->mnexcm("MIGRAD", args, 1, ierflg);
  double err, xup, xlow;
  int iuint;
  TString s;
  for (i = 0; i < 5; i++)
    mn->mnpout(i, s, mpar[i], err, xup, xlow, iuint);
  delete mn;
  /*** End of Minuit code. ***/
  for (i = 0; i < m_nPoints; i++)
    fit[i] = SignalShapeFitFunction(i, mpar);
  par->startTime = mpar[0];
  par->peakTime = mpar[1];
  par->attenuationFreq = mpar[2];
  par->amplitude = mpar[3];
  par->bgAmplitude = mpar[4];
  return c_FPGASuccessfulFit;
}

