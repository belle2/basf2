/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * Prepared for track quality check                                       *
 * Prepared for Phase 2 and Belle II geometry                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>

#include <framework/core/Module.h>
#include <framework/core/HistoModule.h>
#include <TH1F.h>
#include <TH2F.h>

namespace Belle2 {

  /** DQM of tracks
    * their momentum,
    * Number of hits in tracks,
    * Number of tracks.
    *
    */
  class TrackDQMModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Constructor */
    TrackDQMModule();
    /* Destructor */
    ~TrackDQMModule();

    /** Module functions */
    void initialize() override final;
    void beginRun() override final;
    void event() override final;
    void endRun() override final;
    void terminate() override final;

    /**
    * Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
    * to be placed in this function.
    */
    void defineHisto() override final;

  private:
    /** p Value */
    TH1F* m_PValue;
    /** Chi2 */
    TH1F* m_Chi2;
    /** NDF */
    TH1F* m_NDF;
    /** Chi2 / NDF */
    TH1F* m_Chi2NDF;
    /** Unbiased residuals for PXD u vs v */
    TH2F* m_UBResidualsPXD;
    /** Unbiased residuals for SVD u vs v */
    TH2F* m_UBResidualsSVD;
    /** Unbiased residuals for PXD and SVD u vs v per sensor*/
    TH2F** m_UBResidualsSensor;
    /** Track related clusters - hitmap in IP angle range */
    TH2F** m_TRClusterHitmap;
    /** Track related clusters - neighbor corelations in Phi */
    TH2F** m_TRClusterCorrelationsPhi;
    /** Track related clusters - neighbor corelations in Theta */
    TH2F** m_TRClusterCorrelationsTheta;


    /** Track momentum Pt.X */
    TH1F* m_MomX;
    /** Track momentum Pt.Y */
    TH1F* m_MomY;
    /** Track momentum Pt.Z */
    TH1F* m_MomZ;
    /** Track momentum Pt */
    TH1F* m_MomPt;
    /** Track momentum Magnitude */
    TH1F* m_Mom;
    /** Number of hits on PXD */
    TH1F* m_HitsPXD;
    /** Number of hits on VXD */
    TH1F* m_HitsSVD;
    /** Number of hits on CDC */
    TH1F* m_HitsCDC;
    /** Number of all hits in tracks */
    TH1F* m_Hits;
    /** Number of tracks only with VXD */
    TH1F* m_TracksVXD;
    /** Number of tracks only with CDC */
    TH1F* m_TracksCDC;
    /** Number of full tracks with VXD+CDC */
    TH1F* m_TracksVXDCDC;
    /** Number of all finding tracks */
    TH1F* m_Tracks;

    /** Number of VXD layers on Belle II */
    int c_nVXDLayers;
    /** Number of PXD layers on Belle II */
    int c_nPXDLayers;
    /** Number of SVD layers on Belle II */
    int c_nSVDLayers;
    /** First VXD layer on Belle II */
    int c_firstVXDLayer;
    /** Last VXD layer on Belle II */
    int c_lastVXDLayer;
    /** First PXD layer on Belle II */
    int c_firstPXDLayer;
    /** Last PXD layer on Belle II */
    int c_lastPXDLayer;
    /** First SVD layer on Belle II */
    int c_firstSVDLayer;
    /** Last SVD layer on Belle II */
    int c_lastSVDLayer;
    /** Maximum No of PXD ladders on layer */
    unsigned int c_MaxLaddersInPXDLayer;
    /** Maximum No of SVD ladders on layer */
    unsigned int c_MaxLaddersInSVDLayer;
    /** Maximum No of PXD sensors on layer */
    unsigned int c_MaxSensorsInPXDLayer;
    /** Maximum No of SVD sensors on layer */
    unsigned int c_MaxSensorsInSVDLayer;
    /** No of PXD sensors */
    int c_nPXDSensors;
    /** No of SVD sensors */
    int c_nSVDSensors;

    /** Function return index of layer in plots.
       * @param Layer Layer position.
       * @return Index of layer in plots.
       */
    int getLayerIndex(const int Layer) const;
    /** Function return index of layer in plots.
       * @param Index Index of layer in plots.
       * @param Layer return layer position.
       */
    void getLayerIDsFromLayerIndex(const int Index, int& Layer) const;
    /** Function return index of sensor in plots.
       * @param Layer Layer position of sensor.
       * @param Ladder Ladder position of sensor.
       * @param Sensor Sensor position of sensor.
       * @return Index of sensor in plots.
       */
    int getSensorIndex(const int Layer, const int Ladder, const int Sensor) const;
    /** Function return index of sensor in plots.
       * @param Index Index of sensor in plots.
       * @param Layer return Layer position of sensor.
       * @param Ladder return Ladder position of sensor.
       * @param Sensor return Sensor position of sensor.
       */
    void getIDsFromIndex(const int Index, int& Layer, int& Ladder, int& Sensor) const;

  };  //end class declaration

}  // end namespace Belle2

