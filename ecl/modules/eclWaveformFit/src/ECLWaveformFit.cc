/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <ecl/modules/eclWaveformFit/ECLWaveformFit.h>

/* ECL headers. */
#include <ecl/digitization/EclConfiguration.h>
#include <ecl/digitization/shaperdsp.h>

/* Basf2 headers. */
#include <framework/core/Environment.h>

/* ROOT headers. */
#include <TMinuit.h>
#include <TMatrixD.h>
#include <TMatrixDSym.h>
#include <TDecompChol.h>

/* C++ headers. */
#include <numeric>

using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Modules
//-----------------------------------------------------------------
REG_MODULE(ECLWaveformFit);
//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

extern "C" {

  // Load inverse covariance matrix from the packed form.
  // @param[in] packed_matrix Packed matrix.
  void ecl_waveform_fit_load_inverse_covariance(const float* packed_matrix);

  // Multiply vector by the stored inverse covariance matrix.
  // @param[out] y Result vector.
  // @param[in]  x Vector.
  void ecl_waveform_fit_multiply_inverse_covariance(double* y, const double* x);

};


//anonymous namespace for data objects used by both ECLWaveformFitModule class and FCN2h funciton for MINUIT minimization.
namespace {

  // Number of fit points.
  const int c_NFitPoints = 31;

  // Number of fit points for vectorized data.
  const int c_NFitPointsVector = 32;

  //adc data array
  double fitA[c_NFitPoints];

  //g_si: photon template signal shape
  //g_sih: hadron template signal shape
  const SignalInterpolation2* g_si;
  const SignalInterpolation2* g_sih;

  /** Covariance matrix. */
  double s_CurrentCovMat[c_NFitPoints][c_NFitPoints] = {0};

  /** Noise level. */
  double aNoise;

  //Function to minimize in photon template + hadron template fit. (chi2)
  // cppcheck-suppress constParameter ; TF1 fit functions cannot have const parameters
  void FCN2h(int&, double* grad, double& f, double* p, int)
  {
    double df[c_NFitPointsVector];
    double da[c_NFitPointsVector];
    const double Ag = p[1], B = p[0], T = p[2], Ah = p[3];
    double chi2 = 0, gAg = 0, gB = 0, gT = 0, gAh = 0;

    //getting photon and hadron component shapes for set of fit parameters
    double amplitudeGamma[c_NFitPoints], derivativesGamma[c_NFitPoints];
    double amplitudeHadron[c_NFitPoints], derivativesHadron[c_NFitPoints];
    g_si->getshape(T, amplitudeGamma, derivativesGamma);
    g_sih->getshape(T, amplitudeHadron, derivativesHadron);

    //computing difference between current fit result and adc data array
    #pragma omp simd
    for (int i = 0; i < c_NFitPoints; ++i)
      df[i] = fitA[i] - (Ag * amplitudeGamma[i] + Ah * amplitudeHadron[i] + B);

    //computing chi2.
    ecl_waveform_fit_multiply_inverse_covariance(da, df);

    #pragma omp simd reduction(+:chi2) reduction(-:gB,gAg,gT,gAh)
    for (int i = 0; i < c_NFitPoints; ++i) {
      chi2 += da[i] * df[i];
      gB   -= da[i];
      gAg  -= da[i] * amplitudeGamma[i];
      gT   -= da[i] * (derivativesGamma[i] * Ag + derivativesHadron[i] * Ah);
      gAh  -= da[i] * amplitudeHadron[i];
    }

    f = chi2;
    grad[0] = 2 * gB;
    grad[1] = 2 * gAg;
    grad[2] = 2 * gT;
    grad[3] = 2 * gAh;
  }

