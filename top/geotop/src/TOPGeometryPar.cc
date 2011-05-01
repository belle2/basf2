/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <top/geotop/TOPGeometryPar.h>

#include <cmath>
#include <boost/format.hpp>

using namespace std;
using namespace boost;
using namespace Belle2;

TOPGeometryPar* TOPGeometryPar::p_B4TOPGeometryParDB = 0;

TOPGeometryPar* TOPGeometryPar::Instance(void)
{
  if (!p_B4TOPGeometryParDB) p_B4TOPGeometryParDB = new TOPGeometryPar();
  return p_B4TOPGeometryParDB;
}

TOPGeometryPar::TOPGeometryPar()
{
  clear();
  read();
}

TOPGeometryPar::~TOPGeometryPar()
{
}

void
TOPGeometryPar::clear(void)
{
  _DetectorRotation = 0.0;
  _DetectorZShift = 0.0;
  _NumberOfBars = 0;
  _DetectorInnerRadius = 0.0;
  _QuartzWidth = 0.0;
  _QuartzExtWidth = 0.0;
  _QuartzThickness = 0.0;
  _QuartzBPos = 0.0;
  _QuartzLenSeg1 = 0.0;
  _QuartzLenSeg2 = 0.0;
  _QuartzLenMir = 0.0;
  _QuartzZExt = 0.0;
  _QuartzYExtUp = 0.0;
  _QuartzYExtDown = 0.0;
  _GlueThickness1 = 0.0;
  _GlueThickness2 = 0.0;
  _GlueThickness3 = 0.0;
  _MirrorCenterX = 0.0;
  _MirrorCenterY = 0.0;
  _MirrorR = 0.0;
  _GapPMTX = 0.0;
  _GapPMTY = 0.0;
  _GapPMTX0 = 0.0;
  _GapPMTY0 = 0.0;
  _PMTNX = 0;
  _PMTNY = 0;
  _PMTSizeX = 0.0;
  _PMTSizeY = 0.0;
  _PMTSizeZ = 0.0;
  _ActiveSizeX = 0.0;
  _ActiveSizeY = 0.0;
  _ActiveSizeZ = 0.0;
  _WindowThickness = 0.0;
  _PadNX = 0;
  _PadNY = 0;
  _BottomThickness = 0.0;
  _QBBThickness = 0.0;
  _QBBSideThickness = 0.0;
  _QBBFronThickness = 0.0;
  _AirGapUp = 0.0;
  _AirGapDown = 0.0;
  _AirGapSide = 0.0;
  _QBBForwardPos = 0.0;
  _QBBBackwardPos = 0.0;

}

void TOPGeometryPar::read()
{

  GearDir gbxParams = Gearbox::Instance().getContent("TOP");

  //------------------------------
  // Get TOP geometry parameters
  //------------------------------


  _DetectorRotation = gbxParams.getParamLength("Rotation");
  _DetectorZShift = gbxParams.getParamLength("OffsetZ");
  _NumberOfBars = (int)gbxParams.getParamNumValue("Bars/Nbar");
  _DetectorInnerRadius = gbxParams.getParamLength("Bars/Radius");
  _QuartzWidth = gbxParams.getParamLength("Bars/QWidth");
  _QuartzExtWidth = gbxParams.getParamLength("Bars/QWidthExt");
  _QuartzThickness = gbxParams.getParamLength("Bars/QThickness");
  _QuartzBPos = gbxParams.getParamLength("Bars/QZBackward");
  _QuartzLenSeg1 = gbxParams.getParamLength("Bars/QBar1Length");
  _QuartzLenSeg2 = gbxParams.getParamLength("Bars/QBar2Length");
  _QuartzLenMir = gbxParams.getParamLength("Bars/QBarMirror");
  _QuartzZExt = gbxParams.getParamLength("Bars/QZext");
  _QuartzYExtUp = gbxParams.getParamLength("Bars/QYextup");
  _QuartzYExtDown = gbxParams.getParamLength("Bars/QYextdown");
  _GlueThickness1 = gbxParams.getParamLength("Bars/Glue/Thicknes1");
  _GlueThickness2 = gbxParams.getParamLength("Bars/Glue/Thicknes2");
  _GlueThickness3 = gbxParams.getParamLength("Bars/Glue/Thicknes3");
  _MirrorCenterX = gbxParams.getParamLength("Bars/Mirror/Xpos");
  _MirrorCenterY = gbxParams.getParamLength("Bars/Mirror/Ypos");
  _MirrorR = gbxParams.getParamLength("Bars/Mirror/Radius");
  _GapPMTX = gbxParams.getParamLength("PMTs/Xgap");
  _GapPMTY = gbxParams.getParamLength("PMTs/Ygap");
  _GapPMTX0 = gbxParams.getParamLength("PMTs/x0");
  _GapPMTY0 = gbxParams.getParamLength("PMTs/y0");
  _PMTNX = (int)gbxParams.getParamNumValue("PMTs/nPMTx");
  _PMTNY = (int)gbxParams.getParamNumValue("PMTs/nPMTy");
  _PMTSizeX = gbxParams.getParamLength("PMTs/Module/ModuleXSize");
  _PMTSizeY = gbxParams.getParamLength("PMTs/Module/ModuleYSize");
  _PMTSizeZ = gbxParams.getParamLength("PMTs/Module/ModuleZSize");
  _ActiveSizeX = gbxParams.getParamLength("PMTs/Module/SensXSize");
  _ActiveSizeY = gbxParams.getParamLength("PMTs/Module/SensYSize");
  _ActiveSizeZ = gbxParams.getParamLength("PMTs/Module/SensThickness");
  _WindowThickness = gbxParams.getParamLength("PMTs/Module/WindowThickness");
  _PadNX = (int)gbxParams.getParamNumValue("PMTs/Module/PadXNum");
  _PadNY = (int)gbxParams.getParamNumValue("PMTs/Module/PadYNum");
  _BottomThickness = gbxParams.getParamLength("PMTs/Module/BottomThickness");
  _QBBThickness = gbxParams.getParamLength("Support/PannelThickness");
  _QBBSideThickness = gbxParams.getParamLength("Support/SidePannelThickness");
  _QBBFronThickness = gbxParams.getParamLength("Support/FrontPannelThickness");
  _AirGapUp = gbxParams.getParamLength("Support/lowerGap");
  _AirGapDown = gbxParams.getParamLength("Support/upperGap");
  _AirGapSide = gbxParams.getParamLength("Support/sideGap");
  _QBBForwardPos = gbxParams.getParamLength("Support/ZForward");
  _QBBBackwardPos = gbxParams.getParamLength("Support/ZBackward");
  /**/
}

void TOPGeometryPar::Print(void) const
{
}

