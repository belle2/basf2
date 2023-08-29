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


//anonymous namespace for data objects used by both ECLWaveformFitModule class and fcnPhotonHadron funciton for MINUIT minimization.
namespace {

  // Number of fit points.
  const int c_NFitPoints = 31;

  // Number of fit points for vectorized data.
  const int c_NFitPointsVector = 32;

  //adc data array
  double fitA[c_NFitPoints];

  /** Photon template signal shape. */
  const SignalInterpolation2* g_PhotonSignal;

  /** Hadron template signal shape. */
  const SignalInterpolation2* g_HadronSignal;

  /** Noise level. */
  double aNoise;

  //Function to minimize in photon template + hadron template fit. (chi2)
  // cppcheck-suppress constParameter ; TF1 fit functions cannot have const parameters
  void fcnPhotonHadron(int&, double* grad, double& f, double* p, int)
  {
    double df[c_NFitPointsVector];
    double da[c_NFitPointsVector];
    const double Ag = p[1], B = p[0], T = p[2], Ah = p[3];
    double chi2 = 0, gAg = 0, gB = 0, gT = 0, gAh = 0;

    //getting photon and hadron component shapes for set of fit parameters
    double amplitudeGamma[c_NFitPoints], derivativesGamma[c_NFitPoints];
    double amplitudeHadron[c_NFitPoints], derivativesHadron[c_NFitPoints];
    g_PhotonSignal->getShape(T, amplitudeGamma, derivativesGamma);
    g_HadronSignal->getShape(T, amplitudeHadron, derivativesHadron);

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
  void fcnPhotonHadronBackgroundPhoton(int&, double* grad, double& f, double* p, int)
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
    g_PhotonSignal->getShape(T, amplitudeGamma, derivativesGamma);
    // Background photon.
    g_PhotonSignal->getShape(T2, amplitudeGamma2, derivativesGamma2);
    g_HadronSignal->getShape(T, amplitudeHadron, derivativesHadron);

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

ECLWaveformFitModule::ECLWaveformFitModule()
{
  // Set module properties
  setDescription("Module to fit offline waveforms and measure hadron scintillation component light output.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("EnergyThreshold", m_EnergyThreshold, "Energy threshold of online fit result for Fitting Waveforms (GeV).", 0.03);
  addParam("Chi2Threshold25dof", m_Chi2Threshold25dof, "chi2 threshold (25 dof) to classify offline fit as good fit.", 57.1);
  addParam("Chi2Threshold27dof", m_Chi2Threshold27dof, "chi2 threshold (27 dof) to classify offline fit as good fit.", 60.0);
  addParam("CovarianceMatrix", m_CovarianceMatrix,
           "Option to use crystal dependent covariance matrices (false uses identity matrix).", true);
}

ECLWaveformFitModule::~ECLWaveformFitModule()
{
}

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
      new (&m_SignalInterpolation[i][0]) SignalInterpolation2(Ptemp);
      new (&m_SignalInterpolation[i][1]) SignalInterpolation2(Htemp);
      new (&m_SignalInterpolation[i][2]) SignalInterpolation2(Dtemp);
    }
  } else {
    //load mc template
    std::vector<double>  Ptemp(11), Htemp(11), Dtemp(11);
    for (int j = 0; j < 11; j++) {
      Ptemp[j] = (double)m_WaveformParametersForMC->getPhotonParameters()[j];
      Htemp[j] = (double)m_WaveformParametersForMC->getHadronParameters()[j];
      Dtemp[j] = (double)m_WaveformParametersForMC->getDiodeParameters()[j];
    }
    new (&m_SignalInterpolation[0][0]) SignalInterpolation2(Ptemp);
    new (&m_SignalInterpolation[0][1]) SignalInterpolation2(Htemp);
    new (&m_SignalInterpolation[0][2]) SignalInterpolation2(Dtemp);
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

  /* Load covariance matrices from database. */
  if (m_CovarianceMatrix) {
    for (int id = 1; id <= ECLElementNumbers::c_NCrystals; id++) {
      double buf[c_NFitPoints];
      double reg[c_NFitPoints];
      m_AutoCovariance->getAutoCovariance(id, buf);
      double x0 = c_NFitPoints;
      std::memcpy(reg, buf, c_NFitPoints * sizeof(double));
      while (!makecovariance(m_PackedCovariance[id - 1], c_NFitPoints, reg))
        regularize(buf, reg, c_NFitPoints, x0 -= 1, 1);
    }
  } else {
    /* Default covariance matrix is identity for all crystals. */
    double defaultCovariance[c_NFitPoints][c_NFitPoints];
    CovariancePacked packedDefaultCovariance;
    const double isigma = 1 / 7.5;
    for (int i = 0; i < c_NFitPoints; ++i) {
      for (int j = 0; j < c_NFitPoints; ++j) {
        defaultCovariance[i][j] = (i == j) * isigma * isigma;
      }
    }
    int k = 0;
    for (int i = 0; i < c_NFitPoints; i++) {
      for (int j = 0; j < i + 1; j++) {
        packedDefaultCovariance.m_covMatPacked[k] = defaultCovariance[i][j];
        k++;
      }
    }
    ecl_waveform_fit_load_inverse_covariance(
      packedDefaultCovariance.m_covMatPacked);
  }

}

void ECLWaveformFitModule::initialize()
{
  // ECL dataobjects
  m_eclDSPs.registerInDataStore();
  m_eclDigits.registerInDataStore();

  //initializing fit minimizer
  m_MinuitPhotonHadron = new TMinuit(4);
  m_MinuitPhotonHadron->SetFCN(fcnPhotonHadron);
  double arglist[10];
  int ierflg = 0;
  arglist[0] = -1;
  m_MinuitPhotonHadron->mnexcm("SET PRIntout", arglist, 1, ierflg);
  m_MinuitPhotonHadron->mnexcm("SET NOWarnings", arglist, 0, ierflg);
  arglist[0] = 1;
  m_MinuitPhotonHadron->mnexcm("SET ERR", arglist, 1, ierflg);
  arglist[0] = 0;
  m_MinuitPhotonHadron->mnexcm("SET STRategy", arglist, 1, ierflg);
  arglist[0] = 1;
  m_MinuitPhotonHadron->mnexcm("SET GRAdient", arglist, 1, ierflg);
  arglist[0] = 1e-6;
  m_MinuitPhotonHadron->mnexcm("SET EPSmachine", arglist, 1, ierflg);

  //initializing fit minimizer photon+hadron + background photon
  m_MinuitPhotonHadronBackgroundPhoton = new TMinuit(6);
  m_MinuitPhotonHadronBackgroundPhoton->SetFCN(fcnPhotonHadronBackgroundPhoton);
  arglist[0] = -1;
  m_MinuitPhotonHadronBackgroundPhoton->mnexcm("SET PRIntout", arglist, 1, ierflg);
  m_MinuitPhotonHadronBackgroundPhoton->mnexcm("SET NOWarnings", arglist, 0, ierflg);
  arglist[0] = 1;
  m_MinuitPhotonHadronBackgroundPhoton->mnexcm("SET ERR", arglist, 1, ierflg);
  arglist[0] = 0;
  m_MinuitPhotonHadronBackgroundPhoton->mnexcm("SET STRategy", arglist, 1, ierflg);
  arglist[0] = 1;
  m_MinuitPhotonHadronBackgroundPhoton->mnexcm("SET GRAdient", arglist, 1, ierflg);
  arglist[0] = 1e-6;
  m_MinuitPhotonHadronBackgroundPhoton->mnexcm("SET EPSmachine", arglist, 1, ierflg);

  //flag for callback to load templates each run
  m_TemplatesLoaded = false;
}

void ECLWaveformFitModule::event()
{
  const EclConfiguration& ec = EclConfiguration::get();

  if (!m_TemplatesLoaded) {
    /* Load templates once per run in first event that has saved waveforms. */
    if (m_eclDSPs.getEntries() > 0)
      loadTemplateParameterArray();
  }

  for (ECLDsp& aECLDsp : m_eclDSPs) {

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
    for (const ECLDigit& aECLDigit : m_eclDigits) {
      if (aECLDigit.getCellId() - 1 == id) {
        d = &aECLDigit;
        aECLDsp.addRelationTo(&aECLDigit);
        break;
      }
    }
    if (d == nullptr)
      continue;

    //Skipping low amplitude waveforms
    if (d->getAmp() * m_ADCtoEnergy[id] < m_EnergyThreshold)
      continue;

    //loading template for waveform
    if (m_IsMCFlag == 0) {
      //data cell id dependent
      g_PhotonSignal = &m_SignalInterpolation[id][0];
      g_HadronSignal = &m_SignalInterpolation[id][1];
    } else {
      // mc uses same waveform
      g_PhotonSignal = &m_SignalInterpolation[0][0];
      g_HadronSignal = &m_SignalInterpolation[0][1];
    }

    //get covariance matrix for cell id
    if (m_CovarianceMatrix) {
      ecl_waveform_fit_load_inverse_covariance(
        m_PackedCovariance[id].m_covMatPacked);
      aNoise = m_PackedCovariance[id].sigma;
    }

    /* Fit with photon and hadron templates (fit type = 0). */
    double pedestal, amplitudePhoton, signalTime, amplitudeHadron,
           amplitudeBackgroundPhoton, timeBackgroundPhoton, chi2;
    ECLDsp::TwoComponentFitType fitType = ECLDsp::photonHadron;
    chi2 = -1;
    fitPhotonHadron(pedestal, amplitudePhoton, signalTime, amplitudeHadron,
                    chi2);
    aECLDsp.setTwoComponentSavedChi2(ECLDsp::photonHadron, chi2);

    /* If failed, try photon, hadron, and background photon (fit type = 1). */
    if (chi2 >= m_Chi2Threshold27dof) {

      fitType = ECLDsp::photonHadronBackgroundPhoton;
      chi2 = -1;
      fitPhotonHadronBackgroundPhoton(pedestal, amplitudePhoton, signalTime, amplitudeHadron,
                                      amplitudeBackgroundPhoton, timeBackgroundPhoton, chi2);
      aECLDsp.setTwoComponentSavedChi2(ECLDsp::photonHadronBackgroundPhoton,
                                       chi2);

      /* If failed, try diode fit (fit type = 2). */
      if (chi2 >= m_Chi2Threshold25dof) {
        /* Set second component to diode. */
        g_HadronSignal = &m_SignalInterpolation[0][2];
        fitType = ECLDsp::photonDiodeCrossing;
        chi2 = -1;
        fitPhotonHadron(pedestal, amplitudePhoton, signalTime, amplitudeHadron,
                        chi2);
        aECLDsp.setTwoComponentSavedChi2(ECLDsp::photonDiodeCrossing, chi2);

        /* Indicates that all fits tried had bad chi^2. */
        if (chi2 >= m_Chi2Threshold27dof)
          fitType = ECLDsp::poorChi2;
      }

    }

    /* Storing fit results. */
    aECLDsp.setTwoComponentTotalAmp(amplitudePhoton + amplitudeHadron);
    if (fitType == ECLDsp::photonDiodeCrossing) {
      aECLDsp.setTwoComponentHadronAmp(0.0);
      aECLDsp.setTwoComponentDiodeAmp(amplitudeHadron);
    } else {
      aECLDsp.setTwoComponentHadronAmp(amplitudeHadron);
      aECLDsp.setTwoComponentDiodeAmp(0.0);
    }
    aECLDsp.setTwoComponentChi2(chi2);
    aECLDsp.setTwoComponentTime(signalTime);
    aECLDsp.setTwoComponentBaseline(pedestal);
    aECLDsp.setTwoComponentFitType(fitType);
    if (fitType == ECLDsp::photonHadronBackgroundPhoton) {
      aECLDsp.setBackgroundPhotonEnergy(amplitudeBackgroundPhoton);
      aECLDsp.setBackgroundPhotonTime(timeBackgroundPhoton);
    }
  }
}

void ECLWaveformFitModule::endRun()
{
}

void ECLWaveformFitModule::terminate()
{
}

void ECLWaveformFitModule::fitPhotonHadron(
  double& pedestal, double& amplitudePhoton, double& signalTime,
  double& amplitudeHadron, double& chi2)
{
  //minuit parameters
  double arglist[10] = {0};
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
  m_MinuitPhotonHadron->mnparm(0, "B",  B0,    10, B0 / 1.5, B0 * 1.5, ierflg);
  m_MinuitPhotonHadron->mnparm(1, "Ag", A0, A0 / 20,      0,   2 * A0, ierflg);
  m_MinuitPhotonHadron->mnparm(2, "T",  T0,   0.5, T0 - 2.5, T0 + 2.5, ierflg);
  m_MinuitPhotonHadron->mnparm(3, "Ah", 0., A0 / 20,    -A0,   2 * A0, ierflg);

  //perform fit
  arglist[0] = 50000;
  arglist[1] = 1.;
  m_MinuitPhotonHadron->mnexcm("MIGRAD", arglist, 2, ierflg);

  double edm, errdef;
  int nvpar, nparx, icstat;
  m_MinuitPhotonHadron->mnstat(chi2, edm, errdef, nvpar, nparx, icstat);

  //get fit results
  double error;
  m_MinuitPhotonHadron->GetParameter(0, pedestal, error);
  m_MinuitPhotonHadron->GetParameter(1, amplitudePhoton, error);
  m_MinuitPhotonHadron->GetParameter(2, signalTime, error);
  m_MinuitPhotonHadron->GetParameter(3, amplitudeHadron, error);
}

void ECLWaveformFitModule::fitPhotonHadronBackgroundPhoton(
  double& pedestal, double& amplitudePhoton, double& signalTime,
  double& amplitudeHadron, double& amplitudeBackgroundPhoton,
  double& timeBackgroundPhoton, double& chi2)
{
  double arglist[10] = {0};
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
  m_MinuitPhotonHadronBackgroundPhoton->mnparm(0, "B",  B0,    10, B0 / 1.5, B0 * 1.5, ierflg);
  m_MinuitPhotonHadronBackgroundPhoton->mnparm(1, "Ag", A0, A0 / 20,      0,   2 * A0, ierflg);
  m_MinuitPhotonHadronBackgroundPhoton->mnparm(2, "T",  T0,   0.5, T0 - 2.5, T0 + 2.5, ierflg);
  m_MinuitPhotonHadronBackgroundPhoton->mnparm(3, "Ah", 0., A0 / 20,    -A0,   2 * A0, ierflg);
  m_MinuitPhotonHadronBackgroundPhoton->mnparm(4, "A2", A01, A01 / 20,    0,   2 * A01, ierflg);
  m_MinuitPhotonHadronBackgroundPhoton->mnparm(5, "T2", T01,  0.5,    T01 - 2.5,   T01 + 2.5, ierflg);

  // Now ready for minimization step
  arglist[0] = 50000;
  arglist[1] = 1.;
  m_MinuitPhotonHadronBackgroundPhoton->mnexcm("MIGRAD", arglist, 2, ierflg);

  double edm, errdef;
  int nvpar, nparx, icstat;
  m_MinuitPhotonHadronBackgroundPhoton->mnstat(chi2, edm, errdef, nvpar, nparx, icstat);
  double error;
  m_MinuitPhotonHadronBackgroundPhoton->GetParameter(0, pedestal, error);
  m_MinuitPhotonHadronBackgroundPhoton->GetParameter(1, amplitudePhoton, error);
  m_MinuitPhotonHadronBackgroundPhoton->GetParameter(2, signalTime, error);
  m_MinuitPhotonHadronBackgroundPhoton->GetParameter(3, amplitudeHadron, error);
  m_MinuitPhotonHadronBackgroundPhoton->GetParameter(
    4, amplitudeBackgroundPhoton, error);
  m_MinuitPhotonHadronBackgroundPhoton->GetParameter(
    5, timeBackgroundPhoton, error);
}

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
    m_FunctionInterpolation[i] = t[i].first;
    m_DerivativeInterpolation[i] = t[i].second;
  }
  for (int i = 0; i < c_ntail; i++) {
    int j = c_nt * c_ndt + i;
    int k = c_nt * c_ndt + i * c_ndt;
    m_FunctionInterpolation[j] = t[k].first;
    m_DerivativeInterpolation[j] = t[k].second;
  }
  int i1 = c_nt * c_ndt + c_ntail - 2;
  int i2 = c_nt * c_ndt + c_ntail - 1;
  m_r0 = m_FunctionInterpolation[i2] / m_FunctionInterpolation[i1];
  m_r1 = m_DerivativeInterpolation[i2] / m_DerivativeInterpolation[i1];
}

