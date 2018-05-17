/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * This module is used to compute the two component (photon+hadron)       *
 * fit to ecl waveforms stored offline.  Hadron component energy          *
 * measured from fit is used to perform pulse shape discrimination        *
 * for particle id.                                                       *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Savino Longo (longos@uvic.ca)                            *
 *               Alexei Sibidanov (sibid@uvic.ca)                         *
 *                                                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// ECL
#include <ecl/modules/eclWaveformFit/ECLWaveformFit.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <ecl/digitization/EclConfiguration.h>
#include <ecl/dataobjects/ECLWaveformData.h>
#include <ecl/dbobjects/ECLDigitWaveformParameters.h>
#include <ecl/dbobjects/ECLDigitWaveformParametersForMC.h>

//ROOT
#include <TMinuit.h>

using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Modules
//-----------------------------------------------------------------
REG_MODULE(ECLWaveformFit)
//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

// constructor
ECLWaveformFitModule::ECLWaveformFitModule()
{
  // Set module properties
  setDescription("Module to fit offline waveforms and measure hadron scintillation component light output.");
  addParam("TriggerThreshold", m_TriggerThreshold,
           "Energy threshold of waveform trigger to ensure corresponding eclDigit is avaliable (GeV).", 0.01);
  addParam("EnergyThreshold", m_EnergyThreshold, "Energy threshold of online fit result for Fitting Waveforms (GeV).", 0.05);
}

// destructor
ECLWaveformFitModule::~ECLWaveformFitModule()
{
}

//callback for loading templates from database
void ECLWaveformFitModule::loadTemplateParameterArray(bool IsDataFlag)
{

  m_TemplatesLoaded = true;

  if (IsDataFlag) {
    //load data templates
    DBObjPtr<ECLDigitWaveformParameters>  WavePars("ECLDigitWaveformParameters");
    std::vector<double>  Ptemp(11), Htemp(11);
    for (int i = 0; i < 8736; i++) {
      for (int j = 0; j < 11; j++) {
        Ptemp[j] = (double)WavePars->getPhotonParameters(i + 1)[j];
        Htemp[j] = (double)WavePars->getHadronParameters(i + 1)[j];
      }
      new(&m_si[i][0]) SignalInterpolation2(Ptemp);
      new(&m_si[i][1]) SignalInterpolation2(Htemp);
    }
  } else {
    //load mc template
    DBObjPtr<ECLDigitWaveformParametersForMC>  WaveParsMC("ECLDigitWaveformParametersForMC");
    std::vector<double>  Ptemp(11), Htemp(11);
    for (int j = 0; j < 11; j++) {
      Ptemp[j] = (double)WaveParsMC->getPhotonParameters()[j];
      Htemp[j] = (double)WaveParsMC->getHadronParameters()[j];
    }
    new(&m_si[0][0]) SignalInterpolation2(Ptemp);
    new(&m_si[0][1]) SignalInterpolation2(Htemp);
  }
}

void ECLWaveformFitModule::beginRun()
{

  m_TemplatesLoaded = false;

  DBObjPtr<ECLCrystalCalib> Ael("ECLCrystalElectronics"), Aen("ECLCrystalEnergy");
  m_ADCtoEnergy.resize(8736);
  if (Ael) for (int i = 0; i < 8736; i++) m_ADCtoEnergy[i] = Ael->getCalibVector()[i];
  if (Aen) for (int i = 0; i < 8736; i++) m_ADCtoEnergy[i] *= Aen->getCalibVector()[i];

}

//adc data array
double FitA[31];

//g_si: photon template signal shape
//g_sih: hadron template signal shape
const SignalInterpolation2* g_si, *g_sih;

//Function to minimize in minuit fit. (chi2)
void FCN2h(int&, double* grad, double& f, double* p, int)
{
  constexpr int N = 31;
  double df[N], da[N];
  const double Ag = p[1], B = p[0], T = p[2], Ah = p[3];
  double chi2 = 0, gAg = 0, gB = 0, gT = 0, gAh = 0;
  const double ErrorPoint = 0.01777777777; //ErrorPoint  =  1./7.5 * 1./7.5  (Error set to +/- 7.5 adc units)

  //getting photon and hadron component shapes for set of fit parameters
  val_der_t ADg[N], ADh[N];
  g_si->getshape(T, ADg);
  g_sih->getshape(T, ADh);

  //computing difference between current fit result and adc data array
  for (int i = 0; i < N; ++i) df[i] = FitA[i] - (Ag * ADg[i].f0 + Ah * ADh[i].f0 + B);

  //computing chi2.  Error set to +/- 7.5 adc units (identity matrix)
  for (int i = 0; i < N; ++i) da[i] = ErrorPoint * df[i];
  for (int i = 0; i < N; ++i) {
    chi2 += da[i] * df[i];
    gB   -= da[i];
    gAg  -= da[i] * ADg[i].f0;
    gT   -= da[i] * (ADg[i].f1 * Ag + ADh[i].f1 * Ah);
    gAh  -= da[i] * ADh[i].f0;
  }

  f = chi2;
  grad[0] = 2 * gB;
  grad[1] = 2 * gAg;
  grad[2] = 2 * gT;
  grad[3] = 2 * gAh;
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

  //flag for callback to load templates each run
  m_TemplatesLoaded = false;
}

