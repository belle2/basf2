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
//  - allow default parameter values for sections (thickness, material, crotch) and pipe (angle, radiusend)
//  - assign volume hierarchy?

#include <ir/geoir/GeoIRBelleII.h>

#include <framework/gearbox/GearDir.h>
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

GeoIRBelleII regGeoIRBelleII;

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

GeoIRBelleII::GeoIRBelleII() : CreatorBase("IRBelleII")
{
  setDescription("Creates the TGeo objects for the IR geometry of the Belle II detector.");
}


GeoIRBelleII::~GeoIRBelleII()
{

}

#include "TGeoTube.h"
#include "TGeoCone.h"
// -----------------------------------------------------------
// --- TGeoShape function for BelleII Pipe segment ---
// --
// - Will create a pipe shape with either (but NOT both):
// - - different angle ends
// - - different radius ends

TGeoShape* GeoIRBelleII::GeoBelleIIPipe(const char* name_,
                                        const double length_,           // length of pipe
                                        const double radinn1_,          // inner radius at start
                                        const double radout1_,          // outer radius at start
                                        const double radchange_,        // change in radius
                                        const double theta1_,     // angle to previous pipe
                                        const double theta2_)           // angle to next pipe
{
  double cost1 = cos(0.5 * theta1_ * deg);
  double sint1 = sin(0.5 * theta1_ * deg);
  double cost2 = cos(0.5 * theta2_ * deg);
  double sint2 = sin(0.5 * theta2_ * deg);

  double radinn2 = radinn1_ + radchange_;
  double radout2 = radout1_ + radchange_;

  string tubeName = string(name_) + "Tube";
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

  return tube;
}

