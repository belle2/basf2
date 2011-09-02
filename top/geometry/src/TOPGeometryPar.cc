/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj, Rok Pestotnik                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <top/geometry/TOPGeometryPar.h>

#include <cmath>
#include <boost/format.hpp>
#include <boost/foreach.hpp>

using namespace std;
using namespace boost;
using namespace Belle2;

TOPGeometryPar* TOPGeometryPar::p_B4TOPGeometryParDB = 0;

TOPGeometryPar* TOPGeometryPar::Instance(const GearDir& content)
{
  if (!p_B4TOPGeometryParDB) {
    p_B4TOPGeometryParDB = new TOPGeometryPar();
    p_B4TOPGeometryParDB->Initialize(content);
  }
  return p_B4TOPGeometryParDB;
}


TOPGeometryPar::TOPGeometryPar()
{
  clear();
}

TOPGeometryPar::~TOPGeometryPar()
{
}

void TOPGeometryPar::Initialize(const GearDir& content)
{
  read(content);
}

void
TOPGeometryPar::clear(void)
{

  //! Parameters for Bars

  _Nbars = 0;
  _Radius = 0;
  _Qwidth = 0;
  _Qthickness = 0;
  _Bposition = 0;
  _Length1 = 0;
  _Length2 = 0;
  _Length3 = 0;
  _WLength = 0;
  _Wwidth = 0;
  _Wextup = 0;
  _Wextdown = 0;
  _Gwidth1 = 0;
  _Gwidth2 = 0;
  _Gwidth3 = 0;


  //! Parameters for PMT

  _Xgap = 0;
  _Ygap = 0;
  _Npmtx = 0;
  _Npmty = 0;
  _Msizex = 0;
  _Msizey = 0;
  _MWsizez = 0;
  _Asizex = 0;
  _Asizey = 0;
  _Asizez = 0;
  _Winthickness = 0;
  _Botthickness = 0;
  _Npadx = 0;
  _Npady = 0;
  _dGlue = 0;

  //! Mirror parameters

  _Mirposx = 0;
  _Mirposy = 0;
  _Mirthickness = 0;
  _Mirradius = 0;
}

void TOPGeometryPar::read(const GearDir& content)
{

  //------------------------------
  // Get TOP geometry parameters
  //------------------------------
  GearDir barParams(content, "Bars");

  //! Parameters for Bars

  _Nbars = barParams.getInt("Nbar");
  _Radius = barParams.getLength("Radius");
  _Qwidth = barParams.getLength("QWidth");
  _Qthickness = barParams.getLength("QThickness");
  _Bposition = barParams.getLength("QZBackward");
  _Length1 = barParams.getLength("QBar1Length");
  _Length2 = barParams.getLength("QBar2Length");
  _Length3 = barParams.getLength("QBarMirror");
  _WLength = barParams.getLength("QWedgeLength");
  _Wwidth = barParams.getLength("QWedgeWidth");
  _Wextup = barParams.getLength("QWedgetup");
  _Wextdown = barParams.getLength("QWedgetdown");
  _Gwidth1 = barParams.getLength("Glue/Thicknes1");
  _Gwidth2 = barParams.getLength("Glue/Thicknes2");
  _Gwidth3 = barParams.getLength("Glue/Thicknes3");


  //! Parameters for PMT

  GearDir detParams(content, "Detector");

  _Xgap = detParams.getLength("Xgap");
  _Ygap = detParams.getLength("Ygap");
  _Npmtx = detParams.getInt("nPMTx");
  _Npmty = detParams.getInt("nPMTy");
  _Msizex = detParams.getLength("Module/ModuleXSize");
  _Msizey = detParams.getLength("Module/ModuleYSize");
  _MWsizez = detParams.getLength("Module/ModuleZSize");
  _Asizex = detParams.getLength("Module/SensXSize");
  _Asizey = detParams.getLength("Module/SensYSize");
  _Asizez = detParams.getLength("Module/SensThickness");
  _Winthickness = detParams.getLength("Module/WindowThickness");
  _Botthickness = detParams.getLength("Module/BottomThickness");
  _Npadx = detParams.getInt("Module/PadXNum");
  _Npady = detParams.getInt("Module/PadXNum");
  _dGlue = detParams.getLength("dGlue");

  //! Mirror parameters

  GearDir mirParams(content, "Mirror");

  _Mirposx = mirParams.getLength("Xpos");
  _Mirposy = mirParams.getLength("Ypos");
  _Mirthickness = mirParams.getLength("mirrorThickness");
  _Mirradius = mirParams.getLength("Radius");


}

void TOPGeometryPar::Print(void) const
{
}

