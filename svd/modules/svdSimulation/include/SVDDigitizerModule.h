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
#include <svd/calibration/SVDNoiseCalibrations.h>
#include <svd/calibration/SVDPulseShapeCalibrations.h>
#include <svd/calibration/SVDChargeSimulationCalibrations.h>
#include <svd/calibration/SVDFADCMaskedStrips.h>
#include <svd/online/SVDOnlineToOfflineMap.h>
#include <framework/database/PayloadFile.h>
#include <svd/dataobjects/SVDEventInfo.h>
#include <svd/calibration/SVDDetectorConfiguration.h>
#include <framework/dbobjects/HardwareClockSettings.h>

#include <string>

#include <root/TVector3.h>
#include <root/TFile.h>
#include <root/TTree.h>
#include <root/TH1D.h>
#include <root/TH2F.h>

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
       * @param carriers number of electrons and holes to drift
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

      /** Save signals to a root-delimited file (to be analyzed in Python).
       * This method is only called when a name is set for the file.
       */
      void saveSignals();

      /** Initialize the module and check module parameters */
      virtual void initialize() override;
      /** Initialize the list of existing SVD Sensors */
      virtual void beginRun() override;
      /** Digitize one event */
      virtual void event() override;
      /** Terminate the module */
      virtual void terminate() override;

    protected:

      // Members holding module parameters:

      // 1. Collections
      /** Name of the collection for the MCParticles */
      std::string m_storeMCParticlesName = "";
      /** Name of the collection for the SVDSimhits */
      std::string m_storeSimHitsName = "";
      /** Name of the collection for the SVDTrueHits */
      std::string m_storeTrueHitsName = "";
      /** Name of the relation between MCParticles and SVDSimHits */
      std::string m_relMCParticleSimHitName = "";
      /** Name of the relation between SVDTrueHits and SVDSimHits */
      std::string m_relTrueHitSimHitName = "";
      /** Name of the collection for the SVDShaperDigits */
      std::string m_storeShaperDigitsName = "";
      /** Name of the relation between SVDShaperDigits and MCParticles */
      std::string m_relShaperDigitMCParticleName = "";
      /** Name of the relation between SVDShaperDigits and SVDTrueHits */
      std::string m_relShaperDigitTrueHitName = "";
      /** Name of the SVDEventInfo object */
      std::string m_svdEventInfoName = "SVDEventInfoSim";

      // 2. Physics
      /** Max. Segment length to use for charge drifting */
      double m_segmentLength = 0.020;
      /** Width of diffusion cloud for simple drift model (in sigmas) */
      double m_widthOfDiffusCloud = 3.0;

      // 3. Noise
      /** Whether or not to apply poisson fluctuation of charge (Fano factor)*/
      bool  m_applyPoisson = true;
      /** Zero-suppression cut. */
      double m_SNAdjacent = 3.0;
      /** Round ZS cut to nearest ADU */
      bool m_roundZS = true;
      /** Keek digit if at least m_nSamplesOverZS are over threshold */
      unsigned short m_nSamplesOverZS = 1;
      /** (derived from SNAdjacent) Fraction of noisy strips per sensor. */
      double m_noiseFraction = 0.01;

      // 4. Timing
      /** Hardware Clocks*/
      DBObjPtr<HardwareClockSettings> m_hwClock;
      /** Shaping time of the APV25 shapers.*/
      double m_shapingTime = 250.0;
      /** Interval between two waveform samples, by default taken from HardwareClockSettings */
      double m_samplingTime = -1;
      /** Randomize event times?
       * If set to true, event times will be randomized uniformly from
       * m_minTimeFrame to m_maxTimeFrame.
       */
      bool m_randomizeEventTimes = false;
      /** Low edge of randomization time frame */
      float m_minTimeFrame = -300;
      /** High edge of randomization time frame */
      float m_maxTimeFrame = 150;
      /** Current event time.
       * This is what gets randomized if m_randomizeEventTimes is true.
       */
      float m_currentEventTime = 0.0;
      /** number of digitized samples
       * read from SVDEventInfo
       */
      int m_nAPV25Samples = 6;

      /** Time window start, excluding trigger bin effect.
       * This is the parameter used to tune the latency wrt L1 trigger.
       */
      double m_startSampling = -2;
      /** Time window start, including the triggerBin effect.
       * Starting from this time, signal samples are taken in samplingTime intervals.
       */
      double m_initTime = 0;

      // 5. Reporting
      /** Name of the ROOT filename to output statistics */
      std::string m_rootFilename = "";
      /** Store waveform data in the reporting file? */
      bool m_storeWaveforms = false;
      /** Name of the tab-delimited listing of signals */
      std::string m_signalsList = "";


      // 6. 3-mixed-6 and 3-sample daqMode
      /** True if the event should be simulated with 3 sample */
      bool m_is3sampleEvent = false;

      // Other data members:

      /** Structure containing signals in all existing sensors */
      Sensors m_sensors;

      /** Pointer to the SVDSimhit currently digitized */
      const SVDSimHit*   m_currentHit = nullptr;
      /** Index of the particle which caused the current hit */
      int                m_currentParticle = -1;
      /** Index of the TrueHit the current hit belongs to */
      int                m_currentTrueHit = -1;
      /** Pointer to the sensor in which the current hit occurred */
      Sensor*            m_currentSensor = nullptr;
      /** Pointer to the SensorInfo of the current sensor */
      const SensorInfo*  m_currentSensorInfo = nullptr;
      /** Time of the current SimHit.. */
      double m_currentTime = 0;
      /** Thickness of current sensor (read from m_currentSensorInfo).*/
      double m_sensorThickness = 0.03;

      /** relative shift in SVDEventInfo obj */
      int m_relativeShift = 0;
      /** Starting sample for the selection of 3 samples in 3-mixed-6 */
      int m_startingSample = 0;

      /** return the starting sample */
      int getFirstSample(int triggerBin, int relativShift);

      //MC payloads:
      SVDFADCMaskedStrips m_MaskedStr; /**< FADC masked strip payload*/
      static std::string m_xmlFileName /**< channel mapping xml filename*/;
      DBObjPtr<PayloadFile> m_mapping; /**<channel mapping payload*/
      std::unique_ptr<SVDOnlineToOfflineMap> m_map; /**<channel mapping map*/
      SVDDetectorConfiguration m_svdConfig; /**< svd configuration parameters */

      SVDChargeSimulationCalibrations m_ChargeSimCal; /**<SVDChargeSimulationCalibrations calibrations db object*/
      SVDNoiseCalibrations m_NoiseCal; /**<SVDNoise calibrations db object*/
      SVDPulseShapeCalibrations m_PulseShapeCal; /**<SVDPulseShapeCalibrations calibrations db object*/

      // ROOT stuff:
      /** Pointer to the ROOT filename for statistics */
      TFile* m_rootFile = nullptr;
      /** Histogram showing the charge sharing + diffusion in u (r-phi). */
      TH1D*  m_histChargeSharing_u = nullptr;
      /** Histogram showing the charge sharing + diffusion in v (z). */
      TH1D*  m_histChargeSharing_v = nullptr;

      /** Histogram showing the mobility of e-. */
      TH1D*  m_histMobility_e = nullptr;
      /** Histogram showing the mobility of h. */
      TH1D*  m_histMobility_h = nullptr;
      /** Histogram showing the velocity of e-. */
      TH1D*  m_histVelocity_e = nullptr;
      /** Histogram showing the velocity of h. */
      TH1D*  m_histVelocity_h = nullptr;
      /** Histogram showing the distance to plane for e. */
      TH1D*  m_histDistanceToPlane_e = nullptr;
      /** Histogram showing the distance to plane for h. */
      TH1D*  m_histDistanceToPlane_h = nullptr;
      /** Histogram showing the drift time of e. */
      TH1D*  m_histDriftTime_e = nullptr;
      /** Histogram showing the drift time of h. */
      TH1D*  m_histDriftTime_h = nullptr;
      /** Histogram showing the hit time. */
      TH1D*  m_histHitTime = nullptr;
      /** Histogram showing the hit time vs TB. */
      TH2F*  m_histHitTimeTB = nullptr;

      /** Histogram showing the Lorentz angles in u (r-phi). */
      TH1D*  m_histLorentz_u = nullptr;
      /** Histogram showing the Lorentz angles in v (z). */
      TH1D*  m_histLorentz_v = nullptr;
      /** Histogram showing the distribution of digit signals in u (r-phi).*/
      TH1D*  m_signalDist_u = nullptr;
      /** Histogram showing the distribution of digit signals in v (z).*/
      TH1D*  m_signalDist_v = nullptr;
      /** Tree for waveform storage. */
      TTree* m_waveTree = nullptr;

    };//end class declaration


  } // end namespace SVD
} // end namespace Belle2

#endif // SVDDigitizerModule_H
