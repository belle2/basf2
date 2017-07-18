/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Peter Kvasnicka                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDClusterizerModule_H
#define SVDClusterizerModule_H

#include <framework/core/Module.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/dataobjects/VxdID.h>
#include <svd/reconstruction/ClusterCandidate.h>
#include <svd/reconstruction/ClusterCache.h>
#include <svd/reconstruction/NoiseMap.h>
#include <svd/reconstruction/Sample.h>
#include <string>
#include <deque>
#include <set>
#include <boost/array.hpp>
#include <cmath>

namespace Belle2 {

  namespace SVD {

    /** SVDClusterizerModule: The SVD Clusterizer.
     *
     * This module produces clusters from SVDDigits (signal samples taken on
     * individual strips) by first performing 2D clustering in strip coordinate and
     * time, same as in PXD, and then using a waveform fit to determine the initial
     * time of the waveform in the time coordinate and center-of-gravity or analog
     * head-tail to determine the spatial coordinate.
     */
    class SVDClusterizerModule : public Module {

    public:
      /** Container to sort the digits by strip number and time */
      typedef std::set<Sample> SensorSide;
      /** Container to hold the data of u and v strips of a sensor. */
      typedef boost::array<SensorSide, 2> Sensor;
      /** Structure to hold the data of all SVD sensors. */
      typedef std::map<VxdID, Sensor > Sensors;
      /** Iterator type for the map of sensors. */
      typedef std::map<VxdID, Sensor >::iterator SensorIterator;

      /** Constructor defining the parameters */
      SVDClusterizerModule();

      /** Initialize the module */
      virtual void initialize() override;

      /** do the clustering */
      virtual void event() override;

    protected:

      /** Find the cluster a given sample belongs to.
       * For this to work correctly, the samples have to be passed sorted by
       * sensor,strip direction, strip number, and time.
       * @param px sample for which the cluster is sought.
       */
      void findCluster(const Sample& sample);

      /** Write a cluster to the collection. */
      void writeClusters(VxdID sensorID, int side);

      /** Write a cluster to DataStore.
       * Use time fit to get cluster time and amplitudes.
       * This version is only experimental and temporary,
       * the whole module will be re-done to get rid of 2D clustering.
       */
      void writeClustersWithTimeFit(VxdID sensorID, int side);

      // Data members
      //1. Collections
      /** Name of the collection to use for the SVDDigits */
      std::string m_storeDigitsName;
      /** Name of the collection to use for the SVDClusters */
      std::string m_storeClustersName;
      /** Name of the collection to use for the SVDTrueHits */
      std::string m_storeTrueHitsName;
      /** Name of the collection to use for the MCParticles */
      std::string m_storeMCParticlesName;
      /** Name of the relation between SVDDigits and MCParticles */
      std::string m_relDigitMCParticleName;
      /** Name of the relation between SVDClusters and MCParticles */
      std::string m_relClusterMCParticleName;
      /** Name of the relation between SVDClusters and SVDDigits */
      std::string m_relClusterDigitName;
      /** Name of the relation between SVDDigits and SVDTrueHits */
      std::string m_relDigitTrueHitName;
      /** Name of the relation between SVDClusters and SVDTrueHits */
      std::string m_relClusterTrueHitName;

      // 2. Physics
      /** Lorentz angle, electrons.
       * FIXME: These parameters are no longer used to calculate Lorentz shifts,
       * and will be removed shortly.
       */
      double m_tanLorentzAngle_electrons;
      /** LorentzAngle, holes. */
      double m_tanLorentzAngle_holes;

      //3. Noise and ADC
      /** Noise in e- on u (short) strips, read from SensorInfo */
      double m_elNoiseU;
      /** Noise in e- on v (long) strips, read from SensorInfo */
      double m_elNoiseV;

      //4. Clustering
      /** Seed cut in units of m_elNoise. */
      double m_cutSeed;
      /** Noise (cluster member) cut in units of m_elNoise. */
      double m_cutAdjacent;
      /** Cluster cut in units of m_elNoise */
      double m_cutCluster;
      /** Size of the cluster at which we switch from Center of Gravity to Analog Head Tail */
      int m_sizeHeadTail;
      /** Minimum number of significant consecutive samples in a strip signal. Set to 3 in the constructor. */
      const unsigned int c_minSamples;
      /** Time tolerance for clustering: max. mean square distance between maxima in a cluster.*/
      double m_timeTolerance;
      /** Use time ftter? */
      bool m_useFitter;

      // 5. Timing
      // The decay times are shaping times plus typical diffusion time."
      /** Typical decay time for signals of electrons.*/
      double m_shapingTimeElectrons;
      /** Typical decay time for signal of holes. */
      double m_shapingTimeHoles;
      /** Interval between two consecutive signal samples (30 ns). */
      double m_samplingTime;
      /** Base (zero) time of APV25. */
      double m_refTime;
      /** Whether or not to apply a time window cut */
      bool   m_applyWindow;
      /** Time of the trigger. */
      double m_triggerTime;
      /** Acceptance window size.
       * Only clusters with initial time within the acceptance window,
       * (m_triggerTime, m_triggerTime+m_acceptance) will be accepted.
       */
      double m_acceptance;
      /** Rejection level for the fitter - reject clusters with p < pReject of being within acceptance window */
      double m_rejectionLevel;
      /** Likelihood threshold based on rejection level:
       * A cluster will be rejected if neg.loglikelihood(window) - neg.loglikelihood(fitted_time) > threshold)
       */
      double m_rejectionThreshold;

      /** Pointer to the geometry info of the currently active Sensor */
      SensorInfo* m_geometry;

      /** Assume the SVDDigits are sorted by sensor and strip direction. */
      bool m_assumeSorted;

      /** Type of Cluster Charge Calculation (0: use maximum charge sample, 1: use quadratic fit) */
      int m_calClsCharge;

      /** List of all cluster candidates */
      std::deque<ClusterCandidate> m_clusters;

      /** cache of the last seen clusters to speed up clustering */
      ClusterCache m_cache;

      /** Noisemap for the currently active sensor and sensor side. */
      NoiseMap m_noiseMap;

    };//end class declaration


  } //end SVD namespace;
} // end namespace Belle2

#endif // SVDClusterizerModule_H
