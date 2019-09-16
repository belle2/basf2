/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: beaulieu                                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/csi/geometry/CsiGeometryPar.h>
#include <beast/csi/modules/CsIDigitizerModule.h>
#include <beast/csi/dataobjects/CsiDigiHit.h>
#include <beast/csi/dataobjects/CsiSimHit.h>
#include <beast/csi/dataobjects/CsiHit.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

#include <stdint.h> //< For fixed-size integers
#include <vector>
#include <TFile.h>
#include <TGraph.h>
#include <TH1F.h>
#include <TH1I.h>
#include <framework/core/RandomNumbers.h>
#include <TVector3.h>
#include <TVectorD.h>
#include <math.h>

#define PI 3.14159265358979323846

using namespace std;
using namespace Belle2;
using namespace csi;
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CsIDigitizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

//Calibration constants (hard-coded for now, put in the xml file for later)
CsIDigitizerModule::CsIDigitizerModule() : Module(), m_hitNum(0),
  m_TrueEdep(0.0),
  m_nWFcounter(0),
  m_aDigiHit("CsiDigiHits"),
  m_calibConstants(16, 5),
  m_noiseLevels(16, 0.25e-3),
  m_LY(16, 40e6),
  m_tRatio(16, 0),
  m_tFast(16, 1),
  m_tSlow(16, 1),
  m_LCE(16, 0.1),
  m_PmtQE(16, 0.05),
  m_PmtGain(16, 1e5)
{
  // Set module properties
  setDescription("Digitizer for the BEAST CsI system");

  // Parameter definitions
  addParam("Resolution", m_Resolution, "Resolution (in mV) of the ACD", 4.8828e-4);
  addParam("SampleRate", m_SampleRate, "Sample rate (in samples/sec) of the ADC", 250e6);
  addParam("nWaveforms", m_nWaveforms, "Number of waveforms to save. 0: none, -1: all ", 0);
}

CsIDigitizerModule::~CsIDigitizerModule()
{
}

void CsIDigitizerModule::initialize()
{

  B2DEBUG(100, "Initializing ");

  //m_aHit.isRequired();
  m_aSimHit.isRequired();
  m_aDigiHit.registerInDataStore();

  //Calculation of the derived paramaters
  m_dt = 1e9 / m_SampleRate;
  setnSamples(8192);

  //Get crystal and PMT constants from xml files
  CsiGeometryPar* csip = CsiGeometryPar::Instance();
  for (uint i = 0; i != m_LY.size(); ++i) {
    csip->Print(i, 80);

    m_LY.at(i)     = 1;//1e3 * csip->GetMaterialProperty(i, "SCINTILLATIONYIELD");
    m_tFast.at(i)  = 1;//csip->GetMaterialProperty(i, "FASTTIMECONSTANT");
    m_tSlow.at(i)  = 1;//csip->GetMaterialProperty(i, "SLOWTIMECONSTANT");
    m_tRatio.at(i) = 1;//csip->GetMaterialProperty(i, "YIELDRATIO");
  }

  //Operate the pure CsI at higher gain to compensate for lower light yield
  for (int i = 8; i < 16; i++) {
    m_PmtGain[i] = 35e5;
  }

}

void CsIDigitizerModule::beginRun()
{
  //Signal tempo = genTimeSignal(10, 10, 2, 0, 1);
}

void CsIDigitizerModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  int m_currentEventNumber = eventMetaDataPtr->getEvent();

  B2DEBUG(80, "Digitingevent  " << m_currentEventNumber);

  //Loop over CsiSimHits
  if (m_aSimHit.getEntries() > 0) {
    int hitNum = m_aSimHit.getEntries(); /**< Number of Crystal hits */

    // double E_tmp[16] = {0};       /**< Sum energy deposited in each cell */
    // double edepSum = 0;           /**< Sum energy deposited in all cells */

    for (int i = 0 ; i < 16 ; i++) {
      m_SimHitTimes[i].clear();
      m_SimHitEdeps[i].clear();
    }


    B2DEBUG(150, "Looping over CsISimHits");
    for (int i = 0; i < hitNum; i++) { // Loop over CsISimHits
      CsiSimHit* aCsISimHit = m_aSimHit[i];
      int m_cellID = aCsISimHit->getCellId();       /**< Index of the Cell*/
      double edep = aCsISimHit->getEnergyDep();     /**< Energy deposited in the current hit */
      double tof = aCsISimHit->getFlightTime();
      //      double hitTime = aCsISimHit->getTimeAve();    /**< Time average of the hit*/
      //      double hitTimeRMS = sqrt( aCsISimHit->getTimeVar()/aCsISimHit->getEnergyDep());    /**< Time rms of the hit*/
      CsiGeometryPar* csip = CsiGeometryPar::Instance();

      TVector3 hitPos    = aCsISimHit->getPosition();
      TVector3 cellPos   = csip->GetPositionTV3(m_cellID);
      TVector3 cellAngle = csip->GetOrientationTV3(m_cellID);

      double localPos = (15. - (hitPos  - cellPos) *
                         cellAngle);  /**< Distance between the hit and the PIN-diode end of the crystal (cm).*/

      // 0.06 is the speed of light in CsI(Tl)
      double  propagTime = m_SampleRate *
                           (0.0600 * localPos + (tof / CLHEP::ns)) * 1E-9; /**< Time when photons from the hit reach the PIN-diode (in ns).*/


      m_SimHitTimes[m_cellID].push_back(propagTime);
      m_SimHitEdeps[m_cellID].push_back(edep);

      /*
      if (i<10){
      B2INFO("Hit No = : " << i );
      B2INFO("Deposited energy = : " << edep );
      B2INFO("Average time = : " << hitTime );
      B2INFO("Time RMS = : " << hitTimeRMS );

      csip->Print(m_cellID);
      }
      */

    }

    for (int iCh = 0; iCh < 16; iCh++) {

      int n = m_SimHitTimes[iCh].size();

      if (n > 0) {

        B2DEBUG(140, "Generating Time signal");
        Signal tempSignal;
        m_TrueEdep = genTimeSignal(&tempSignal, m_SimHitEdeps[iCh], m_SimHitTimes[iCh], iCh,  m_dt,  m_nSamples, false);


        B2DEBUG(140, "Launching Charge integration");
        bool  recordWaveform = false;
        if ((m_nWaveforms == -1) || (m_nWFcounter < m_nWaveforms)) {
          m_nWFcounter++;
          recordWaveform = true;
          B2DEBUG(80, "Recording WF");
        }
        uint16_t max = doChargeIntegration(tempSignal, 128, &m_Baseline, &m_Charge, &m_Time, &m_Waveform,
                                           &m_DPPCIBits,  5, 1.2e4, 1e4, 1e3, recordWaveform);

        if (m_Charge > 0) {
          m_aDigiHit.appendNew();
          m_hitNum = m_aDigiHit.getEntries() - 1;
          m_aDigiHit[m_hitNum]->setCellId(iCh);
          m_aDigiHit[m_hitNum]->setCharge(m_Charge);
          m_aDigiHit[m_hitNum]->setTime(m_Time);
          m_aDigiHit[m_hitNum]->setBaseline(m_Baseline);
          m_aDigiHit[m_hitNum]->setTrueEdep(m_TrueEdep);

          m_aDigiHit[m_hitNum]->setMaxVal(max);

          m_aDigiHit[m_hitNum]->setWaveform(&m_Waveform);
          m_aDigiHit[m_hitNum]->setStatusBits(&m_DPPCIBits);
        }
      }
    }
  }
}


void CsIDigitizerModule::endRun()
{
}

void CsIDigitizerModule::terminate()
{
}

