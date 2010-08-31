/**************************************************************************
*  BASF2 (Belle Analysis Framework 2)                                    *
*  Copyright(C) 2010 - Belle II Collaboration                            *
*                                                                        *
*  Author: The Belle II Collaboration                                    *
*  Contributors: Andreas Moll, Zbynek Drasal                             *
*  Modified to include surrounding IR geometry by Clement Ng
*                                                                        *
*  This software is provided "as is" without any warranty.               *
* *************************************************************************/

//** todo:
//  - allow default parameter values
//  - assign volume hierarchy?
//  - add color xml parameter?


#include <ir/geoir/GeoIRBelleIISymm.h>

#include <geometry/gearbox/GearDir.h>
#include <framework/datastore/Units.h>

#include <cmath>
#include <boost/format.hpp>

#include <TMath.h>
#include <TVector3.h>
#include <TGeoMatrix.h>
#include <TGeoManager.h>
#include <TGeoVolume.h>
#include <TGeoBBox.h>
#include <TGeoPcon.h>
#include "TGeoCompositeShape.h"

#include <iostream>

using namespace std;
using namespace boost;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Creator
//-----------------------------------------------------------------

GeoIRBelleIISymm regGeoIRBelleIISymm;

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

GeoIRBelleIISymm::GeoIRBelleIISymm() : CreatorBase("IRBelleIISymm")
{
  setDescription("Creates the TGeo objects for the outer IR geometry of the Belle II detector.");
}


GeoIRBelleIISymm::~GeoIRBelleIISymm()
{

}

#include "TGeoTube.h"
#include "TGeoCone.h"
// -----------------------------------------------------------
// --- TGeoShape function for BelleII Pipe segment ---
// --
// - Will create a pipe shape, optionally cut along x-axis with either:
// - - different angle ends
// - - different radius ends
// - but NOT both

TGeoShape* GeoIRBelleIISymm::GeoBelleIICutPipe(const char* name_,
                                               const double length_,           // length of pipe
                                               const double radinn1_,          // inner radius at start
                                               const double radout1_,          // outer radius at start
                                               const double radchange_,        // change in radius
                                               const double theta1_,     // angle to previous pipe
                                               const double theta2_,           // angle to next pipe
                                               const double cutheight_,        // x-axis cut distance from centre
                                               const int cutdirection_,        // x-axis cut direction (to do: 0 ignores cut)
                                               const double rotation_)         // rotation around y-axis
{
  double cost1 = cos(0.5 * theta1_ * deg);
  double sint1 = sin(0.5 * theta1_ * deg);
  double cost2 = cos(0.5 * theta2_ * deg);
  double sint2 = sin(0.5 * theta2_ * deg);

  double radinn2 = radinn1_ + radchange_;
  double radout2 = radout1_ + radchange_;

  string tubeName = string(name_) + string("Tube");
  TGeoShape* tube;
  if (radinn2 != radinn1_) {
    // --- Form conical pipe
    if (theta1_ != theta2_) {
      cout << "Warning: different end-angled cone pipes not supported: ignoring end-angles." << endl;
    }
    tube = new TGeoConeSeg(tubeName.c_str(), length_,
                           radinn1_, radout1_,
                           radinn2, radout2,
                           0, 360);
  } else {
    // --- Form slanted pipe
    tube = new TGeoCtub(tubeName.c_str(), radinn1_, radout1_, length_,
                        0, 360,
                        0, -sint1, -cost1,          // coordinates later rotated around z-axis
                        0, -sint2, cost2);
  }

  //  --- Rotate and cut pipe along y-z plane
  string pipeRotName         = string("pipeRot") + string(name_);
  TGeoRotation* pipeRot      = new TGeoRotation(pipeRotName.c_str(), 90., rotation_, 0.);
  pipeRot->RegisterYourself();

  string clipName       = string("clip") + string(name_);
  TGeoBBox* clip        = new TGeoBBox(clipName.c_str(), radout1_, radout1_, length_ + radout1_);  // using halfSpace function may be faster? does not visualise properly
  string clipTransName       = "clipTrans" + string(name_);
  TGeoTranslation* clipTrans = new TGeoTranslation(clipTransName.c_str(), (cutdirection_*radout1_) + cutheight_, 0, 0);
  clipTrans->RegisterYourself();

  TGeoShape* pipe;
  if (cutdirection_) {
    string arg = tubeName + ":" + pipeRotName + "-" + clipName + ":" + clipTransName;
    pipe = new TGeoCompositeShape(name_, arg.c_str());
  } else {
    // Create pipe with no cut
    string arg = tubeName + ":" + pipeRotName;
    //pipe = new TGeoCompositeShape(name_, arg.c_str());        // ** currently not supported: need way to create a rotated shape without compositeshape
    pipe = tube;                                          // ** defer rotation
  }
  return pipe;
}