void SignalInterpolation2::getShape(
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
  double function0[c_NFitPoints], function1[c_NFitPoints];
  double derivative0[c_NFitPoints], derivative1[c_NFitPoints];

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
    function0[i] = m_FunctionInterpolation[j];
    function1[i] = m_FunctionInterpolation[j + 1];
    derivative0[i] = m_DerivativeInterpolation[j];
    derivative1[i] = m_DerivativeInterpolation[j + 1];
    j = j + c_ndt;
  }

  /* Interpolation. */
  #pragma omp simd
  for (int i = k; i < iMax; ++i) {
    double a[4];
    double dfdt = (function1[i] - function0[i]) * c_idtn;
    double fp = derivative1[i] + derivative0[i];
    a[0] = function0[i];
    a[1] = derivative0[i];
    a[2] = -((fp + derivative0[i]) - 3 * dfdt);
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
  w2 = w * w;
  hw2 = 0.5 * w2;
  tw3 = ((1. / 6) * w) * w2;

  /* Number of interpolation points. */
  iMax = k + c_ntail - 1;
  if (iMax > c_NFitPoints)
    iMax = c_NFitPoints;

  /* Interpolation. */
  #pragma omp simd
  for (int i = k; i < iMax; ++i) {
    j = c_nt * c_ndt + i - k;
    /*
     * The interpolation step is the same as the distance between
     * the fit points. It is possible to load the values in the interpolation
     * loop while keeping its vectorization.
     */
    double f0 = m_FunctionInterpolation[j];
    double f1 = m_FunctionInterpolation[j + 1];
    double fp0 = m_DerivativeInterpolation[j];
    double fp1 = m_DerivativeInterpolation[j + 1];
    double a[4];
    double dfdt = (f1 - f0) * c_idt;
    double fp = fp1 + fp0;
    a[0] = f0;
    a[1] = fp0;
    a[2] = -((fp + fp0) - 3 * dfdt);
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
