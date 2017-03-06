/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Zbynek Drasal, Martin Ritter, Peter Kvasnicka            *
 *                                                                        *
 **************************************************************************/

#ifndef SVDDigitizerModule_H
#define SVDDigitizerModule_H

#include <framework/core/Module.h>
#include <svd/dataobjects/SVDSimHit.h>
#include <svd/simulation/SVDSignal.h>
#include <svd/geometry/SensorInfo.h>
#include <framework/dataobjects/RelationElement.h>
#include <string>
#include <set>
#include <vector>

#include <root/TVector3.h>
#include <root/TFile.h>
#include <root/TTree.h>
#include <root/TH1D.h>
#include <root/TH2D.h>

namespace Belle2 {
  namespace SVD {

    /** Map of all signals in one sensor. */
    typedef std::map<short int, SVDSignal> StripSignals;

    /** Signals of u- and v- strips in one sensor. */
    typedef std::pair<StripSignals, StripSignals> Sensor;

    /** Map of all signals in all sensors */
    typedef std::map<VxdID, Sensor> Sensors;



    /** The SVD Digitizer module.
     * This module is responsible for converting the simulated energy
     * deposition from Geant4 into real SVD detector response of single strips.
     */

    class SVDDigitizerModule : public Module {
    public:
      /** Constructor.  */
      SVDDigitizerModule();

      /** Process one SVDSimHit by dividing the step in smaller steps and drifting the charge */
      void processHit();

      /** Drift the charge inside the silicon.
       * This method will drift the charge inside the silicon along the E/B fieldlines.
       * @param position start position of the charge
       * @param electrons number of electrons and holes to drift
       * @param carrierType electrons or holes
       */
      void driftCharge(const TVector3& position, double carriers, SVD::SensorInfo::CarrierType carrierType);

      /** Calculate the noise contribution to one strip with given charge.
       * @param charge the original charge on the strip
       * @param noise the standard RMS noise on the strip
       * @return the new charge of the strip.
       */
      double addNoise(double charge, double noise);

      /** Save digits to the DataStore
       * Saves samples of generated waveforms.
       */
      void saveDigits();

      /** Save waveforms to the statistics file.
       * This method is only called when storage of waveforms is required.
       */
      void saveWaveforms();

      /** Initialize the module and check module parameters */
      virtual void initialize();
      /** Initialize the list of existing SVD Sensors */
      virtual void beginRun();
      /** Digitize one event */
      virtual void event();
      /** Terminate the module */
      virtual void terminate();

    protected:

      /** A small helper function to convert between electons and ADU */
      inline double eToADU(double charge) const
      {
        return round(std::min(m_maxADC, std::max(m_minADC, charge)) / m_unitADC);
      }
      // Members holding module parameters:

      // 1. Collections
      /** Name of the collection for the MCParticles */
      std::string m_storeMCParticlesName;
      /** Name of the collection for the SVDDigits */
      std::string m_storeDigitsName;
      /** Name of the collection for the SVDSimhits */
      std::string m_storeSimHitsName;
      /** Name of the collection for the SVDTrueHits */
      std::string m_storeTrueHitsName;
      /** Name of the relation between MCParticles and SVDSimHits */
      std::string m_relMCParticleSimHitName;
      /** Name of the relation between SVDDigits and MCParticles */
      std::string m_relDigitMCParticleName;
      /** Name of the relation between SVDTrueHits and SVDSimHits */
      std::string m_relTrueHitSimHitName;
      /** Name of the relation between SVDDigits and SVDTrueHits */
      std::string m_relDigitTrueHitName;

      // 2. Physics
      /** Max. Segment length to use for charge drifting */
      double m_segmentLength;
      /** Width of diffusion cloud for simple drift model (in sigmas) */
      double m_widthOfDiffusCloud;

      // 3. Noise
      /** Whether or not to apply poisson fluctuation of charge */
      bool   m_applyPoisson;
      /** Whether or not to apply Gaussian noise */
      bool  m_applyNoise;
      /** Zero-suppression cut. */
      double m_SNAdjacent;
      /** (derived from SNAdjacent) Fraction of noisy strips per sensor. */
      double m_noiseFraction;

