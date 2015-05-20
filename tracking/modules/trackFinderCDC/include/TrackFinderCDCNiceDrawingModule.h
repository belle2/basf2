/*
 * CDCNiceDrawingModule.h
 *
 *  Created on: May 31, 2013
 *      Author: kronenbitter
 */

#pragma once

#include <framework/core/Module.h>

#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/translators/SimpleTDCCountTranslator.h>

#include <fstream>
#include <genfit/Track.h>
#include <genfit/TrackCand.h>

#include <tracking/trackFindingCDC/legendre/CDCLegendreNiceDrawing.h>

class TVector2;

namespace Belle2 {

  class CDCHit;
  class MCParticle;

  class CDCNiceDrawingModule : public Module {

  public:

    CDCNiceDrawingModule();

    virtual ~CDCNiceDrawingModule();

    virtual void initialize();

    virtual void beginRun() {};

    virtual void event();

    virtual void endRun() {};

    virtual void terminate() {};


  private:

    std::string m_StoreDirectory;

    std::string m_TrackCandColName;

    std::string m_MCTrackCandColName;

    std::string m_HitColName;

    std::string m_mcParticlesColName;

    std::string m_trackColName;

    bool m_drawMCSignal;

    bool m_drawAlsoDifference;  /// Output three pictures for each event: one with the MCTracks and the Candidates, one with the Candidates with no MCTrack relates to and one with the MCTracks with no Candidate related to

    bool m_drawCands;

    TrackFindingCDC::NiceDrawing* m_cdcLegendreNiceDrawing;

  };


}
