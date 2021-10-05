/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <cdc/geometry/CDCGeometryPar.h>
#include <calibration/CalibrationCollectorModule.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>
#include <tracking/trackFindingCDC/topology/CDCWire.h>

#include <framework/dataobjects/EventT0.h>
#include <framework/datastore/StoreArray.h>

#include <TVector3.h>
#include <string>

namespace Belle2 {
  namespace CDC {

    /**
     * Collect hit information for cdc calibration with CAF
     */
    class CDCCalibrationCollectorModule : public CalibrationCollectorModule {

    public:

      /**
       * Constructor.
       */
      CDCCalibrationCollectorModule();

      /**
       * Destructor.
       */
      virtual ~CDCCalibrationCollectorModule();

      /**
       * Initializes the Module.
       */
      void prepare() override;


      /**
       * Event action, collect information for calibration.
       */

      void collect() override;

      /**
       * Termination action.
       */
      void finish() override;

    private:


      /**
       * collect hit information of fitted track.
       */
      void harvest(Belle2::RecoTrack* track);
      /**
      * fills efficiency objects
      */
      void buildEfficiencies(std::vector<unsigned short> wireHits, const Helix helixFit);
      /**
       * extrapolates the helix fit to a given layer and finds the wire which it would be hitting
       */
      const TrackFindingCDC::CDCWire& getIntersectingWire(const TVector3& xyz, const TrackFindingCDC::CDCWireLayer& layer,
                                                          const Helix& helixFit) const;

      StoreObjPtr<EventT0> m_eventTimeStoreObject;  /**< Event t0 object */
      StoreArray<Track> m_Tracks;                   /**< Tracks. */
      StoreArray<RecoTrack> m_RecoTracks;           /**< Tracks. */
      StoreArray<TrackFitResult> m_TrackFitResults; /**< Track fit results. */
      StoreArray<CDCHit> m_CDCHits;                 /**< CDC hits. */
      TrackFindingCDC::StoreWrappedObjPtr<std::vector<TrackFindingCDC::CDCTrack>> m_CDCTracks; /**< CDC tracks. */

      std::string m_trackArrayName;           /**< Belle2::Track StoreArray name. */
      std::string m_cdcHitArrayName;          /**< Belle2::CDCHit StoreArray name*/
      std::string m_cdcTrackVectorName = "CDCTrackVector";       /**< Belle2::CDCTrack vectorpointer name*/
      std::string m_recoTrackArrayName ;      /**< Belle2::RecoTrack StoreArray nam.e */
      std::string m_trackFitResultArrayName;  /**< Belle2::TrackFitResult StoreArray name. */
      std::string m_relRecoTrackTrackName;    /**< Relation between RecoTrack and Belle2:Track. */
      std::string m_treeName;                 /**< Name of tree for the output file. */
      std::string m_effTreeName;              /**< Name of efficiency tree for the output file. */

      Float_t weight;            /**<  Weight of hit.*/
      Float_t alpha;             /**< Entrance Azimuthal angle of hit (degree). */
      Float_t theta;             /**< Entrance Polar angle of hit (degree). */
      UShort_t adc;              /**< adc value. */
      Float_t t;                 /**< Measurement Drift time. */
      Float_t t_fit;             /**< Drift time calculated from x_fit. */
      Float_t evtT0;             /**< event T0*/
      Float_t x_mea;             /**< measure drift length (signed by left right).*/
      Float_t x_u;               /**< X_fit for unbiased track fit.*/
      Float_t x_b;               /**< X_fit for biased track fit.*/
      UChar_t lay;              /**< Layer ID. */
      UShort_t IWire;           /**< Wire ID. */
      Float_t Pval;              /**< P-value of fitted track.  */
      Float_t ndf;               /**< degree of freedom. */
      Float_t d0;                /**< Track Parameter, d0. */
      Float_t z0;                /**< Track Parameter, z0. */
      Float_t phi0;              /**< Track Parameter, phi0. */
      Float_t tanL;              /**< Track Parameter, tanL. */
      Float_t omega;             /**< Track Parameter, omega. */
      double m_minimumPt = 0;   /**< minimum pt required for track*/
      double m_minimumNDF = 0;   /**< minimum NDF required for track*/

      bool m_calExpectedDriftTime = true;      /**< Calculate expected drift time from x_fit or not. */
      bool m_bField = true;                    /**< fit incase no magnetic Field of not, if false, NDF=4 in cal P-value */
      bool m_storeTrackParams = true;          /**< Store Track parameter or not. */
      bool m_eventT0Extraction = true;         /**< use Event T0 extract t0 or not. */
      bool m_isCosmic = false;                 /**< true when we process cosmic events, else false (collision). */
      bool m_effStudy = false;                 /**< When true module collects info only necessary for wire eff study. */

      unsigned short wireID;                   /**< wireID for hit-level wire monitoring */
      unsigned short layerID;                  /**< layerID for hit-level wire monitoring */
      float z;                                 /**< z of hit fot hit-level wire monitoring */
      bool isFound;                            /**< flag for a hit that has been found near a track as expected by extrapolation */
    };
  }
}
