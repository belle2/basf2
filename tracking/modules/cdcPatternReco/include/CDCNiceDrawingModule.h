/*
 * CDCNiceDrawingModule.h
 *
 *  Created on: May 31, 2013
 *      Author: kronenbitter
 */

#pragma once

#include <framework/core/Module.h>

#include "cdc/geometry/CDCGeometryPar.h"
#include "cdc/translators/SimpleTDCCountTranslator.h"

#include <fstream>
#include "genfit/Track.h"
#include "genfit/TrackCand.h"

class TVector2;
//class TrackCand;
//class Track;

namespace Belle2 {

  class CDCHit;
  class MCParticle;
  class CDCLegendreNiceDrawing;

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

    std::string m_HitColName;

    std::string m_mcParticlesColName;

    std::string m_trackColName;

    bool m_drawMCSignal;

//    double m_zReference;

    bool m_drawCands;

    CDCLegendreNiceDrawing* m_cdcLegendreNiceDrawing;

  };


}