// -----------------------------------------------------------
// ---  Outer pipes creator ---
// --
// Fills containers with specified range of pipes from GearDir content
// Manages crotch pipe shape creation
// Returns array of end overlap lengths with IP chamber due to the crotch joints
// ** Better to use shared_ptr?
double* GeoIRBelleII::createPipe(const char* name_,
                                 vector<TGeoShape*>& shape_,
                                 vector<TGeoMedium*>& medium_,
                                 vector<TGeoCombiTrans*>& trans_,
                                 GearDir& content_,
                                 double start_,
                                 double end_,
                                 const bool solid_)    // if true create solid pipes
{
  // --- Collect global parameters
  double length       = content_.getParamLength("Length");
  double offsetZ      = content_.getParamLength("OffsetZ");

  double IPBeampipeRad    = content_.getParamLength("Radius");
  double sectionThickness   = content_.getParamLength("Thickness");
  string sectionMatName         = content_.getParamString("Material");
  TGeoMedium* sectionMedium     = gGeoManager->GetMedium(sectionMatName.c_str());

  if (start_ > end_)
    swap(start_, end_);

  // create crotch shape containers for +/- streams
  vector<TGeoShape*> crotchInn[2];
  vector<TGeoShape*> crotchOut[2];
  vector<TGeoCombiTrans*> crotchTrans[2];
  double crotchMaxAngle[2];

  // --- Loop content hierachy - Stream (HER/LER,Up/Downstream) -> Section (Inner/Mid/Outer etc) -> Pipe number
  int nStream = content_.getNumberNodes("Streams/Stream");

  for (int iStream = 1; iStream <= nStream; ++iStream) {
    GearDir streamContent(content_);
    streamContent.append((format("Streams/Stream[%1%]/") % (iStream)).str());

    // --- Collect stream information
    string streamName    = streamContent.getParamString("attribute::name");
    int streamDirZ = int (streamContent.getParamNumValue("Direction"));   // ** is there an Int param class?
    TGeoRotation* streamRot = new TGeoRotation();
    if (streamDirZ < 0)
      streamRot->SetAngles(0, 180, 0);                          // ReflectZ(kTRUE) causes visualisation problems

    // initialise previous pipe parameter variables
    TVector3 pipePrevEnd(0., 0., 0.5*streamDirZ*length + offsetZ);
    double   pipePrevRadEnd(IPBeampipeRad);
    double   pipePrevRotY(0);

    int nSection = streamContent.getNumberNodes("Section");

    for (int iSection = 1; iSection <= nSection; ++iSection) {
      GearDir sectionContent(streamContent);
      sectionContent.append((format("Section[%1%]/") % (iSection)).str());

      // --- Collect section information
      string sectionName  = sectionContent.getParamString("attribute::name");
      sectionThickness    = sectionContent.getParamLength("Thickness"); // **to do: use previous if undefined
      sectionMatName      = sectionContent.getParamString("Material");  // **to do: use previous if undefined
      sectionMedium       = gGeoManager->GetMedium(sectionMatName.c_str());
      bool sectionCrotch      = bool (sectionContent.getParamNumValue("Crotch")); //** to do: use default 0 if undefined

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

        cout << "Entered pipe: " << pipeName << endl;

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
        double pipeRotY   = pipePrevRotY + pipeAngle;
        double sint       = sin(pipeRotY * deg);
        double cost       = cos(pipeRotY * deg);
        TVector3 pipeEnd  = 0.5 * TVector3(pipeLength * sint, 0., streamDirZ * pipeLength * cost);
        TVector3 pipePos  = pipePrevEnd + pipeEnd;

        // --- Add pipe
        double pipeRadInn = pipePrevRadEnd;
        double pipeRadOut = pipePrevRadEnd + sectionThickness;
        double pipeRadChange  = pipeRadEnd - pipePrevRadEnd;

        TGeoRotation* pipeRot = new TGeoRotation(pipeName.c_str(), 90., pipeRotY, 0.);
        pipeRot->MultiplyBy(streamRot, kFALSE);
        TGeoCombiTrans* pipeTrans = new TGeoCombiTrans(pipeName.c_str(), pipePos.x(), 0.0, pipePos.z(), pipeRot);
        pipeTrans->RegisterYourself();

        if (!sectionCrotch) {
          // --- Create outer pipes
          if (solid_) pipeRadInn = 0;
          TGeoShape* pipeShape = GeoBelleIIPipe(pipeName.c_str(),
                                                0.5 * pipeLength,
                                                pipeRadInn,
                                                pipeRadOut,
                                                pipeRadChange,
                                                pipeAngle,
                                                pipeNextAngle);

          shape_.push_back(pipeShape);
          medium_.push_back(sectionMedium);
          trans_.push_back(pipeTrans);

        } else {
          // --- Store crotch pipe inner/outer components for construction
          string crotchNameInn = pipeName + string("Inn");
          string crotchNameOut = pipeName + string("Out");
          TGeoShape* CrotchInnShape = GeoBelleIIPipe(crotchNameInn.c_str(),
                                                     0.5 * pipeLength,
                                                     0,
                                                     pipeRadInn,
                                                     pipeRadChange,
                                                     pipeAngle,
                                                     pipeNextAngle);
          TGeoShape* CrotchOutShape = GeoBelleIIPipe(crotchNameOut.c_str(),
                                                     0.5 * pipeLength,
                                                     0,
                                                     pipeRadOut,
                                                     pipeRadChange,
                                                     pipeAngle,
                                                     pipeNextAngle);

          int side;
          streamDirZ > 0 ? side = 0 : side = 1;

          crotchInn[side].push_back(CrotchInnShape);
          crotchOut[side].push_back(CrotchOutShape);
          crotchTrans[side].push_back(pipeTrans);

          if (iPipe == 1 && fabs(pipeAngle) > fabs(crotchMaxAngle[side]))
            crotchMaxAngle[side] = pipeAngle;
        }

        // store pipe information for next segment
        pipePrevRotY   = pipeRotY;
        pipePrevEnd    = pipePos + pipeEnd;
        pipePrevRadEnd = pipeRadEnd;
      }
    }
  }

  // --- Combine crotches on both sides when all components have been registered
  double* IPChamberOverlap = new double[2];
  IPChamberOverlap[0] = 0;
  IPChamberOverlap[1] = 0;
  for (int side = 0; side < 2; side++) {
    int nCrotchPipe = (int)crotchInn[side].size();
    if (!nCrotchPipe)
      continue;
    int crotchDirZ = 1;
    TGeoRotation* crotchRot = new TGeoRotation();
    string crotchName = string(name_) + "LERUpstreamCrotch";
    if (side == 1) {
      crotchDirZ = -1;
      crotchRot->SetAngles(0, 180, 0);
      crotchName = string(name_) + "HERUpstreamCrotch";
    }
    cout << "Combining " << nCrotchPipe << " pipes for crotch at " << crotchName << endl;

    // --- Create extention to IP Beampipe to meet with crotch pipe
    double extRadOut    = IPBeampipeRad + sectionThickness;
    double extLength  = extRadOut * tan(0.5 * fabs(crotchMaxAngle[side]) * deg);
    double extPosZ      = 0.5 * crotchDirZ * length + offsetZ - 0.5 * crotchDirZ * extLength;
    string extName      = crotchName + "Ext";
    string extInnName   = crotchName + "ExtInn";
    string extOutName   = crotchName + "ExtOut";

    TGeoShape* extInnShape = GeoBelleIIPipe(extInnName.c_str(), 0.5 * extLength, 0, IPBeampipeRad, 0 , 0, 0);
    TGeoShape* extOutShape = GeoBelleIIPipe(extOutName.c_str(), 0.5 * extLength, 0, extRadOut, 0, 0, 0);

    TGeoCombiTrans* extTrans = new TGeoCombiTrans(extName.c_str(), 0.0, 0.0, extPosZ, crotchRot);
    extTrans->RegisterYourself();

    cout << "IP Chamber extension created: " << extName << endl;

    // Add outer crotch shapes
    string arg = string(extOutShape->GetName()) + ":" + string(extTrans->GetName());
    for (int i = 0; i < nCrotchPipe; i++)
      arg += "+" + string(crotchOut[side][i]->GetName()) + ":" + string(crotchTrans[side][i]->GetName());
    // Subtract inner crotch shapes
    if (!solid_) {
      arg += "-" + string(extInnShape->GetName()) + ":" + string(extTrans->GetName());
      for (int i = 0; i < nCrotchPipe; i++)
        arg += "-" + string(crotchInn[side][i]->GetName()) + ":" + string(crotchTrans[side][i]->GetName());
    }

    cout << "Creating crotch pipe: " << crotchName << " with arguments:" << endl << arg << endl;

    TGeoCompositeShape* crotchShape = new TGeoCompositeShape(crotchName.c_str(), arg.c_str());

    shape_.push_back(crotchShape);
    medium_.push_back(sectionMedium);
    trans_.push_back(new TGeoCombiTrans());
    IPChamberOverlap[side] = extLength;

    cout << "Crotch pipe defined. " << endl;
  }

  return IPChamberOverlap;
}


