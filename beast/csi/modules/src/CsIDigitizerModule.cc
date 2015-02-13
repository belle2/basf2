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

#include <vector>
#include <TGraph.h>
#include <TRandom3.h>
#include <TVector3.h>
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
CsIDigitizerModule::CsIDigitizerModule() : Module(), m_aDigiHit("DigitizedHit"),
  m_calibConstants(12, 5),
  m_noiseLevels(12, 0.5e-3)
{
  // Set module properties
  setDescription("Digitizer for the BEAST CsI system");

  // Parameter definitions
  addParam("Resolution", m_Resolution, "Resolution (in mV) of the ACD", 4.8828e-4);
  addParam("SampleRate", m_SampleRate, "Sample rate (in samples/sec) of the ADC", 250e6);
}

CsIDigitizerModule::~CsIDigitizerModule()
{
}

void CsIDigitizerModule::initialize()
{

  B2INFO(" Initialize CsI Digitizer ");

  m_aHit.isRequired();
  m_aSimHit.isOptional();
  m_aDigiHit.registerInDataStore();

  //Calculation of the derived paramaters
  m_dt = 1e9 / m_SampleRate;
  m_CsITlSignalTemplate = genSignalTemplate(10240, 1024, m_tCsITl);

  B2INFO("Template  length " << m_CsITlSignalTemplate.size());
  B2INFO("Template  capacity " << m_CsITlSignalTemplate.capacity());;
  B2INFO("dt = " << m_dt);

}

void CsIDigitizerModule::beginRun()
{
  Signal tempo = genTimeSignal(10, 10, 2, 0, 1);
}

void CsIDigitizerModule::event()
{

  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  int m_currentEventNumber = eventMetaDataPtr->getEvent();

  //Loop over CsiHits
  if (m_aSimHit.getEntries() > 0) {
    int hitNum = m_aSimHit.getEntries(); /**< Number of Crystal hits */

    // double E_tmp[16] = {0};       /**< Sum energy deposited in each cell */
    // double edepSum = 0;           /**< Sum energy deposited in all cells */
    double localPos = 0;          /**< Distance between the hit and the PIN-diode end of the crystal (cm).*/
    double  propagTime = 0;          /**< Time when photons from the hit reach the PIN-diode (in ns).*/
    for (int i = 0 ; i < 12 ; i++) {
      m_SimHitTimes[i].clear();
      m_SimHitEdeps[i].clear();
    }


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

      localPos = (15. - (hitPos  - cellPos) * cellAngle);

      // 0.06 is the speed of light in CsI(Tl)
      propagTime = m_SampleRate * (0.0600 * localPos + (tof / CLHEP::ns)) * 1e-9;   ;

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

    if (m_currentEventNumber == 6) {
      int n = m_SimHitTimes[1].size();
      TGraph* gPlot = new TGraph(n, &m_SimHitTimes[1].at(0), &m_SimHitEdeps[1].at(0));
      gPlot->SaveAs("EventOut.root");


      Signal tempo = genTimeSignal(m_SimHitEdeps[1], m_SimHitTimes[1],  1,  m_dt,  10240, true);

    }
  }
}


void CsIDigitizerModule::endRun()
{
}

void CsIDigitizerModule::terminate()
{
}