      // 4. Timing
      /** Shaping time of the APV25 shapers.*/
      double m_shapingTime;
      /** Interval between two waveform samples (30 ns). */
      double m_samplingTime;
      /** Whether or not to apply a time window cut */
      bool   m_applyWindow;
      /** Time window start.
       * Starting from this time, signal samples are taken in samplingTime intervals.
       */
      double m_startSampling;
      /** Number of samples
       * Number of consecutive APV25 samples
       */
      int m_nAPV25Samples;
      /** Whether or not to apply random phase sampling.
       * If set to true, the first samples of the event will be taken at a random time point
       * with probability centered around the time when first particle reaches
       * the SVD. */
      bool m_randomPhaseSampling;

      // 5. Processing
      /** Whether or not to apply discrete ADC on output values. */
      bool   m_applyADC;
      /** Low end of ADC range in e-. */
      double m_minADC;
      /** High end of ADC range in e-. */
      double m_maxADC;
      /** Number of ADC bits. */
      int m_bitsADC;
      /** adu in electrons (derived from the above). */
      double m_unitADC;

      // 6. Reporting
      /** Name of the ROOT filename to output statistics */
      std::string m_rootFilename;
      /** Store waveform data in the reporting file? */
      bool m_storeWaveforms;


      // Other data members:

      /** Structure containing signals in all existing sensors */
      Sensors m_sensors;

      /** Pointer to the SVDSimhit currently digitized */
      const SVDSimHit*   m_currentHit;
      /** Index of the particle which caused the current hit */
      int                m_currentParticle;
      /** Index of the TrueHit the current hit belongs to */
      int                m_currentTrueHit;
      /** Pointer to the sensor in which the current hit occurred */
      Sensor*            m_currentSensor;
      /** Pointer to the SensorInfo of the current sensor */
      const SensorInfo*  m_currentSensorInfo;
      /** Time of the current detector event, from the currently processed SimHit.. */
      double             m_currentTime;

      /** Thickness of current sensor (read from m_currentSensorInfo).*/
      double m_sensorThickness;
      /** The depletion voltage of the Silicon sensor */
      double m_depletionVoltage;
      /** The bias voltage on the sensor */
      double m_biasVoltage;
      /** The backplane capacitanceU wrt. the strips. */
      double m_backplaneCapacitanceU;
      /** The interstrip capacitanceU for the sensor. */
      double m_interstripCapacitanceU;
      /** The coupling capacitanceU for the sensor. */
      double m_couplingCapacitanceU;
      /** The backplane capacitanceV wrt. the strips. */
      double m_backplaneCapacitanceV;
      /** The interstrip capacitanceV for the sensor. */
      double m_interstripCapacitanceV;
      /** The coupling capacitanceV for the sensor. */
      double m_couplingCapacitanceV;
      /** Electronic noise for u-strips. */
      double m_elNoiseU;
      /** Electronic noise for v-strips. */
      double m_elNoiseV;

      // ROOT stuff:
      /** Pointer to the ROOT filename for statistics */
      TFile* m_rootFile;
      /** Histogram showing the diffusion cloud in u (r-phi). */
      TH1D*  m_histDiffusion_u;
      /** Histogram showing the diffusion cloud in v (z). */
      TH1D*  m_histDiffusion_v;
      /** Histogram showing the Lorentz angles in u (r-phi). */
      TH1D*  m_histLorentz_u;
      /** Histogram showing the Lorentz angles in v (z). */
      TH1D*  m_histLorentz_v;
      /** Histogram showing the distribution of digit signals in u (r-phi).*/
      TH1D*  m_signalDist_u;
      /** Histogram showing the distribution of digit signals in v (z).*/
      TH1D*  m_signalDist_v;
      /** Tree for waveform storage. */
      TTree* m_waveTree;

    };//end class declaration


  } // end namespace SVD
} // end namespace Belle2

#endif // SVDDigitizerModule_H