// -----------------------------------------------------------
// ---  Pipe stream creator ---
// --
// Fills containers with specified range of pipe segments from GearDir content
// ** Better to use shared_ptr?
void GeoIRBelleIISymm::createPipe(const string& name_,
                                  vector<TGeoShape*>& shape_,
                                  vector<TGeoMedium*>& medium_,
                                  vector<TGeoCombiTrans*>& trans_,
                                  GearDir& content_,
                                  const double start_,
                                  const double end_,
                                  const bool solid_)  // if true create solid pipes
{
  // --- Collect global parameters
  double length     = content_.getParamLength("Length");
  double offsetZ    = content_.getParamLength("OffsetZ");

  double IPBeampipeRad  = content_.getParamLength("Radius");
  double thickness      = content_.getParamLength("Thickness");
  string matName        = content_.getParamString("Material");
  TGeoMedium* medium    = gGeoManager->GetMedium(matName.c_str());

  // --- Loop content hierachy - Stream (HER/LER,Up/Downstream) -> Section (Inner/Mid/Outer etc) -> Pipe number
  int nStream = content_.getNumberNodes("Streams/Stream");

  for (int iStream = 1; iStream <= nStream; ++iStream) {
    GearDir streamContent(content_);
    streamContent.append((format("Streams/Stream[%1%]/") % (iStream)).str());

    // --- Collect stream information
    string streamName    = streamContent.getParamString("attribute::name");
    int streamDirZ = int (streamContent.getParamNumValue("Direction"));   // ** is there an Int param class?
    int streamDirX = 0;
    TGeoRotation* streamRot = new TGeoRotation();
    if (streamDirZ < 0)
      //streamRot->ReflectZ(kTRUE);
      streamRot->SetAngles(0, 180, 0);

    // initialise previous pipe parameter variables
    TVector3 pipePrevEnd(0., 0., 0.5*streamDirZ*length + offsetZ);
    double   pipePrevRadEnd(IPBeampipeRad);
    double   pipePrevRotY(0);

    int nSection = streamContent.getNumberNodes("Section");

    for (int iSection = 1; iSection <= nSection; ++iSection) {
      //if ( (start_ && streamDirZ == -1 && pipePrevEnd.z() <= start_) || (end_ && streamDirZ == 1 && pipePrevEnd.z() >= end_) )
      //    break;
      GearDir sectionContent(streamContent);
      sectionContent.append((format("Section[%1%]/") % (iSection)).str());

      // --- Collect section information
      string sectionName  = sectionContent.getParamString("attribute::name");
      thickness     = sectionContent.getParamLength("Thickness");   // **to do: use previous if undefined
      matName       = sectionContent.getParamString("Material");    // **to do: use previous if undefined
      medium        = gGeoManager->GetMedium(matName.c_str());

      int nPipe = sectionContent.getNumberNodes("Pipe");

      for (int iPipe = 1; iPipe <= nPipe; ++iPipe) {
        GearDir pipeContent(sectionContent);
        pipeContent.append((format("Pipe[%1%]/") % (iPipe)).str());

        // --- Collect pipe information
        string pipeName         = (format("%1%%2%%3%%4%") % name_ % streamName % sectionName % iPipe).str();
        double pipeLength   = pipeContent.getParamLength("Length");   // full length
        double pipeAngle      = pipeContent.getParamAngle("Angle") / deg; // **to do: use previous if undefined
        double pipeRadEnd   = pipeContent.getParamLength("RadiusEnd");  // **to do: use previous if undefined

        if (streamDirZ == -1 && ((start_ && pipePrevEnd.z() <= start_) || (end_   &&  pipePrevEnd.z() - pipeLength >= end_)))
          break;
        if (streamDirZ == 1  && ((end_   && pipePrevEnd.z() >= end_) || (start_ &&  pipePrevEnd.z() + pipeLength <= start_)))
          break;

        if (iSection == 1 && iPipe == 1) {
          // set direction of stream from first pipe angle
          if (pipeAngle > 0)
            streamDirX = 1;
          else if (pipeAngle < 0)
            streamDirX = -1;

          // --- Create extention to IP Beampipe to meet with crotch pipe
          double radinn    = solid_ ? 0 : IPBeampipeRad;
          double radout    = IPBeampipeRad + thickness;
          double extLength = radout * tan(0.5 * fabs(pipeAngle) * deg);
          double posZ      = pipePrevEnd.z() + 0.5 * streamDirZ * extLength;
          double cut       = 0.;
          int cutDir       = -streamDirX;
          double rotY      = 0.;
          string name      = pipeName + string("Ext");
          TGeoShape* extShape      = GeoBelleIICutPipe(name.c_str(), 0.5 * extLength, radinn, radout, 0, 0, pipeAngle, cut, cutDir, rotY);
          TGeoCombiTrans* extTrans = new TGeoCombiTrans(name.c_str(), 0.0, 0.0, posZ, streamRot);
          extTrans->RegisterYourself();
          //TGeoVolume* extVol  = new TGeoVolume(name.c_str(), extShape, medium);
          //extVol->SetLineColor(kCyan-3);
          //volGrpBP->AddNode( extVol, 1, new TGeoCombiTrans(0.0, 0.0, posZ, streamRot) );
          shape_.push_back(extShape);
          medium_.push_back(medium);
          trans_.push_back(extTrans);
          pipePrevEnd.SetZ(posZ + 0.5*streamDirZ*extLength);
        }

        // collect information about next pipe
        GearDir pipeNextContent(streamContent);
        double pipeNextAngle(0);
        // **to do: use previous angle if undefined
        if (iPipe < nPipe) {
          pipeNextContent.append((format("Section[%1%]/Pipe[%2%]/") % (iSection) % (iPipe + 1)).str());
          pipeNextAngle = pipeNextContent.getParamAngle("Angle") / deg;
        } else if (iSection < nSection) {
          pipeNextContent.append((format("Section[%1%]/Pipe[1]/") % (iSection + 1)).str());
          pipeNextAngle = pipeNextContent.getParamAngle("Angle") / deg;
        } else {
          pipeNextAngle = pipeAngle;
        }

        // calculate position
        double pipeRotY  = pipePrevRotY + pipeAngle;
        double sint      = sin(pipeRotY * deg);
        double cost      = cos(pipeRotY * deg);
        TVector3 pipeEnd = 0.5 * TVector3(pipeLength * sint, 0., streamDirZ * pipeLength * cost);
        TVector3 pipePos = pipePrevEnd + pipeEnd;

        // detect need for cut, based on simple pipe position assumptions
        double pipeRadout = pipePrevRadEnd + thickness;
        double height    = fabs(pipePrevEnd.x()) - cost * pipeRadout;
        double pipeCut   = 0;
        int pipeCutDir;
        if (height < 0) {
          pipeCut        = -pipePos.x();
          pipeCutDir     = -streamDirX;
        } else {
          pipeCutDir     = 0;
        }

        // --- Add pipe
        double pipeRadinn    = solid_ ? 0 : pipePrevRadEnd;
        double pipeRadchange = pipeRadEnd - pipePrevRadEnd;

        TGeoShape* pipeShape = GeoBelleIICutPipe(pipeName.c_str(),
                                                 0.5 * pipeLength,
                                                 pipeRadinn,
                                                 pipeRadout,
                                                 pipeRadchange,
                                                 pipeAngle,
                                                 pipeNextAngle,
                                                 pipeCut,
                                                 pipeCutDir,
                                                 pipeRotY);

        //TGeoVolume* pipeVol       = new TGeoVolume(pipeName.c_str(), pipeShape, medium);

        TGeoRotation* volRot;
        if (pipeCutDir) {
          // rotation already performed in GeoBelleIICutPipe for cut pipes
          volRot = streamRot;
        } else {
          // perform rotation here
          string volRotName       = string("volRot") + pipeName;
          volRot            = new TGeoRotation(volRotName.c_str(), 90., pipeRotY, 0.);
          volRot->MultiplyBy(streamRot, kFALSE);
        }
        TGeoCombiTrans* pipeTrans = new TGeoCombiTrans(pipeName.c_str(), pipePos.x(), 0.0, pipePos.z(), volRot);
        pipeTrans->RegisterYourself();

        //volGrpBP->AddNode( pipeVol, 1, pipeTrans );
        shape_.push_back(pipeShape);
        medium_.push_back(medium);
        trans_.push_back(pipeTrans);

        // store pipe information for next segment
        pipePrevRotY   = pipeRotY;
        pipePrevEnd    = pipePos + pipeEnd;
        pipePrevRadEnd = pipeRadEnd;
      }
    }
  }
}


