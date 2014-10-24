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
#include <bklm/dataobjects/BKLMHit2d.h>

#include "CLHEP/Vector/ThreeVector.h"
#include "CLHEP/Matrix/Matrix.h"

#include <bklm/geometry/Module.h>

using namespace std;
using namespace CLHEP;

namespace Belle2 {
  namespace bklm {
    class GeometryPar;
  }

  class BKLMEffnRadioModule : public Module {
  public:

    BKLMEffnRadioModule();

    virtual  ~BKLMEffnRadioModule();
    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

  protected:
    //no x because layers are along x
    struct TrackParams {
      float bz;
      float by;
      float mz;
      float my;
    };

    struct SimplePoint {
      float x;
      float y;
      float z;
    };

    //to make this a valid track..
    int m_minNumPointsOnTrack;
    int m_maxEffDistance;
    long int m_eventCounter;
    //iterate over 2D hits get possible track candidates, check if they are valid
    // and if the given plane is efficient
    void getEffs();
    void getTrackCandidate();
    void trackFit(vector<SimplePoint*>& points, TrackParams& params);
    void planeEfficient();
    bool validTrackCandidate(int firstHit, int secondHit,  StoreArray<BKLMHit2d>& hits2D, vector<SimplePoint*>& points, const Belle2::bklm::Module* refMod, int effLayer);

    void set_plot_style();
    TCanvas*** m_cModule;
    TCanvas*** m_cModuleEff;
    TCanvas*** m_cModuleEff2D;

    TH1D* m_hTrackPhi;
    TH1D* m_hTrackTheta;

    TH1D* m_hHitsPerLayer;
    TH1D* m_hClusterSize;

    TH1D* m_hTracksPerEvent;
    TH1D* m_hHitsPerEvent1D;
    TH1D* m_hHitsPerEvent2D;
    TH2D* m_hHitsPerEventPerLayer1D;



    TH2D**** m_eff2DFound;
    TH2D**** m_eff2DExpected;

    TBox****** m_strips;
    TBox****** m_stripsEff;

    TFile* m_file;
    string m_filename;

    //number of hits for each strip. Don't think that moving average is necessary here...
    int***** m_stripHits;
    int***** m_stripHitsEff;
    int***** m_stripNonHitsEff;

  private:
    bklm::GeometryPar* m_GeoPar;
    //to keep track which 2D hits where already used for tracks...
    set<int> m_pointIndices;
  };
}


#endif