uint16_t  CsIDigitizerModule::doChargeIntegration(Signal _u, int _NsamBL, uint16_t* _BSL, uint32_t* _Q,
                                                  uint32_t* _t, vector<uint16_t>* _Waveform,
                                                  vector<uint8_t>* _DPPCIBits, int _Treshold,
                                                  double _TriggerHoldoff, double _GateWidth,
                                                  double _GateOffset, bool _recordTraces)
{

  B2DEBUG(80, "Arguments: " << &_u << ", " << _NsamBL << ", " << _BSL << ", " << _Q  << ", " <<  _t
          << ", " << _Waveform << ", " <<  _Treshold  << ", " <<  _TriggerHoldoff  << ", " << _GateWidth
          << ", " << _GateOffset << ", " <<  _recordTraces);

  vector<int> x = doDigitization(_u, m_Resolution);
  int nSam = x.size();

  // Plots for debugging (see CAEN DPP-CI figs 2.2,2.3
  TH1I h_trigger("h_trigger", "Trigger", nSam, 0, nSam - 1);
  TH1I h_gate("h_gate", "Gate", nSam, 0, nSam - 1);
  TH1I h_holdoff("h_holdoff", "Holdoff", nSam, 0, nSam - 1);
  TH1I h_baseline("h_baseline", "Baseline", nSam, 0, nSam - 1);
  TH1I h_charge("h_charge", "Charge", nSam, 0, nSam - 1);
  TH1F h_signal("h_signal", "Continous signal", nSam, 0, nSam - 1);
  TH1I h_digsig("h_digsig", "Digital signal", nSam, 0, nSam - 1);


  int max_gated  = (int) floor(_GateWidth / m_dt);
  int gate_offset = (int) floor(_GateOffset / m_dt);
  int max_holdoff = (int) floor(_TriggerHoldoff / m_dt);

  int baseline  = 0;
  int charge    = 0;
  int n_holdoff = 0;
  int n_gated   = 0;

  bool gate = false;
  bool holdoff = false;
  bool stop = false;
  bool trigger = false;

  // Find trigger position
  int i = 0;
  int iFirstTrigger = 0;
  list<int> baselineBuffer;
  vector<int>::iterator it;
  float tempBaseline = 0.0;

  // Saving inverse number of samples used for baseline averaging (avoid division)
  const double invMaxNavgBL = 1.0 / _NsamBL; /**< 1 / N_samples used for baseline averaging for most of the signal*/
  double invNavgBL = 0.0;                   /**< 1 / N_samples used for baseline averaging (at beginning of signal)*/

  _Waveform->resize(nSam, 0);
  _DPPCIBits->resize(nSam, 0);

  B2DEBUG(140, "Scanning vector: all should have nSam=" << nSam);

  uint16_t maxval = 0;

  for (it = x.begin(); (it != x.end()); ++it, ++i) {

    if (*it > maxval)
      maxval = *it;

    _Waveform->at(i) = *it;
    _DPPCIBits->at(i) = trigger + (gate << 1) + (holdoff << 2) + (stop << 3);

    if (_recordTraces) {

      h_trigger.Fill(i, (int) trigger)  ;
      h_gate.Fill(i, (int) gate)  ;
      h_holdoff.Fill(i, (int) holdoff)  ;
      h_baseline.Fill(i, baseline)  ;
      h_charge.Fill(i, charge)  ;
      h_digsig.Fill(i, *it)  ;
      h_signal.Fill(i, _u.at(i))  ;
    }

    if (!gate && !holdoff) {

      baselineBuffer.push_back(*it);


      if ((i + 1)  >  _NsamBL) {
        baselineBuffer.pop_front();
        invNavgBL = invMaxNavgBL;
      } else {
        invNavgBL = (1.0 / i);
      }

      tempBaseline = 0;
      for (list<int>::iterator itbl = baselineBuffer.begin(); itbl != baselineBuffer.end(); ++itbl)
        tempBaseline += (float) * itbl;

      tempBaseline *= invNavgBL;

      baseline = (int) round(tempBaseline);

      trigger = (*it - baseline) > _Treshold;

      //first time we see a trigger
      if (trigger && !iFirstTrigger)
        iFirstTrigger = i;

    } else {

      if (gate) {
        charge += (*(it - gate_offset) - baseline);
        *_BSL = baseline;
        n_gated++;
      }

      if (holdoff) {
        n_holdoff++;
      }

      trigger = false;
    }

    holdoff = trigger || (holdoff && (n_holdoff < max_holdoff));
    gate    = trigger || (gate    && (n_gated  < max_gated));

    stop = iFirstTrigger && !gate && !holdoff;
  }

  *_Q = charge;
  // from the doc: 2 sample uncertainty.
  *_t = (uint)(iFirstTrigger  + 2.0 * (gRandom->Rndm() - 0.5));

  if (not(_recordTraces)) {
    _Waveform->clear();
    _DPPCIBits->clear();
  } else {
    //Below is obsolete: recording now done in the study module
    /*
      char rootfilename[100];
      sprintf(rootfilename, "output/BEAST/plots/dpp-ci_WF%i.root", m_nWFcounter);

      B2INFO("Writing to " << rootfilename);
      TFile fs(rootfilename, "recreate");

      h_trigger.Write();
      h_gate.Write();
      h_holdoff.Write();
      h_baseline.Write();
      h_charge.Write();
      h_signal.Write();
      h_digsig.Write();

      TVectorD Edep(1);
      Edep[0] = m_TrueEdep;
      Edep.Write("Edep");

      fs.Close();
    */
  }

  return maxval;
}