void GeoIRBelleIISymm::create(GearDir& content)
{
  //Collect global parameters
  double rotAngle   = content.getParamAngle("Rotation") / deg;

  double zMin   = content.getParamLength("RangeStart");
  double zMax     = content.getParamLength("RangeEnd");

  TGeoVolume* topVolume = gGeoManager->GetTopVolume();
  TGeoVolumeAssembly* volGrpBP = new TGeoVolumeAssembly("OuterIR");

  TGeoRotation* geoRot = new TGeoRotation("BeamPipeRot", 90.0, rotAngle, 0.0);
  // volGrpBP is aligned and centred on BelleII IP
  topVolume->AddNode(volGrpBP, 1, new TGeoCombiTrans(0.0, 0.0, 0.0, geoRot));   // note: modified to apply translations inside nodes

  // -------------------------------------------------
  // ---  Build BeamPipe from shells ---
  // --

  double length     = content.getParamLength("Length");   // full length
  double offsetZ    = content.getParamLength("OffsetZ");

  int nShell = content.getNumberNodes("Shells/Shell");

  for (int iShell = 1; iShell <= nShell; ++iShell) {
    GearDir shellContent(content);
    shellContent.append((format("Shells/Shell[%1%]/") % (iShell)).str());

    //Collect information
    string shellName   = shellContent.getParamString("attribute::name");
    double innerRadius = shellContent.getParamLength("InnerRadius");
    double outerRadius = shellContent.getParamLength("OuterRadius");
    string shellMatName     = shellContent.getParamString("Material");

    //Get Material
    TGeoMedium* shellMed = gGeoManager->GetMedium(shellMatName.c_str());

    //Create Tube
    TGeoVolume* shellTube = gGeoManager->MakeTube(shellName.c_str(), shellMed, innerRadius, outerRadius, 0.5 * length);
    volGrpBP->AddNode(shellTube, 1, new TGeoTranslation(0.0, 0.0, offsetZ));    //translation moved to here
  }


  // -------------------------------------------------
  // ---  Build IR chamber streams ---
  // --

  vector<TGeoShape*>IRPipe;
  vector<TGeoMedium*>IRMed;
  vector<TGeoCombiTrans*>IRTrans;
  createPipe(string(), IRPipe, IRMed, IRTrans, content, zMin, zMax, 0);
  for (int i = 0; i < (int)IRPipe.size(); i++) {
    TGeoVolume* IRPipeVol = new TGeoVolume(IRPipe[i]->GetName(), IRPipe[i], IRMed[i]);
    IRPipeVol->SetLineColor(kTeal + 3);
    volGrpBP->AddNode(IRPipeVol, 1, IRTrans[i]);
  }

  // -------------------------------------------------
  // ---  Build shields ---
  // --
  int nShield = content.getNumberNodes("Shields/Shield");

  for (int iShield = 1; iShield <= nShield; ++iShield) {
    GearDir shieldContent(content);
    shieldContent.append((format("Shields/Shield[%1%]/") % (iShield)).str());

    // --- Collect shield information
    string shieldName   = shieldContent.getParamString("attribute::name");
    double shieldStart    = shieldContent.getParamLength("Pos");
    double shieldEnd;
    string shieldMatName    = shieldContent.getParamString("Material");
    TGeoMedium* shieldMed   = gGeoManager->GetMedium(shieldMatName.c_str());

    int nSeg = shieldContent.getNumberNodes("Seg");

    // --- Build shield bulk
    string shieldBulkName = shieldName.c_str() + string("Bulk");
    TGeoPcon* shieldBulk = new TGeoPcon(shieldBulkName.c_str(), 0, 360, nSeg);
    for (int iSeg = 1; iSeg <= nSeg; ++iSeg) {
      GearDir segContent(shieldContent);
      segContent.append((format("Seg[%1%]/") % (iSeg)).str());
      shieldBulk->DefineSection(iSeg - 1, segContent.getParamLength("z"), 0, segContent.getParamLength("rad"));
      if (iSeg == nSeg)
        shieldEnd = shieldStart + segContent.getParamLength("z");
    }

    TGeoTranslation* shieldTrans = new TGeoTranslation(shieldName.c_str(), 0, 0, shieldStart);
    shieldTrans->RegisterYourself();

    // --- Build and subtract pipes
    vector<TGeoShape*>shieldPipe;
    vector<TGeoMedium*>shieldPipeMed;
    vector<TGeoCombiTrans*>shieldPipeTrans;
    if (shieldStart > shieldEnd)
      swap(shieldStart, shieldEnd);
    createPipe(string("ShieldPipe"), shieldPipe, shieldPipeMed, shieldPipeTrans, content, shieldStart, shieldEnd, 1);
    string arg = shieldBulkName + ":" + shieldTrans->GetName();
    for (int i = 0; i < (int)shieldPipe.size(); i++)
      arg += "-" + string(shieldPipe[i]->GetName()) + ":" + string(shieldPipeTrans[i]->GetName());
    TGeoCompositeShape* shield = new TGeoCompositeShape(shieldName.c_str(), arg.c_str());
    TGeoVolume* shieldVol = new TGeoVolume(shieldName.c_str(), shield, shieldMed);
    shieldVol->SetLineColor(kCyan + 4);
    volGrpBP->AddNode(shieldVol, 1);
  }
}
