/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

    /** Designed to hold a "continuous" (in time and amplitude) signal  */
    typedef std::vector<double> Signal;
    /** Designed to hold a "digital" (limited by the DAQ board rate and resolution) signal  */
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
      virtual void initialize() override;

      /** To do before each runs. NOOP.*/
      virtual void beginRun() override;

      /** Each event
       * This is where the actual digitization is done,
       * and the hits are written to the DataStore
       */
      virtual void event() override;

      /** Clean up. NOOP. */
      virtual void endRun() override;

      /** Final clean up. CPU clock stops */
      virtual void terminate() override;


    private:

      /** Generates a time signal for a mean energy deposit
       * The energy deposit is modelled at a Gaussian whose
       * parameters are given as inputs.
       *
       * @param _energy TO BE COMPLETED
       * @param _timeAvg TO BE COMPLETED
       * @param _timeRMS TO BE COMPLETED
       * @param iChannel TO BE COMPLETED
       * @param _save TO BE COMPLETED
       */
      Signal genTimeSignal(double _energy, double _timeAvg, double _timeRMS, int iChannel, bool _save = 0);

      /** Generates a time signal for a set of hits.
       * The hits correspond to all CsiSimHits of  a single event,
       * each are recorded at specific times so the energies and times
       * vectors are given as inputs.
       *
       * @param _output TO BE COMPLETED
       * @param _energies TO BE COMPLETED
       * @param _times TO BE COMPLETED
       * @param _iChannel TO BE COMPLETED
       * @param _dt TO BE COMPLETED
       * @param _nsam TO BE COMPLETED
       * @param _save TO BE COMPLETED
       */
      double genTimeSignal(Signal* _output, Signal _energies, Signal _times,  int _iChannel, int _dt, int _nsam, bool _save = 0);

      /** Generates the template for a signal (obsolete)
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
       * @param _gain: Overall gain applied to the ouput
       * @param _u: The input to the system
       * @param _y0: Initial value of the output
       * @param _dt: Integration step (in ns)
       * @param _tSlow: Time constant of the system
       * @param _delay: Time delay of the system (in ns)
       *
       * @return a Signal corresponding tot he time response
       */
      Signal firstOrderResponse(double _gain, Signal _u, double _y0, double _dt, double _tSlow, double _delay);


      /** Method to use when crystal has two light components
       * @param _tRatio: For crystal with two light components: ratio fast light / slow light
       * @param _tFast: For crystal with two light components: fast time constant
       */
      //      Signal firstOrderResponse(double _gain, Signal _u, double _y0, double _dt, double _tSlow, double _delay, double _tRatio, double _tFast);



      /** Realizes the charge integration of the input signal
       *
       * @param _u: The input signal in Volts
       * @param _NsamBL: Number of samples to conduct the baseline measurement (8,32,128)
       * @param BSL: A pointer to the value of the baseline (in LSB)
       * @param Q: A pointer to the value holding the total charge (in LSB.sample)
       * @param t: A pointer to the value holding the trigger time
       * @param _Waveform: A pointer to the vector holding all waveform data points
       * @param _DPPCIBits: A pointer to the vector holding the bits of the DPP-CI status. Order [MSB-LSB] is [stop,holdoff,gate,trigger]
       * @param _Treshold: Treshold above which generate a trigger (in LSB)
       * @param _TriggerHoldoff: Width of signal integration (in ns)
       * @param _GateWidth: Width of signal integration (in ns)
       * @param _GateOffset: Width of signal integration (in ns)
       * @param _recordTraces: Record traces
       *
       * @return The maximum ADC value (to check for saturations)
       */
      uint16_t  doChargeIntegration(Signal _u, int _NsamBL, uint16_t* BSL, uint32_t* Q, uint32_t* t,
                                    std::vector<uint16_t>* _Waveform, std::vector<uint8_t>* _DPPCIBits,
                                    int _Treshold, double _TriggerHoldoff = 0.0, double _GateWidth = 320.0,
                                    double _GateOffset = 40.0, bool _recordTraces = false);


      /** Adds noise to the signal
       *
       * @param y: a pointer to the signal to noisify
       * @param _rms: the rms of the high-frequency noise to add
       * @param _offset: the offset to apply to the signal (e.g. to simulate low-frew noise)
       *
       * @return The number of samples in the signal
       */
      int addNoise(Signal* y, double _rms, double  _offset);

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




      /** Gets the number of points in the waveforms arrays
       */
      int getnSamples() const { return m_nSamples; }

      /** Sets the number of points in the waveforms arrays
       */
      void setnSamples(int nsamples) { m_nSamples = nsamples; }


    private:

      /** index of csiHit */
      int    m_hitNum;

      // Member data objects
      double m_TrueEdep;          /**< Sum of the MC (true) deposited energies in the event-channel */
      double m_Resolution;  /**< Parameter: Resolution (in mV) of the ACD */
      double m_SampleRate;  /**< Parameter: Sample rate (in samples/sec) of the ADC */
      double m_dt;          /**< Time interval (in ns) (calculated from m_SampleRate */
      int m_nWaveforms;     /**< Number of waveforms to save. 0: none, -1: all */
      int m_nWFcounter;     /**< Counter for the number of waveforms to save*/


      uint8_t   m_CellId;          /**< Cell ID */
      uint16_t  m_Baseline;        /**< Baseline (pedestal) frozen during charge integration */
      uint32_t  m_Charge;          /**< Integrated Charge */
      uint16_t  m_MaxADC;          /**< Max ACD of the hit (to check saturations) */
      uint32_t  m_Time;            /**< Trigger Time */
      int       m_nSamples;               /**< Number of points requested in the waveform arrays */
      std::vector<uint16_t>  m_Waveform;  /**< Saved waveform*/
      std::vector<uint8_t>   m_DPPCIBits; /**< status of the DPP-CI */

      const double m_tRisePMT = 2;   /**< 2.6 Rise time of the PMT signal (in ns) */
      const double m_tTransitPMT = 48; /**< 48Mean transit time of the PMT signal (in ns) */
      const double m_Zl = 50;    /**< Line impedance of the analog chain (to get voltage from anode current) */

      StoreArray<CsiHit>     m_aHit; /**<  Time-accumulated hits... deprecated      */
      StoreArray<CsiSimHit>  m_aSimHit; /**< Each simulated particle in the crystal */
      StoreArray<CsiDigiHit> m_aDigiHit; /**< Output: a digitized hit */

      Signal m_CsITlSignalTemplate; /**< Template Signal of a CsITl trace */

      std::vector<double> m_calibConstants; /**< Calibration constants for each channel (in V/keV)*/
      std::vector<double> m_noiseLevels;    /**< Noise level for each channel (in V)*/
      std::vector<double> m_LY;  /**< Light yield for each channel (gamma per GeV) */
      std::vector<double> m_tRatio; /**< Ratio fast light / slow light for each channel */
      std::vector<double> m_tFast; /**< Fast time constant for each channel (ns) */
      std::vector<double> m_tSlow; /**< Slow time constant for each channel (ns) */
      std::vector<double> m_LCE; /**< Light collection efficiency for each channel */
      std::vector<double> m_PmtQE; /**<PMT quantum efficiency for each channel */
      std::vector<double> m_PmtGain; /**< PMT gain for each channel */

      Signal m_SimHitTimes[16]; /**< Array of signals (each corresponding to one channel) */
      Signal m_SimHitEdeps[16];  /**< Array of signals (each corresponding to one channel) */

    };

  }
}

#endif /* CSIDIGITIZERMODULE_H */