void ECLWaveformFitModule::event()
{
  const EclConfiguration& ec = EclConfiguration::get();

  if (!m_TemplatesLoaded) {
    //load templates once per run in first event that has saved waveforms.
    if (m_eclDSPs.getEntries() > 0)  loadTemplateParameterArray(m_eclDSPs[0]->getIsData());
  }

  for (auto& aECLDsp : m_eclDSPs) {

    aECLDsp.setTwoComponentTotalAmp(-1);
    aECLDsp.setTwoComponentHadronAmp(-1);
    aECLDsp.setTwoComponentChi2(-1);
    aECLDsp.setTwoComponentTime(-1);
    aECLDsp.setTwoComponentBaseline(-1);

    const int id = aECLDsp.getCellId() - 1;

    //Filling array with ADC values.
    for (int j = 0; j < ec.m_nsmp; j++) FitA[j] = aECLDsp.getDspA()[j];

    //Trigger check to remove noise pulses in random trigger events.
    //In random trigger events all eclDSP saved but only eclDigits above online threshold are saved.
    //Set 10 MeV threshold for now.
    //Trigger amplitude is computed with algorithm described in slide 5 of:
    //https://kds.kek.jp/indico/event/22581/session/20/contribution/236
    //note the trigger check is a temporary workaround to ensure all eclDsp's have a corresponding eclDigit.
    double baselineADC = 0.25 * (FitA[12] + FitA[13] + FitA[14] + FitA[15]),
           maxADC = 0.5 * (FitA[20] + FitA[21]),
           triggerAmp = (maxADC - baselineADC) * m_ADCtoEnergy[id];
    if (triggerAmp < m_TriggerThreshold) continue;

    //setting relation of eclDSP to aECLDigit
    const ECLDigit* d = NULL;
    for (const auto& aECLDigit : m_eclDigits) {
      if (aECLDigit.getCellId() - 1 == id) {
        d = &aECLDigit;
        aECLDsp.addRelationTo(&aECLDigit);
        break;
      }
    }
    if (d == NULL) continue;

    //Skipping low amplitude waveforms
    if (d->getAmp() * m_ADCtoEnergy[id] < m_EnergyThreshold)  continue;

    //loading template for waveform
    if (aECLDsp.getIsData()) {
      //data cell id dependent
      g_si = &m_si[id][0];
      g_sih = &m_si[id][1];
    } else {
      // mc uses same waveform
      g_si = &m_si[0][0];
      g_sih = &m_si[0][1];
    }

    //Calling optimized fit
    double p2_b, p2_a, p2_t, p2_a1, p2_chi2;
    p2_chi2 = -1;
    Fit2h(p2_b, p2_a, p2_t, p2_a1, p2_chi2);

    //storing fit results
    aECLDsp.setTwoComponentTotalAmp(p2_a + p2_a1);
    aECLDsp.setTwoComponentHadronAmp(p2_a1);
    aECLDsp.setTwoComponentChi2(p2_chi2);
    aECLDsp.setTwoComponentTime(p2_t);
    aECLDsp.setTwoComponentBaseline(p2_b);
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

//Optimized two component fit. Fits photon+hadron component hypothesis
void ECLWaveformFitModule::Fit2h(double& B, double& Ag, double& T, double& Ah, double& amin)
{
  //minuit parameters
  double arglist[10];
  int ierflg = 0;

  // setting inital fit parameters
  double dt = 0.5;
  double amax = 0;
  int jmax = 6;
  for (int j = 0; j < 31; j++) if (amax < FitA[j]) { amax = FitA[j]; jmax = j;}
  double sumB0 = 0; int jsum = 0;
  for (int j = 0; j < 31; j++) if (j < jmax - 3 || jmax + 4 < j) { sumB0 += FitA[j]; ++jsum;}
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

  for (int i = 0; i < c_nt * c_ndt; i++) m_F[i] = t[i];
  for (int i = 0; i < c_ntail; i++) m_F[c_nt * c_ndt + i] = t[c_nt * c_ndt + i * c_ndt];
  const auto& Fm = *(std::end(m_F) - 2), &F0 = *(std::end(m_F) - 1);
  m_r0 = F0.first / Fm.first;
  m_r1 = F0.second / Fm.second;
}

val_der_t SignalInterpolation2::operator()(double t0) const
{
  auto I = [this](double z, int j, double idt, double dt) {
    double z2 = z * z, hz2 = 0.5 * z2, tz3 = ((1. / 6) * z) * z2;
    double a[4],
           f0 = m_F[j].first, f1 = m_F[j + 1].first,
           fp0 = m_F[j].second, fp1 = m_F[j + 1].second,
           dfdt = (f1 - f0) * idt, fp = fp1 + fp0;

    a[0] = f0;
    a[1] = fp0;
    a[2] = -((fp + fp0) - 3 * dfdt);
    a[3] = ((fp) - 2 * dfdt);

    double b2 = 2 * a[2], b3 = 6 * a[3];
    val_der_t y;
    y.f0 = a[0] + dt * (a[1] * z + b2 * hz2 + b3 * tz3);
    y.f1 = a[1] + b2 * z + b3 * hz2;
    y.f2 = (b2 + b3 * z) * idt;
    return y;
  };

  const int iend0 = c_nt * c_ndt, ilast = iend0 + c_ntail - 2, jlast = c_nt + c_ntail - 2;
  double x = t0 * c_idtn, ix = floor(x);
  x -= ix;
  int i = ix;
  val_der_t y;
  if (i < 0) {
    y = {0, 0, 0};
  } else if (i < iend0) {
    y = I(x, i, c_idtn, c_dtn);
  } else {
    double z = t0 * c_idt, jz = floor(z);
    z -= jz;
    int k = jz;
    y = I(z, std::min((k - c_nt) + iend0, ilast), c_idt, c_dt);
    int j = k - jlast;
    while (j-- > 0) {
      y.f0 *= m_r0;
      y.f1 *= m_r1;
    }
  }
  return y;
}

//Evaluates template
void SignalInterpolation2::getshape(double t0, val_der_t* A) const
{
  const int iend0 = c_nt * c_ndt, iend1 = c_nt * c_ndt + c_ntail;
  const val_der_t* Aend = A + 31;
  while (t0 < 0) {
    *A = {0, 0, 0};
    if (++A >= Aend) return;
    t0 += c_dt;
  }

  double x = t0 * c_idtn, ix = floor(x), w = x - ix;
  int i = ix;
  double w2 = w * w, hw2 = 0.5 * w2, tw3 = ((1. / 6) * w) * w2;
  auto I = [this, &w, &w2, &hw2, &tw3](int j, double idt, double dt) {
    double a[4],
           f0 = m_F[j].first, f1 = m_F[j + 1].first,
           fp0 = m_F[j].second, fp1 = m_F[j + 1].second,
           dfdt = (f1 - f0) * idt, fp = fp1 + fp0;

    a[0] = f0;
    a[1] = fp0;
    a[2] = -((fp + fp0) - 3 * dfdt);
    a[3] = ((fp) - 2 * dfdt);

    double b2 = 2 * a[2], b3 = 6 * a[3];
    val_der_t y;
    y.f0 = a[0] + dt * (a[1] * w + b2 * hw2 + b3 * tw3);
    y.f1 = a[1] + b2 * w + b3 * hw2;
    y.f2 = (b2 + b3 * w) * idt;
    return y;
  };

  while (i < iend0) {
    *A = I(i, c_idtn, c_dtn);
    if (++A >= Aend) return;
    i += c_ndt;
    t0 += c_dt;
  }

  x = t0 * c_idt; ix = floor(x); w = x - ix;
  int j = ix;
  i = (j - c_nt) + iend0;
  w2 = w * w, hw2 = 0.5 * w2, tw3 = ((1. / 6) * w) * w2;
  while (i < iend1 - 1) {
    *A = I(i++, c_idt, c_dt);
    if (++A >= Aend) return;
  }

  while (A < Aend) {
    const val_der_t& p = *(A - 1);
    val_der_t& y = *A++;
    y.f0 = p.f0 * m_r0;
    y.f1 = p.f1 * m_r1;
    y.f2 = 0;
  }
}
