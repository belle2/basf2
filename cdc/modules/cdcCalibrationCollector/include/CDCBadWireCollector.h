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
#include <framework/datastore/StoreArray.h>
#include <Math/Vector3D.h>
#include <string>

namespace Belle2 {
  namespace CDC {

    /**
     * Collect hit information for cdc calibration with CAF
     */
    class CDCBadWireCollectorModule : public CalibrationCollectorModule {

    public:
      /**
       * Constructor.
       */
      CDCBadWireCollectorModule();

      /**
       * Destructor.
       */
      virtual ~CDCBadWireCollectorModule();

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
       * Build efficiency
       */
      void buildEfficiencies(std::vector<unsigned short> wireHits, const Helix helixFit);
      /**
       * extrapolates the helix fit to a given layer and finds the wire which it would be hitting
       */
      const TrackFindingCDC::CDCWire& getIntersectingWire(const TVector3& xyz, const TrackFindingCDC::CDCWireLayer& layer,
                                                          const Helix& helixFit) const;

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

      double m_minimumPt = 0;   /**< minimum pt required for track*/
      bool m_storeTrackParams = true;          /**< Store Track parameter or not. */
      bool m_bField = true;                    /**< fit incase no magnetic Field of not, if false, NDF=4 in cal P-value */
      double m_minimumNDF = 0;   /**< minimum NDF required for track*/
      bool m_effStudy = true;                 /**< When true module collects info only necessary for wire eff study. */
      Float_t ndf;               /**< degree of freedom. */
      Float_t Pval;              /**< P-value of fitted track.  */

      unsigned short wireID;                   /**< wireID for hit-level wire monitoring */
      unsigned short layerID;                  /**< layerID for hit-level wire monitoring */
      float z;                                 /**< z of hit fot hit-level wire monitoring */
      bool isFound;                            /**< flag for a hit that has been found near a track as expected by extrapolation */
    };
  }
}