  //Function to minimize in photon template + hadron template + background photon fit. (chi2)
  // cppcheck-suppress constParameter ; TF1 fit functions cannot have const parameters
  void FCN2h2(int&, double* grad, double& f, double* p, int)
  {
    double df[c_NFitPointsVector];
    double da[c_NFitPointsVector];
    const double A2 = p[4], T2 = p[5];
    const double Ag = p[1], B = p[0], T = p[2], Ah = p[3];
    double chi2 = 0, gA2  = 0, gT2 = 0;
    double gAg = 0, gB = 0, gT = 0, gAh = 0;

    //getting photon and hadron component shapes for set of fit parameters
    double amplitudeGamma[c_NFitPoints], derivativesGamma[c_NFitPoints];
    double amplitudeGamma2[c_NFitPoints], derivativesGamma2[c_NFitPoints];
    double amplitudeHadron[c_NFitPoints], derivativesHadron[c_NFitPoints];
    g_si->getshape(T, amplitudeGamma, derivativesGamma);
    // Background photon.
    g_si->getshape(T2, amplitudeGamma2, derivativesGamma2);
    g_sih->getshape(T, amplitudeHadron, derivativesHadron);

    //computing difference between current fit result and adc data array
    #pragma omp simd
    for (int i = 0; i < c_NFitPoints; ++i) {
      df[i] = fitA[i] - (Ag * amplitudeGamma[i] + Ah * amplitudeHadron[i]
                         + A2 * amplitudeGamma2[i] + B);
    }

    //computing chi2.
    ecl_waveform_fit_multiply_inverse_covariance(da, df);

    #pragma omp simd reduction(+:chi2) reduction(-:gB,gAg,gT,gAh,gA2,gT2)
    for (int i = 0; i < c_NFitPoints; ++i) {
      chi2 += da[i] * df[i];
      gB  -= da[i];
      gAg  -= da[i] * amplitudeGamma[i];
      gAh  -= da[i] * amplitudeHadron[i];
      gT   -= da[i] * (derivativesGamma[i] * Ag + derivativesHadron[i] * Ah);

      gA2 -= da[i] * amplitudeGamma2[i];
      gT2 -= da[i] * derivativesGamma2[i] * A2;
    }
    f = chi2;
    grad[0] = 2 * gB;
    grad[1] = 2 * gAg;
    grad[2] = 2 * gT;
    grad[3] = 2 * gAh;
    grad[4] = 2 * gA2;
    grad[5] = 2 * gT2;
  }

  // regularize autocovariance function by multipling it by the step
  // function so elements above u0 become 0 and below are untouched.
  void regularize(double* dst, const double* src, const int n, const double u0 = 13.0, const double u1 = 0.8)
  {
    for (int k = 0; k < n; k++) dst[k] = src[k] / (1 + exp((k - u0) / u1));
  }

  // transform autocovariance function of c_NFitPoints elements to the covariance matrix
  bool makecovariance(CovariancePacked& M, const int nnoise, const double* acov)
  {
    TMatrixDSym E(c_NFitPoints);
    for (int i = 0; i < c_NFitPoints; i++)
      for (int j = 0; j < i + 1; j++)
        if (i - j < nnoise) E(i, j) = E(j, i) = acov[i - j];

    TDecompChol dc(E);
    const bool status = dc.Invert(E);

    if (status) {
      int count = 0;
      for (int i = 0; i < c_NFitPoints; i++)
        for (int j = 0; j < i + 1; j++)
          M[count++] = E(i, j);
      M.sigma = sqrtf(acov[0]);
    }
    return status;
  }

}