int  CsIDigitizerModule::doChargeIntegration(Signal _u, int _NsamBL, uint64_t* Q, uint* t,
                                             int _Treshold, double _TriggerHoldoff,
                                             double _GateWidth, double _GateOffset)
{
  vector<int> x = doDigitization(_u, m_Resolution);

  int baseline = 0;

  int max_gated  = (int) floor(_GateWidth / m_dt);
  int gate_offset = (int) floor(_GateOffset / m_dt);
  int max_holdoff = (int) floor(_TriggerHoldoff / m_dt);

  int charge    = 0;
  int n_holdoff = 0;
  int n_gated   = 0;

  bool gate = false;
  bool holdoff = false;
  bool triggered = false;
  bool stop = false;
  bool trigger = false;

  // Find trigger position
  int i = 0;
  int iFirstTrigger = 0;
  vector<int>::iterator it;
  for (it = x.begin(); (it != x.end()) && !stop; ++it, ++i) {
    if (!gate && !holdoff) {
      for (vector<int>::iterator itbl = it ; (itbl > (it - _NsamBL)) && itbl != x.begin(); --itbl) {
        baseline += *itbl;
      }

      trigger = (*it - baseline) > _Treshold;

      //first time we see a trigger
      if (trigger && !triggered && !iFirstTrigger)
        iFirstTrigger = i;

    } else {

      if (gate) {
        charge += (*(it - gate_offset) - baseline);
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

  *Q = charge;
  *t = (uint) iFirstTrigger;

  return i; //Where to resume in case more events per trace
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

Signal CsIDigitizerModule::genTimeSignal(Signal _energies, Signal _times,  int _iChannel, int _dt, int _nsam, bool _save)
{

  double invdt = 1.0 / _dt;

  double tf = 0;
  double t0 = 1e9;
  int timeIndex = 0;

  TRandom3 noise(m_seed);

  for (Signal::iterator it = _times.begin() ; it != _times.end(); ++it) {
    if (*it < t0)
      t0 = *it;

    if (*it > tf)
      tf = *it;
  }

  cout << "well, time interval is " << t0 << " -- " << tf << endl;
  //_nsam = (int) (tf-t0)*invdt;
  cout << "nample is " << _nsam << endl;

  Signal edepos(_nsam, 0.0);

  int i = 0;
  int ioffset = 2048;
  for (Signal::iterator it = _times.begin() ; it != _times.end(); ++it, ++i) {
    timeIndex = ((int)(*it - t0) * invdt + noise.Gaus(1, 20)) + ioffset;
    edepos.at(timeIndex) += _energies.at(i);
  }

  cout << "here" << endl;

  Signal LightDep = firstOrderResponse(edepos, 0, _dt, m_tCsITl);
  Signal V = firstOrderResponse(LightDep, 0, _dt, m_tRisePMT, m_tTransitPMT);

  for (Signal::iterator it = V.begin() ; it != V.end(); ++it) {
    *it *= m_calibConstants[_iChannel];
    *it += noise.Gaus(0, m_noiseLevels[_iChannel]);
    *it  = floor(*it * (1.0 /  m_Resolution));
  }

  cout << "there" << endl;

  if (_save) {
    Signal t(_nsam, 0);
    t.at(0) = t0;

    for (int i = 1; i < _nsam; i++)
      t.at(i) = t.at(i - 1) + _dt;

    cout << "or there" << endl;

    TGraph gPlot1(_nsam, &t[0], &edepos[0]);
    gPlot1.SaveAs("EdeposOut.root");

    TGraph gPlot2(_nsam, &t[0], &LightDep[0]);
    gPlot2.SaveAs("LightDepoOut.root");

    TGraph gPlot3(_nsam, &t[0], &V[0]);
    gPlot3.SaveAs("VOut.root");
  }

  return V;
}

Signal CsIDigitizerModule::genTimeSignal(double _energy, double _timeAvg, double _timeRMS,  int iChannel, bool _save)
{
  TRandom3 noise(m_seed);

  int i0 = floor(_timeAvg / m_dt);
  int ir = floor(_timeRMS / m_dt);
  int n = 10240;

  Signal _u(n, 0);

  static const double invroottwopi = 1.0 / sqrt(2 * PI);
  double invrms = 1.0 / ir;

  for (int i = 0; i < n; i++) {
    _u.at(i) = invrms * invroottwopi * exp(-0.5 * pow((i - i0) * invrms, 2)) ;
  }

  Signal LightDep = firstOrderResponse(_u, 0, m_dt, m_tCsITl);
  Signal V = firstOrderResponse(LightDep, 0, m_dt, m_tRisePMT, m_tTransitPMT);

  for (Signal::iterator it = V.begin() ; it != V.end(); ++it) {
    *it *= (_energy * m_calibConstants[iChannel]);
    *it += noise.Gaus(0, m_noiseLevels[iChannel]);
  }

  Signal t(n, 0);
  for (int i = 1; i < n; i++)
    t[i] = t[i - 1] + m_dt;

  if (_save) {
    TGraph* gPlot = new TGraph(n, &t[0], &V[0]);
    gPlot->SaveAs("ScopeOut.root");
  }

  return V;
}

Signal CsIDigitizerModule::genSignalTemplate(int _n, int _i0, double _t1, double _t2, double _rFastTot)
{
  Signal _u(_n, 0);
  for (int i = 0; i < 500; i++) {
    _u.at(_i0 + i) = -1;
  }
  Signal y1 = firstOrderResponse(_u, 0, m_dt, _t1);

  if (_rFastTot >= 1)
    return y1;

  Signal y2 = firstOrderResponse(_u, 0, m_dt, _t2);


  Signal yout(y1);
  int n = yout.size();
  for (int i = 0; i < n; i++) {
    yout[i] = _rFastTot * y1[i] + (1.0 - _rFastTot) * y2[i];

  }
  return yout;
}

/*
  int CsiDigitizerModule::addNoise(Signal * y, double rms, int seed)
  {
  TRandom3 noise(seed);
  for (Signal::iterator it = y->begin() ; it != y->end(); ++it)
  *it += noise.Gaus(0,rms);

  return y->size();
  }
*/


Signal CsIDigitizerModule::firstOrderResponse(Signal _u, double _y0, double _dt, double _tau, double _delay)
{
  // First skip everything if the time constant in infinitely short.
  if (_tau == 0)
    return _u;

  int n = _u.size();
  Signal y;
  double k[4] = {0};

  static const double invSix = 1.0 / 6.0;
  double invtau = 1.0 / _tau;
  double y_max = 0.0;
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
    k[0] = f(i      , _u[i], _u[j], y[i]              , invtau);
    k[1] = f(i + 0.5, _u[i], _u[j], y[i] + 0.5 * _dt * k[0], invtau);
    k[2] = f(i + 0.5, _u[i], _u[j], y[i] + 0.5 * _dt * k[1], invtau);
    k[3] = f(j      , _u[i], _u[j], y[i] +     _dt * k[2], invtau);

    y.push_back(y[i] + _dt * invSix * (k[0] + 2 * k[1] + 2 * k[2] + k[3]));

    if (fabs(y.back()) > y_max) y_max = fabs(y.back());
  }


  /*

  //Normalize the response peak (remember this is only a template fo a signal..)
  double inv_max = 1.0 / y_max;
  for (Signal::iterator it = y.begin() ; it != y.end(); ++it)
    *it *= inv_max;
    */

  return y;
}


inline double CsIDigitizerModule::f(double fi, double u_i, double u_j, double y, double invtau)
{
  //linear interpolation of the input at fractional index fi
  double u = u_i * (fi - floor(fi)) + u_j * (ceil(fi) - fi);

  return u - invtau * y;
}