void GeoIRBelleII::create(GearDir& content)
{
  // --- Collect global parameters
  double rotAngle   = content.getParamAngle("Rotation") / deg;

  double zMin   = content.getParamLength("RangeStart");
  double zMax     = content.getParamLength("RangeEnd");
  if (zMin > zMax)
    swap(zMin, zMax);

  // volGrpBP is aligned and centred on BelleII IP
  TGeoRotation* geoRot = new TGeoRotation("BeamPipeRot", 90.0, rotAngle, 0.0);
  TGeoVolumeAssembly* volGrpBP = addSubdetectorGroup("IR", new TGeoCombiTrans(0.0, 0.0, 0.0, geoRot));

  // -------------------------------------------------
  // ---  Build IR chamber streams ---
  // --

  vector<TGeoShape*>IRPipe;
  vector<TGeoMedium*>IRMed;
  vector<TGeoCombiTrans*>IRTrans;

  double* overlap = createPipe("", IRPipe, IRMed, IRTrans, content, zMin, zMax, 0);
  cout << "IR Chamber defined: returned - " << overlap[0] << ": " << overlap[1] << endl;

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
    if ((zMin && shieldStart < 0 && shieldStart <= zMin) || (zMax && shieldStart > 0 && shieldStart >= zMax))
      break;
    string shieldBulkName = shieldName + "Bulk";
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

    double* dummy = createPipe("ShieldPipe", shieldPipe, shieldPipeMed, shieldPipeTrans, content, shieldStart, shieldEnd, 1);

    string arg = shieldBulkName + ":" + shieldTrans->GetName();
    for (int i = 0; i < (int)shieldPipe.size(); i++)
      arg += "-" + string(shieldPipe[i]->GetName()) + ":" + string(shieldPipeTrans[i]->GetName());

    TGeoCompositeShape* shield = new TGeoCompositeShape(shieldName.c_str(), arg.c_str());

    TGeoVolume* shieldVol = new TGeoVolume(shieldName.c_str(), shield, shieldMed);
    shieldVol->SetLineColor(kCyan + 4);
    volGrpBP->AddNode(shieldVol, 1);
  }

  // -------------------------------------------------
  // ---  Build IP Chamber ---
  // --

  double length     = content.getParamLength("Length");   // full length
  double offsetZ    = content.getParamLength("OffsetZ");
  length -= (overlap[0] + overlap[1]);
  offsetZ -= 0.5 * (overlap[0] - overlap[1]);

  int nShell = content.getNumberNodes("Shells/Shell");

  for (int iShell = 1; iShell <= nShell; ++iShell) {
    GearDir shellContent(content);
    shellContent.append((format("Shells/Shell[%1%]/") % (iShell)).str());

    //Collect information
    string shellName    = shellContent.getParamString("attribute::name");
    double innerRadius  = shellContent.getParamLength("InnerRadius");
    double outerRadius  = shellContent.getParamLength("OuterRadius");
    string shellMatName = shellContent.getParamString("Material");

    //Get Material
    TGeoMedium* shellMed = gGeoManager->GetMedium(shellMatName.c_str());

    //Create Tube
    TGeoVolume* shellTube = gGeoManager->MakeTube(shellName.c_str(), shellMed, innerRadius, outerRadius, 0.5 * length);
    shellTube->SetLineColor(kOrange + 3);
    volGrpBP->AddNode(shellTube, 1, new TGeoTranslation(0.0, 0.0, offsetZ));    //translation moved to here
  }
}
