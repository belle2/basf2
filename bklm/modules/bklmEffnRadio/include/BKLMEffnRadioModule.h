/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anselm Vossen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef BKLMEFFNRADIOMODLUE
#define  BKLMEFFNRADIOMODLUE

#include <framework/core/Module.h>
#include "TFile.h"
#include "TBox.h"
#include "TCanvas.h"
#include "TH2D.h"
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>
#include <bklm/dataobjects/BKLMHit2d.h>

#include "CLHEP/Vector/ThreeVector.h"
#include "CLHEP/Matrix/Matrix.h"

#include <bklm/geometry/Module.h>
#include <iostream>
#include <set>


namespace Belle2 {
  namespace bklm {
    class GeometryPar;
  }
  /**
   * produces QA plots for the RPC. E.g. Efficiency and occupancy plots. Uses linear fits to extrapolate straight tracks
   */
  class BKLMEffnRadioModule : public Module {
  public:
    //! Constructor
    BKLMEffnRadioModule();
    //! Destructor
    virtual  ~BKLMEffnRadioModule();
    //! Initialize at start of job
    virtual void initialize();
    //! begin run actions
    virtual void beginRun();
    //! Process one event (fill histograms etc)
    virtual void event();
    //! end run cleanunp
    virtual void endRun();
    //! Terminate at the end of job
    virtual void terminate();

  protected:
    //! struct to save the track parameters
    struct TrackParams {
      //! slope of the track along z
      float bz;
//! slope of the track along y
      float by;
      //! z offset
      float mz;
      //! y offset
      float my;
    };

    //! simple point class for the tracking
    struct SimplePoint {
      //! x coordinate
      float x;
      //! y coordinate
      float y;
      //! z coordinate
      float z;
    };

    //! number of points needed for a valid track
    int m_minNumPointsOnTrack;
    //! max distance in cm for which a hit is deemed to belong to the track for the efficiency measurements
    int m_maxEffDistance;
    //! event counter
    long int m_eventCounter;
    //! iterate over 2D hits get possible track candidates, check if they are valid
    //! and if the given plane is efficient
    void getEffs();
    //! fit track to the given points
    //! @param[in] points the points used
    //! @param[out] params the fitted track parameters
    void trackFit(std::vector<SimplePoint*>& points, TrackParams& params);
    //! is this a valid track candidate?
    //! @param[in] firstHit the first seed
    //! @param[in] secondHit the second seed hit
    //! @param[in] hits2D the 2D hits of this event
    //! @param[out] points the points that belong to the track found
    //! @param[in] refMod The reference module that defines the coordinate system used for the track
    //! @param[in] effLayer The layer of which we want to determine the efficiency, which is excluded from the tracking
    //! @return true if this is a valid candidate
    bool validTrackCandidate(int firstHit, int secondHit,  StoreArray<BKLMHit2d>& hits2D, std::vector<SimplePoint*>& points,
                             const Belle2::bklm::Module* refMod, int effLayer);
    //! seet the plot style
    void set_plot_style();
    //! canvas to draw the module occupancy
    TCanvas*** m_cModule;
    //! canvas to draw module efficiencies
    TCanvas*** m_cModuleEff;
    //! canvas to draw 2D module efficiencies
    TCanvas*** m_cModuleEff2D;
    //! histogram for track phi
    TH1D* m_hTrackPhi;
    //! histogram for track theta
    TH1D* m_hTrackTheta;
    //! histogram given
    TH1D* m_hHitsPerLayer;
    //! histogram for cluster sizes
    TH1D* m_hClusterSize;
    //! histogram for number of tracks per event
    TH1D* m_hTracksPerEvent;
    //! histogram for hits per event 1D
    TH1D* m_hHitsPerEvent1D;
    //! histogram for hits per even (2D)
    TH1D* m_hHitsPerEvent2D;
    //! histogram for hits per event per layer
    TH2D* m_hHitsPerEventPerLayer1D;
    //! 1D occupancy frequency histograms
    TH1D***** m_hOccupancy1D;



    //! histograms for found hits
    TH2D**** m_eff2DFound;
    //! histograms for the expected hits
    TH2D**** m_eff2DExpected;
    //! tbox to draw the strip occupancies
    TBox****** m_strips;
    //! tbox to draw the strip efficiencies
    TBox****** m_stripsEff;
    //! TFile to save everything
    TFile* m_file;
    //! filename for the root file
    std::string m_filename;

    //! number of hits for each strip. Don't think that moving average is necessary here...
    int***** m_stripHits;
    //! strip is hit
    int***** m_stripHitsEff;
    //! strip is not hit
    int***** m_stripNonHitsEff;

  private:
    //! geometry object
    bklm::GeometryPar* m_GeoPar;
    //! to keep track which 2D hits where already used for tracks...
    std::set<int> m_pointIndices;
    //! min seed layer distance
    int minSeedLayerDistance = 3;
    //! 2d hits
    StoreArray<BKLMHit2d> hits2D;
    //! 1d hits
    StoreArray<BKLMHit1d> hits1D;

  };
}


#endif