vector<int>  CsIDigitizerModule::doDigitization(Signal _v, double _LSB)
{
  vector<int>  output(_v.size(), 0);
  double invLSB = 1.0 / _LSB;

  int i = 0;
  for (Signal::iterator it = _v.begin() ; it != _v.end(); ++it, ++i) {
    output.at(i)  = (int) round(*it * invLSB);
  }

  return  output;
}

double CsIDigitizerModule::genTimeSignal(Signal* _output, Signal _energies, Signal _times, int _iChannel, int _dt, int _nsam,
                                         bool _save)
{

  double invdt = 1.0 / _dt;

  double tf = 0;
  double t0 = 1e9;
  double sumEnergies = 0.0;

  for (Signal::iterator it = _times.begin() ; it != _times.end(); ++it) {
    if (*it < t0)
      t0 = *it;

    if (*it > tf)
      tf = *it;
  }

  Signal edepos(_nsam, 0.0);

  int i = 0;
  int ioffset = floor(_nsam * 0.25);
  B2DEBUG(150, "Filling edepos vector. Container length is " << _nsam);

  for (Signal::iterator it = _times.begin() ; it != _times.end(); ++it, ++i) {
    sumEnergies += _energies.at(i);
    // time index +/- 1 time bin
    int timeIndex = ((int)(*it - t0) * invdt  + ioffset);
    if ((timeIndex - 1) > (int) edepos.size()) {
      B2WARNING(" genTimeSignal: TimeIndex greater than length of signal container. Skipping deposit of " << _energies.at(i) << "GeV");
    } else {
      edepos.at(timeIndex) += _energies.at(i);
    }
  }

  B2DEBUG(80, "Generating time responses for channel " << _iChannel);
  B2DEBUG(80, "    Fast time constant  " << m_tFast[_iChannel]);
  B2DEBUG(80, "    Slow time constant  " << m_tSlow[_iChannel]);
  /*
    Signal Qcathode = firstOrderResponse(m_LY[_iChannel] * m_LCE[_iChannel] * m_PmtQE[_iChannel],
    edepos, 0, _dt, m_tSlow[_iChannel], 0.0, m_tRatio[_iChannel], m_tFast[_iChannel]);
  */

  Signal Qcathode = firstOrderResponse((1 - m_tRatio[_iChannel]) * m_LY[_iChannel] * m_LCE[_iChannel] * m_PmtQE[_iChannel], edepos, 0,
                                       _dt, m_tSlow[_iChannel], 0.0);


  if (m_tRatio[_iChannel]) {
    Signal QcathodeF = firstOrderResponse(m_tRatio[_iChannel] * m_LY[_iChannel] * m_LCE[_iChannel] * m_PmtQE[_iChannel], edepos, 0, _dt,
                                          m_tFast[_iChannel], 0.0);

    int j = 0;
    for (Signal::iterator it = Qcathode.begin() ; it != Qcathode.end(); ++it, ++j)
      *it += QcathodeF.at(j);
  }

  Signal Vanode   = firstOrderResponse(1.602e-10 * m_Zl * invdt * m_PmtGain[_iChannel], Qcathode, 0, _dt, m_tRisePMT, m_tTransitPMT);

  i = 0;
  B2DEBUG(150, "Adding noise. Container length is " << Vanode.size());
  addNoise(&Vanode, m_noiseLevels[_iChannel], 5e-3 * gRandom->Rndm() + 1e-2);

  if (_save) {
    Signal t(_nsam, 0);
    t.at(0) = t0;

    for (i = 1; i < _nsam; i++)
      t.at(i) = t.at(i - 1) + _dt;

    TGraph gPlot1(_nsam, &t[0], &edepos[0]);
    gPlot1.SaveAs("EdeposOut.root");

    TGraph gPlot2(_nsam, &t[0], &Qcathode[0]);
    gPlot2.SaveAs("QOut.root");

    TGraph gPlot3(_nsam, &t[0], &Vanode[0]);
    gPlot3.SaveAs("VOut.root");
  }

  *_output = Vanode;

  return sumEnergies;
}


