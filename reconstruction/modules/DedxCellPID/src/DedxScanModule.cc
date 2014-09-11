/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/modules/DedxCellPID/DedxScanModule.h>
#include <reconstruction/modules/DedxCellPID/LineHelper.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/Const.h>
#include <framework/utilities/FileSystem.h>

#include <reconstruction/dataobjects/DedxCell.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <vxd/geometry/GeoCache.h>
#include <geometry/GeometryManager.h>
#include <tracking/gfbfield/GFGeant4Field.h>

#include <genfit/TrackCand.h>
#include <genfit/Track.h>
#include <genfit/AbsTrackRep.h>
#include <genfit/Exception.h>
#include <genfit/FieldManager.h>
#include <genfit/MaterialEffects.h>
#include <genfit/TGeoMaterialInterface.h>
#include <genfit/StateOnPlane.h>

#include <TGeoManager.h>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include <cassert>
#include <cmath>
#include <algorithm>
#include <utility>
#include <stdlib.h>
#include <time.h>

#include "TMath.h"
#include "TFile.h"
#include "TH2F.h"

using namespace Belle2;
using namespace CDC;
using namespace Dedx;

REG_MODULE(DedxScan)

DedxScanModule::DedxScanModule() : Module()
{

  setDescription("Extract dE/dx and corresponding log-likelihood from fitted tracks and hits in the CDC, SVD and PXD.");
}

DedxScanModule::~DedxScanModule() { }

void DedxScanModule::initialize()
{

  // register outputs
  StoreArray<DedxCell>::registerPersistent();

  // create instances here to not confuse profiling
  CDCGeometryPar::Instance();
  VXD::GeoCache::getInstance();

  // if the TGeo geometry is not initialized, do it ourselves
  if (!gGeoManager) {
    // convert geant4 geometry to TGeo geometry
    geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
    geoManager.createTGeoRepresentation();

    // initialize some things for genfit
    genfit::FieldManager::getInstance()->init(new GFGeant4Field());
    genfit::MaterialEffects::getInstance()->init(new genfit::TGeoMaterialInterface());
  }
}

void DedxScanModule::event()
{

  // outputs
  StoreArray<DedxCell> dedxArray;

  // get the geometry of the cdc
  static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();

  // **************************************************
  //
  //  GENERATE PARAMETERS OVER THE FULL RANGE
  //
  // **************************************************

  srand(time(NULL));

  boost::shared_ptr<DedxCell> dedxCell = boost::make_shared<DedxCell>();

  for (int i = 0; i < 56; ++i) {
    int nWires = cdcgeo.nWiresInLayer(i);

    // position of a sense wire in this layer at endpoints
    const TVector3& wirePosF = cdcgeo.wireForwardPosition(i, 0);
    const TVector3& wirePosB = cdcgeo.wireBackwardPosition(i, 0);
    const TVector3 wireDir = (wirePosB - wirePosF).Unit();

    // radii of field wires for this layer
    double inner = cdcgeo.innerRadiusWireLayer()[i];
    double outer = cdcgeo.outerRadiusWireLayer()[i];

    double cellHeight = outer - inner;
    double topHeight = outer - wirePosF.Perp();
    double bottomHeight = wirePosF.Perp() - inner;
    double topHalfWidth = PI * outer / nWires;
    double bottomHalfWidth = PI * inner / nWires;
    double cellHalfWidth = PI * wirePosF.Perp() / nWires;

    // first construct the boundary lines, then create the cell
    const Point tl = Point(-topHalfWidth, topHeight);
    const Point tr = Point(topHalfWidth, topHeight);
    const Point br = Point(bottomHalfWidth, -bottomHeight);
    const Point bl = Point(-bottomHalfWidth, -bottomHeight);
    Cell c = Cell(tl, tr, br, bl);

    for (int j = 0; j < 100; ++j) {
      for (int k = 0; k < 100; ++k) {
        double doca = j * cellHalfWidth / 50.0 - cellHalfWidth;
        double entAng = k * 3.14159265 / 100.0 - 3.14159265 / 2.0;

        // now calculate the path length for this hit
        double celldx = c.dx(doca, entAng);
        if (!c.isValid()) continue;

        dedxCell->addHit(i, 0, doca, entAng, 0.0, celldx, 0.0, cellHeight, cellHalfWidth);
      }
    }
    dedxArray.appendNew(*dedxCell);
  }
}

void DedxScanModule::terminate()
{

  B2INFO("DedxScanModule exiting after processing " << m_trackID <<
         " tracks in " << m_eventID + 1 << " events.");
}
