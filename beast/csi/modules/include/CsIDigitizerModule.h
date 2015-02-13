/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: beaulieu                                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CSIDIGITIZERMODULE_H
#define CSIDIGITIZERMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <beast/csi/dataobjects/CsiDigiHit.h>
#include <beast/csi/dataobjects/CsiSimHit.h>
#include <beast/csi/dataobjects/CsiHit.h>

#include <vector>



namespace Belle2 {
  namespace csi {

    typedef std::vector<double> Signal;
    typedef std::vector<int> DigitalSignal;

    /**
     * Digitizer for the BEAST CsI system
     *
    * This modules reads CsiHits for deposited energy, associates a pulse shape then fits it for amplitude and time. In a future version: calculate pulse shape from sim hits.   *
    */

    class CsIDigitizerModule : public Module {

    public:

      /**
       * Constructor: Sets the description, the properties and the parameters of the module.
       */
      CsIDigitizerModule();

      /** Default destructor */
      virtual ~CsIDigitizerModule();

      /** Register input and output data */
      virtual void initialize();

      /** To do before each runs. NOOP.*/
      virtual void beginRun();

      /** Each event
       * This is where the actual digitization is done,
       * and the hits are written to the DataStore
       */
      virtual void event();

      /** Clean up. NOOP. */
      virtual void endRun();

      /** Final clean up. CPU clock stops */
      virtual void terminate();


    private:

      /** Generates a time signal for a given hit on a specific channel
       *
       * param _energy: energy of the hit in GeV
       * param _timeAvg: time at which the hit occured in ns (average weighted by energy deposited)
       * param _timeRMS: RMS of all hit times in the event in ns
       * param iChannel: Index of the channel (same as cellID)
       * param _save: Set to 1 to save the trase in a root file (for debug purposes)
       */
      Signal genTimeSignal(double _energy, double _timeAvg, double _timeRMS, int iChannel, bool _save = 0);

      Signal genTimeSignal(Signal _energies, Signal _times,  int _iChannel, int _dt, int _nsam, bool _save = 0);

      /** Generates the template for a signal
       *
       * param _n: number of time samples in the signal to create
       * param _i0: index of the first input
       * param _t1: Fast time constant (in ns)
       * param _t2: Slow time constant (in ns)
       * param _rFastTot: Ratio of the fast light  to the total light
       */
      Signal genSignalTemplate(int _n, int _i0, double _t1, double _t2 = 0.0, double _rFastTot = 1.0);


      /** Calculates the time response of a first order system (such as crystal, PMT, etc)
       *
       * param _u: The input to the system
       * param _y0: Initial value of the output
       * param _dt: Integration step (in ns)
       * param _tau: Time constant of the system
       * param _delay: Time delay of the system (in ns)
       */
      Signal firstOrderResponse(Signal _u, double _y0, double _dt, double _tau, double _delay = 0.0);

      /** Realizes the charge integration of the input signal
       *
       * param _u: The input signal in Volts
       * param _NsamBL: Number of samples to conduct the baseline measurement (8,32,128)
       * param _Treshold: Treshold above which generate a trigger (in LSB)
       * param _PreTrigger: time to record before trigger (in ns)
       * param _TriggerHoldoff: Width of signal integration (in ns)
       * param _GateWidth: Width of signal integration (in ns)
       * param _GateOffset: Width of signal integration (in ns)
       *
       * returnthe index of after the end of the holdoff (where to pick up if we have more peaks in the signal)
       */
      int  doChargeIntegration(Signal _u, int _NsamBL, uint64_t* Q, uint* t, int _Treshold,
                               double _PreTrigger, double _TriggerHoldoff = 0.0,
                               double _GateWidth = 320.0, double _GateOffset = 40.0);

      /** Digitizes the signal the signal
       *
       * param _y: The input signal in Volts
       * param _LSB: The value of a LSB in volts (=resolution)
       *
       * return a std<vector> containing the signal in LSB
       */
      DigitalSignal  doDigitization(Signal _v, double _LSB);


      /** This returns the RHS of first order differential equation
       *
       * param fi: index at which we should evaluate the function.
       *           Do linear interpolation of the input in case fi isn't an integer
       * param u_i: input at index i
       * param u_j: input at index j=i+1
       * param y  : output function
       * param invtau: 1 / (time constant)
       */
      double f(double fi, double u_i, double u_j, double y, double invtau);


      // Member data objects
      double m_Resolution;  /**< Parameter: Resolution (in mV) of the ACD */
      double m_SampleRate;  /**< Parameter: Sample rate (in samples/sec) of the ADC */
      double m_dt;          /**< Time interval (in ns) (calculated from m_SampleRate */


      const double m_tRisePMT = 2.6;
      const double m_tTransitPMT = 48;
      const double m_tCsITl = 1220;
      const double m_tCsIslow = 30;
      const double m_tCsIfast =  6;

      const int m_seed = 0;
      StoreArray<CsiHit> m_aHit; /**<  The result of each incoming particle in a crystal */
      StoreArray<CsiSimHit>  m_aSimHit; /**< Each simulated particle in the crystal */
      StoreArray<CsiDigiHit> m_aDigiHit; /**< Output: a digitized hit */

      Signal m_CsITlSignalTemplate; /**< Template Signal of a CsITl trace */
      Signal m_calibConstants; /**< Calibration constants for each channel (in V/keV)*/
      Signal m_noiseLevels; /**< Noise level for each channel (in V)*/

      Signal m_SimHitTimes[12]; /**< Array of signals (each corresponding to one channel) */
      Signal m_SimHitEdeps[12];  /**< Array of signals (each corresponding to one channel) */
    };

  }
}

#endif /* CSIDIGITIZERMODULE_H */