int CsIDigitizerModule::addNoise(Signal* y, double _rms, double  _offset)
{
  for (Signal::iterator it = y->begin() ; it != y->end(); ++it)
    *it += _offset + gRandom->Gaus(0, _rms);

  return y->size();
}

/*
Signal CsIDigitizerModule::firstOrderResponse(double _gain, Signal _u, double _y0, double _dt, double _tSlow, double _delay, double _tRatio, double _tFast)
{

  Signal slowLight = firstOrderResponse(_gain*(1-_tRatio), _u, _y0, _dt, _tFast, _delay);

  if (_tRatio>0) {
    Signal fastLight = firstOrderResponse(_gain*   _tRatio , _u, _y0, _dt, _tFast, _delay);
    int i=0;
     for (Signal::iterator it = slowLight.begin() ; it != slowLight.end(); ++it, ++i)
          *it += fastLight.at(i);
  }
  B2WARNING("You shouldn't see this!");
  return slowLight;

}
*/

Signal CsIDigitizerModule::firstOrderResponse(double _gain, Signal _u, double _y0, double _dt, double _tau, double _delay)
{

  B2DEBUG(80, "Generating 1st order response with arguments");
  B2DEBUG(80, "       _gain: " << _gain);
  B2DEBUG(80, "  length(_u): " << _u.size());
  B2DEBUG(80, "         _y0: " << _y0);
  B2DEBUG(80, "         _dt: " << _dt);
  B2DEBUG(80, "        _tau: " << _tau);
  B2DEBUG(80, "      _delay: " << _delay);


  // First skip everything if the time constant in infinitely short.
  if (_tau == 0)
    return _u;

  int n = _u.size();
  Signal y;
  double k[4] = {0};

  static const double invSix = 1.0 / 6.0;
  double invtau = 1.0 / _tau;
  y.push_back(_y0);

  // Apply delay to input
  int n_delay = (int) round(_delay / _dt);
  Signal::iterator it = _u.begin();
  double _u_0 = _u.front();
  _u.insert(it , n_delay, _u_0);
  _u.resize(n);

  // Apply that input to the good old Runge-Kutta 4 routine.
  for (int i = 0, j = 0; i < (n - 1); i++) {
    j = i + 1;
    k[0] = f(i      , _u[i], _u[j], y[i]                   , invtau);
    k[1] = f(i + 0.5, _u[i], _u[j], y[i] + 0.5 * _dt * k[0], invtau);
    k[2] = f(i + 0.5, _u[i], _u[j], y[i] + 0.5 * _dt * k[1], invtau);
    k[3] = f(j      , _u[i], _u[j], y[i] +       _dt * k[2], invtau);

    y.push_back(y[i] + _dt * invSix * (k[0] + 2 * k[1] + 2 * k[2] + k[3]));
  }


  // Apply gain
  if (_gain != 1) {
    for (Signal::iterator it2 = y.begin() ; it2 != y.end(); ++it2)
      *it2 *= _gain;
  }

  return y;
}


double CsIDigitizerModule::f(double fi, double u_i, double u_j, double y, double invtau)
{
  //linear interpolation of the input at fractional index fi
  double u = u_i * (fi - floor(fi)) + u_j * (ceil(fi) - fi);

  return u - invtau * y;
}