// constructor
ECLWaveformFitModule::ECLWaveformFitModule()
{
  // Set module properties
  setDescription("Module to fit offline waveforms and measure hadron scintillation component light output.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("EnergyThreshold", m_EnergyThreshold, "Energy threshold of online fit result for Fitting Waveforms (GeV).", 0.03);
  addParam("Chi2Threshold25dof", m_chi2Threshold25dof, "chi2 threshold (25 dof) to classify offline fit as good fit.", 57.1);
  addParam("Chi2Threshold27dof", m_chi2Threshold27dof, "chi2 threshold (27 dof) to classify offline fit as good fit.", 60.0);
  addParam("CovarianceMatrix", m_CovarianceMatrix,
           "Option to use crystal dependent covariance matrices (false uses identity matrix).", true);
}

// destructor
ECLWaveformFitModule::~ECLWaveformFitModule()
{
}

//callback for loading templates from database
void ECLWaveformFitModule::loadTemplateParameterArray()
{

  m_TemplatesLoaded = true;

  if (m_IsMCFlag == 0) {
    //load data templates
    std::vector<double>  Ptemp(11), Htemp(11), Dtemp(11);
    for (int i = 0; i < ECLElementNumbers::c_NCrystals; i++) {
      for (int j = 0; j < 11; j++) {
        Ptemp[j] = (double)m_WaveformParameters->getPhotonParameters(i + 1)[j];
        Htemp[j] = (double)m_WaveformParameters->getHadronParameters(i + 1)[j];
        Dtemp[j] = (double)m_WaveformParameters->getDiodeParameters(i + 1)[j];
      }
      new (&m_si[i][0]) SignalInterpolation2(Ptemp);
      new (&m_si[i][1]) SignalInterpolation2(Htemp);
      new (&m_si[i][2]) SignalInterpolation2(Dtemp);
    }
  } else {
    //load mc template
    std::vector<double>  Ptemp(11), Htemp(11), Dtemp(11);
    for (int j = 0; j < 11; j++) {
      Ptemp[j] = (double)m_WaveformParametersForMC->getPhotonParameters()[j];
      Htemp[j] = (double)m_WaveformParametersForMC->getHadronParameters()[j];
      Dtemp[j] = (double)m_WaveformParametersForMC->getDiodeParameters()[j];
    }
    new (&m_si[0][0]) SignalInterpolation2(Ptemp);
    new (&m_si[0][1]) SignalInterpolation2(Htemp);
    new (&m_si[0][2]) SignalInterpolation2(Dtemp);
  }
}

void ECLWaveformFitModule::beginRun()
{

  m_IsMCFlag = Environment::Instance().isMC();
  m_TemplatesLoaded = false;

  m_ADCtoEnergy.resize(ECLElementNumbers::c_NCrystals);
  if (m_CrystalElectronics.isValid()) {
    for (int i = 0; i < ECLElementNumbers::c_NCrystals; i++)
      m_ADCtoEnergy[i] = m_CrystalElectronics->getCalibVector()[i];
  }
  if (m_CrystalEnergy.isValid()) {
    for (int i = 0; i < ECLElementNumbers::c_NCrystals; i++)
      m_ADCtoEnergy[i] *= m_CrystalEnergy->getCalibVector()[i];
  }

  //Load covariance matricies from database;
  if (m_CovarianceMatrix) {
    for (int id = 1; id <= ECLElementNumbers::c_NCrystals; id++) {
      double buf[c_NFitPoints];
      double reg[c_NFitPoints];
      m_AutoCovariance->getAutoCovariance(id, buf);
      double x0 = c_NFitPoints;
      std::memcpy(reg, buf, c_NFitPoints * sizeof(double));
      while (!makecovariance(m_c[id - 1], c_NFitPoints, reg))
        regularize(buf, reg, c_NFitPoints, x0 -= 1, 1);
    }
  } else {
    //default covariance matrix is identity for all crystals
    const double isigma = 1 / 7.5;
    for (int i = 0; i < c_NFitPoints; ++i) {
      for (int j = 0; j < c_NFitPoints; ++j) {
        s_CurrentCovMat[i][j] = (i == j) * isigma * isigma;
      }
    }
  }

}

void ECLWaveformFitModule::initialize()
{
  // ECL dataobjects
  m_eclDSPs.registerInDataStore();
  m_eclDigits.registerInDataStore();

  //initializing fit minimizer
  m_Minit2h = new TMinuit(4);
  m_Minit2h->SetFCN(FCN2h);
  double arglist[10];
  int ierflg = 0;
  arglist[0] = -1;
  m_Minit2h->mnexcm("SET PRIntout", arglist, 1, ierflg);
  m_Minit2h->mnexcm("SET NOWarnings", arglist, 0, ierflg);
  arglist[0] = 1;
  m_Minit2h->mnexcm("SET ERR", arglist, 1, ierflg);
  arglist[0] = 0;
  m_Minit2h->mnexcm("SET STRategy", arglist, 1, ierflg);
  arglist[0] = 1;
  m_Minit2h->mnexcm("SET GRAdient", arglist, 1, ierflg);
  arglist[0] = 1e-6;
  m_Minit2h->mnexcm("SET EPSmachine", arglist, 1, ierflg);

  //initializing fit minimizer photon+hadron + background photon
  m_Minit2h2 = new TMinuit(6);
  m_Minit2h2->SetFCN(FCN2h2);
  arglist[0] = -1;
  m_Minit2h2->mnexcm("SET PRIntout", arglist, 1, ierflg);
  m_Minit2h2->mnexcm("SET NOWarnings", arglist, 0, ierflg);
  arglist[0] = 1;
  m_Minit2h2->mnexcm("SET ERR", arglist, 1, ierflg);
  arglist[0] = 0;
  m_Minit2h2->mnexcm("SET STRategy", arglist, 1, ierflg);
  arglist[0] = 1;
  m_Minit2h2->mnexcm("SET GRAdient", arglist, 1, ierflg);
  arglist[0] = 1e-6;
  m_Minit2h2->mnexcm("SET EPSmachine", arglist, 1, ierflg);

  //flag for callback to load templates each run
  m_TemplatesLoaded = false;
}

void ECLWaveformFitModule::event()
{
  const EclConfiguration& ec = EclConfiguration::get();

  if (!m_TemplatesLoaded) {
    //load templates once per run in first event that has saved waveforms.
    if (m_eclDSPs.getEntries() > 0)  loadTemplateParameterArray();
  }

  for (auto& aECLDsp : m_eclDSPs) {

    aECLDsp.setTwoComponentTotalAmp(-1);
    aECLDsp.setTwoComponentHadronAmp(-1);
    aECLDsp.setTwoComponentDiodeAmp(-1);
    aECLDsp.setTwoComponentChi2(-1);
    aECLDsp.setTwoComponentSavedChi2(ECLDsp::photonHadron, -1);
    aECLDsp.setTwoComponentSavedChi2(ECLDsp::photonHadronBackgroundPhoton, -1);
    aECLDsp.setTwoComponentSavedChi2(ECLDsp::photonDiodeCrossing, -1);
    aECLDsp.setTwoComponentTime(-1);
    aECLDsp.setTwoComponentBaseline(-1);
    aECLDsp.setTwoComponentFitType(ECLDsp::poorChi2);

    const int id = aECLDsp.getCellId() - 1;

    // Filling array with ADC values.
    for (int j = 0; j < ec.m_nsmp; j++)
      fitA[j] = aECLDsp.getDspA()[j];

    //setting relation of eclDSP to aECLDigit
    const ECLDigit* d = nullptr;
    for (const auto& aECLDigit : m_eclDigits) {
      if (aECLDigit.getCellId() - 1 == id) {
        d = &aECLDigit;
        aECLDsp.addRelationTo(&aECLDigit);
        break;
      }
    }
    if (d == nullptr) continue;

    //Skipping low amplitude waveforms
    if (d->getAmp() * m_ADCtoEnergy[id] < m_EnergyThreshold)  continue;

    //loading template for waveform
    if (m_IsMCFlag == 0) {
      //data cell id dependent
      g_si = &m_si[id][0];
      g_sih = &m_si[id][1];
    } else {
      // mc uses same waveform
      g_si = &m_si[0][0];
      g_sih = &m_si[0][1];
    }

    //get covariance matrix for cell id
    if (m_CovarianceMatrix) {
      ecl_waveform_fit_load_inverse_covariance(m_c[id].m_covMatPacked);
      aNoise = m_c[id].sigma;
    }

    //Calling optimized fit photon template + hadron template (fit type = 0)
    double p2_b, p2_a, p2_t, p2_a1, p2_chi2, p_extraPhotonEnergy, p_extraPhotonTime;
    ECLDsp::TwoComponentFitType fitType = ECLDsp::photonHadron;
    p2_chi2 = -1;
    Fit2h(p2_b, p2_a, p2_t, p2_a1, p2_chi2);
    aECLDsp.setTwoComponentSavedChi2(ECLDsp::photonHadron, p2_chi2);

    //if hadron fit failed try hadron + background photon (fit type = 1)
    if (p2_chi2 >= m_chi2Threshold27dof) {

      fitType = ECLDsp::photonHadronBackgroundPhoton;
      p2_chi2 = -1;
      Fit2hExtraPhoton(p2_b, p2_a, p2_t, p2_a1, p_extraPhotonEnergy, p_extraPhotonTime, p2_chi2);
      aECLDsp.setTwoComponentSavedChi2(ECLDsp::photonHadronBackgroundPhoton, p2_chi2);

      //hadron + background photon fit failed try diode fit (fit type = 2)
      if (p2_chi2 >= m_chi2Threshold25dof) {
        g_sih = &m_si[0][2];//set second component to diode
        fitType = ECLDsp::photonDiodeCrossing;
        p2_chi2 = -1;
        Fit2h(p2_b, p2_a, p2_t, p2_a1, p2_chi2);
        aECLDsp.setTwoComponentSavedChi2(ECLDsp::photonDiodeCrossing, p2_chi2);

        if (p2_chi2 >= m_chi2Threshold27dof) fitType = ECLDsp::poorChi2;  //indicates all fits tried had bad chi2
      }

    }

    //storing fit results
    aECLDsp.setTwoComponentTotalAmp(p2_a + p2_a1);
    if (fitType == ECLDsp::photonDiodeCrossing) {
      aECLDsp.setTwoComponentHadronAmp(0.0);
      aECLDsp.setTwoComponentDiodeAmp(p2_a1);
    } else {
      aECLDsp.setTwoComponentHadronAmp(p2_a1);
      aECLDsp.setTwoComponentDiodeAmp(0.0);
    }
    aECLDsp.setTwoComponentChi2(p2_chi2);
    aECLDsp.setTwoComponentTime(p2_t);
    aECLDsp.setTwoComponentBaseline(p2_b);
    aECLDsp.setTwoComponentFitType(fitType);
    if (fitType == ECLDsp::photonHadronBackgroundPhoton) {
      aECLDsp.setbackgroundPhotonEnergy(p_extraPhotonEnergy);
      aECLDsp.setbackgroundPhotonTime(p_extraPhotonTime);
    }
  }
}

// end run
void ECLWaveformFitModule::endRun()
{
}

// terminate
void ECLWaveformFitModule::terminate()
{
}

//Two component fit. Fits photon+hadron component hypothesis
void ECLWaveformFitModule::Fit2h(double& B, double& Ag, double& T, double& Ah, double& amin)
{
  //minuit parameters
  double arglist[10];
  int ierflg = 0;

  // setting inital fit parameters
  double dt = 0.5;
  double amax = 0;
  int jmax = 6;
  for (int j = 0; j < c_NFitPoints; j++) if (amax < fitA[j]) { amax = fitA[j]; jmax = j;}
  double sumB0 = 0; int jsum = 0;
  for (int j = 0; j < c_NFitPoints; j++) if (j < jmax - 3 || jmax + 4 < j) { sumB0 += fitA[j]; ++jsum;}
  double B0 = sumB0 / jsum;
  amax -= B0;
  if (amax < 0) amax = 10;
  double T0 = dt * (4.5 - jmax);
  double A0 = amax;

  //initalize minimizer
  m_Minit2h->mnparm(0, "B",  B0,    10, B0 / 1.5, B0 * 1.5, ierflg);
  m_Minit2h->mnparm(1, "Ag", A0, A0 / 20,      0,   2 * A0, ierflg);
  m_Minit2h->mnparm(2, "T",  T0,   0.5, T0 - 2.5, T0 + 2.5, ierflg);
  m_Minit2h->mnparm(3, "Ah", 0., A0 / 20,    -A0,   2 * A0, ierflg);

  //perform fit
  arglist[0] = 50000;
  arglist[1] = 1.;
  m_Minit2h->mnexcm("MIGRAD", arglist, 2, ierflg);

  double edm, errdef;
  int nvpar, nparx, icstat;
  m_Minit2h->mnstat(amin, edm, errdef, nvpar, nparx, icstat);

  //get fit results
  double ep;
  m_Minit2h->GetParameter(0, B,  ep);
  m_Minit2h->GetParameter(1, Ag, ep);
  m_Minit2h->GetParameter(2, T,  ep);
  m_Minit2h->GetParameter(3, Ah, ep);
}

//Two component fit background ith extra photon. Fits photon+hadron component + extra photon hypothesis
void ECLWaveformFitModule::Fit2hExtraPhoton(double& B, double& Ag, double& T, double& Ah, double& A2, double& T2, double& amin)
{
  double arglist[10];
  int ierflg = 0;
  double dt = 0.5;
  double amax = 0; int jmax = 6;
  for (int j = 0; j < c_NFitPoints; j++) if (amax < fitA[j]) { amax = fitA[j]; jmax = j;}

  double amax1 = 0; int jmax1 = 6;
  for (int j = 0; j < c_NFitPoints; j++)
    if (j < jmax - 3 || jmax + 4 < j) {
      if (j == 0) {
        if (amax1 < fitA[j] && fitA[j + 1] < fitA[j]) { amax1 = fitA[j]; jmax1 = j;}
      } else if (j == 30) {
        if (amax1 < fitA[j] && fitA[j - 1] < fitA[j]) { amax1 = fitA[j]; jmax1 = j;}
      } else {
        if (amax1 < fitA[j] && fitA[j + 1] < fitA[j] && fitA[j - 1] < fitA[j]) { amax1 = fitA[j]; jmax1 = j;}
      }
    }

  double sumB0 = 0; int jsum = 0;
  for (int j = 0; j < c_NFitPoints; j++) if ((j < jmax - 3 || jmax + 4 < j) && (j < jmax1 - 3 || jmax1 + 4 < j)) { sumB0 += fitA[j]; ++jsum;}
  double B0 = sumB0 / jsum;
  amax -= B0; amax = std::max(10.0, amax);
  amax1 -= B0; amax1 = std::max(10.0, amax1);
  double T0 = dt * (4.5 - jmax);
  double T01 = dt * (4.5 - jmax1);

  double A0 = amax, A01 = amax1;
  m_Minit2h2->mnparm(0, "B",  B0,    10, B0 / 1.5, B0 * 1.5, ierflg);
  m_Minit2h2->mnparm(1, "Ag", A0, A0 / 20,      0,   2 * A0, ierflg);
  m_Minit2h2->mnparm(2, "T",  T0,   0.5, T0 - 2.5, T0 + 2.5, ierflg);
  m_Minit2h2->mnparm(3, "Ah", 0., A0 / 20,    -A0,   2 * A0, ierflg);
  m_Minit2h2->mnparm(4, "A2", A01, A01 / 20,    0,   2 * A01, ierflg);
  m_Minit2h2->mnparm(5, "T2", T01,  0.5,    T01 - 2.5,   T01 + 2.5, ierflg);

  // Now ready for minimization step
  arglist[0] = 50000;
  arglist[1] = 1.;
  m_Minit2h2->mnexcm("MIGRAD", arglist, 2, ierflg);

  double edm, errdef;
  int nvpar, nparx, icstat;
  m_Minit2h2->mnstat(amin, edm, errdef, nvpar, nparx, icstat);
  double ep;
  m_Minit2h2->GetParameter(0, B,  ep);
  m_Minit2h2->GetParameter(1, Ag, ep);
  m_Minit2h2->GetParameter(2, T,  ep);
  m_Minit2h2->GetParameter(3, Ah, ep);
  m_Minit2h2->GetParameter(4, A2, ep);
  m_Minit2h2->GetParameter(5, T2, ep);
}



//Signal interpolation code used for fast evaluation of shaperDSP templates
SignalInterpolation2::SignalInterpolation2(const std::vector<double>& s)
{
  double T0 = -0.2;
  std::vector<double> p(s.begin() + 1, s.end());
  p[1] = std::max(0.0029, p[1]);
  p[4] = std::max(0.0029, p[4]);

  ShaperDSP_t dsp(p, s[0]);
  dsp.settimestride(c_dtn);
  dsp.settimeseed(T0);
  dd_t t[(c_nt + c_ntail)*c_ndt];
  dsp.fillarray(sizeof(t) / sizeof(t[0]), t);

  for (int i = 0; i < c_nt * c_ndt; i++) {
    m_FunctionInterpolation[i][0] = t[i].first;
    m_FunctionInterpolation[i][1] = t[i].second;
  }
  for (int i = 0; i < c_ntail; i++) {
    int j = c_nt * c_ndt + i;
    int k = c_nt * c_ndt + i * c_ndt;
    m_FunctionInterpolation[j][0] = t[k].first;
    m_FunctionInterpolation[j][1] = t[k].second;
  }
  int i1 = c_nt * c_ndt + c_ntail - 2;
  int i2 = c_nt * c_ndt + c_ntail - 1;
  m_r0 = m_FunctionInterpolation[i2][0] / m_FunctionInterpolation[i1][0];
  m_r1 = m_FunctionInterpolation[i2][1] / m_FunctionInterpolation[i1][1];
}

void SignalInterpolation2::getshape(
  double t0, double* function, double* derivatives) const
{
  /* If before pulse start time (negative times), return 0. */
  int k = 0;
  while (t0 < 0) {
    function[k] = 0;
    derivatives[k] = 0;
    t0 += c_dt;
    ++k;
    if (k >= c_NFitPoints)
      return;
  }

  /* Function and derivative values. */
  double f0[c_NFitPoints], f1[c_NFitPoints];
  double fp0[c_NFitPoints], fp1[c_NFitPoints];

  /* Interpolate first c_nt points (short time steps). */
  double x = t0 * c_idtn;
  double ix = floor(x);
  double w = x - ix;
  int j = ix;
  double w2 = w * w;
  double hw2 = 0.5 * w2;
  double tw3 = ((1. / 6) * w) * w2;

  /* Number of interpolation points. */
  int iMax = k + c_nt;
  if (iMax > c_NFitPoints)
    iMax = c_NFitPoints;

  /* Fill interpolation points. */
  for (int i = k; i < iMax; ++i) {
    f0[i] = m_FunctionInterpolation[j][0];
    f1[i] = m_FunctionInterpolation[j + 1][0];
    fp0[i] = m_FunctionInterpolation[j][1];
    fp1[i] = m_FunctionInterpolation[j + 1][1];
    j = j + c_ndt;
  }

  /* Interpolation. */
  #pragma omp simd
  for (int i = k; i < iMax; ++i) {
    double a[4];
    double dfdt = (f1[i] - f0[i]) * c_idtn;
    double fp = fp1[i] + fp0[i];
    a[0] = f0[i];
    a[1] = fp0[i];
    a[2] = -((fp + fp0[i]) - 3 * dfdt);
    a[3] = fp - 2 * dfdt;
    double b2 = 2 * a[2];
    double b3 = 6 * a[3];
    function[i] = a[0] + c_dtn * (a[1] * w + b2 * hw2 + b3 * tw3);
    derivatives[i] = a[1] + b2 * w + b3 * hw2;
  }
  t0 = t0 + c_dt * c_nt;
  if (iMax == c_NFitPoints)
    return;
  k = iMax;

  /* Interpolate next c_ntail points (long time steps). */
  x = t0 * c_idt;
  ix = floor(x);
  w = x - ix;
  j = ix; /* j == c_nt */
  j = (j - c_nt) + c_nt * c_ndt;
  w2 = w * w;
  hw2 = 0.5 * w2;
  tw3 = ((1. / 6) * w) * w2;

  /* Number of interpolation points. */
  iMax = k + c_ntail;
  if (iMax > c_NFitPoints)
    iMax = c_NFitPoints;

  /* Fill interpolation points. */
  for (int i = k; i < iMax; ++i) {
    f0[i] = m_FunctionInterpolation[j][0];
    f1[i] = m_FunctionInterpolation[j + 1][0];
    fp0[i] = m_FunctionInterpolation[j][1];
    fp1[i] = m_FunctionInterpolation[j + 1][1];
    j = j + 1;
  }

  /* Interpolation. */
  #pragma omp simd
  for (int i = k; i < iMax; ++i) {
    double a[4];
    double dfdt = (f1[i] - f0[i]) * c_idt;
    double fp = fp1[i] + fp0[i];
    a[0] = f0[i];
    a[1] = fp0[i];
    a[2] = -((fp + fp0[i]) - 3 * dfdt);
    a[3] = fp - 2 * dfdt;
    double b2 = 2 * a[2];
    double b3 = 6 * a[3];
    function[i] = a[0] + c_dt * (a[1] * w + b2 * hw2 + b3 * tw3);
    derivatives[i] = a[1] + b2 * w + b3 * hw2;
  }
  if (iMax == c_NFitPoints)
    return;
  k = iMax;

  /* Exponential tail. */
  while (k < c_NFitPoints) {
    function[k] = function[k - 1] * m_r0;
    derivatives[k] = derivatives[k - 1] * m_r1;
    ++k;
  }
}
