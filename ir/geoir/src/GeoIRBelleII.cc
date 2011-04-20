/**************************************************************************
 *  BASF2 (Belle Analysis Framework 2)                                    *
 *  Copyright(C) 2010 - Belle II Collaboration                            *
 *                                                                        *
 *  Author: The Belle II Collaboration                                    *
 *  Contributors: Clement Ng, Andreas Moll                                *
 *                                                                        *
 *  This software is provided "as is" without any warranty.               *
 **************************************************************************/

//** todo:
// - Would be a lot better implementing a separate classes for streams, shields, ip chamber etc
//    to replace the pass vector to function method, which creates inaccessible new pointers etc

#include <ir/geoir/GeoIRBelleII.h>
#include <ir/simir/IRSensitiveDetector.h>

#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/GearboxIOAbs.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <cmath>
#include <boost/format.hpp>

#include <TMath.h>
#include <TVector3.h>
#include <TGeoMatrix.h>
#include <TGeoManager.h>
#include <TGeoVolume.h>
#include <TGeoBBox.h>
#include <TGeoPcon.h>
#include <TGeoTube.h>
#include <TGeoCone.h>
#include <TGeoCompositeShape.h>

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

  //The IR subdetector uses the "SD_" prefix to flag its sensitive volumes
  addSensitiveDetector("SD_", new IRSensitiveDetector("IRSensitiveDetector"));
}


GeoIRBelleII::~GeoIRBelleII()
{

}


TGeoShape* GeoIRBelleII::GeoBelleIIPipe(const char* name_,
                                        const double length_,           // length of pipe
                                        const double radinn1_,          // inner radius at start
                                        const double radout1_,          // outer radius at start
                                        const double radchange_,        // change in radius
                                        const double theta1_,           // angle to previous pipe
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
      B2WARNING("Warning: different end-angled cone pipes not supported: ignoring end-angles." <<
                "  Rad: " << radinn1_ << " -> " << radinn2 << "; Angle: " << theta1_ << "->" << theta2_ << endl)
    }
    tube = new TGeoConeSeg(tubeName.c_str(), length_,
                           radinn1_, radout1_,
                           radinn2, radout2,
                           0, 360);
  } else {
    // --- Form slanted pipe
    tube = new TGeoCtub(tubeName.c_str(), radinn1_, radout1_, length_,
                        0, 360,
                        sint1, 0, -cost1,
                        sint2, 0, cost2);
  }

  return tube;
}


double* GeoIRBelleII::createPipe(const char* name_,
                                 vector<TGeoShape*>& shape_,            // container of pipe shapes
                                 vector<TGeoMedium*>& medium_,          // container of pipe material
                                 vector<TGeoCombiTrans*>& trans_,       // container of pipe transformations
                                 GearDir& content_,                     // GearBox content
                                 double start_,                         // starting pipe range
                                 double end_,                           // end pipe range
                                 const bool solid_)                     // specify solid geometry
{
  // --- Collect global parameters
  double length                 = 0;
  double offsetZ                = 0;
  double IPBeampipeRad          = 0;
  if (content_.isParamAvailable("Length")) length = content_.getParamLength("Length");
  if (content_.isParamAvailable("OffsetZ")) offsetZ = content_.getParamLength("OffsetZ");
  if (content_.isParamAvailable("Radius")) IPBeampipeRad = content_.getParamLength("Radius");

  double sectionThickness       = 0;
  string sectionMatName;
  TGeoMedium* sectionMedium;

  if (start_ > end_)
    swap(start_, end_);

  // create crotch shape containers for +/- streams
  vector<TGeoShape*> crotchInn[2];
  vector<TGeoShape*> crotchOut[2];
  vector<TGeoCombiTrans*> crotchTrans[2];
  double crotchMaxAngle[2] = {0};

  // --- Loop content hierachy - Stream (HER/LER,Up/Downstream) -> Section (Inner/Mid/Outer etc) -> Pipe number
  int nStream = content_.getNumberNodes("Streams/Stream");

  for (int iStream = 1; iStream <= nStream; ++iStream) {
    // --- Reset section defaults
    if (content_.isParamAvailable("Thickness")) sectionThickness = content_.getParamLength("Thickness");
    if (content_.isParamAvailable("Material")) {
      sectionMatName = content_.getParamString("Material");
      sectionMedium  = gGeoManager->GetMedium(sectionMatName.c_str());
    }

    // --- Collect stream information
    GearDir streamContent(content_);
    streamContent.append("Streams/Stream", iStream);
    string streamName           = streamContent.getParamString("attribute::name");
    int streamDirZ              = int (streamContent.getParamNumValue("Direction"));
    TGeoRotation* streamRot     = new TGeoRotation();
    if (streamDirZ < 0)
      streamRot->SetAngles(0, 180, 0);                  // ReflectZ(kTRUE) causes visualisation problems

    // initialise previous pipe parameter variables
    TVector3 pipePrevEnd(0., 0., 0.5*streamDirZ*length + offsetZ);
    double   pipePrevRadEnd(IPBeampipeRad);
    double   pipePrevRotY(0);

    int nSection = streamContent.getNumberNodes("Section");

    for (int iSection = 1; iSection <= nSection; ++iSection) {
      GearDir sectionContent(streamContent);
      sectionContent.append("Section", iSection);

      // --- Collect section information
      string sectionName        = sectionContent.getParamString("attribute::name");
      int sectionCrotch         = 0;
      if (sectionContent.isParamAvailable("Crotch")) sectionCrotch = int (sectionContent.getParamNumValue("Crotch"));
      if (sectionContent.isParamAvailable("Thickness")) sectionThickness = sectionContent.getParamLength("Thickness");
      if (sectionContent.isParamAvailable("Material")) {
        sectionMatName = sectionContent.getParamString("Material");
        sectionMedium  = gGeoManager->GetMedium(sectionMatName.c_str());
      }

      int nPipe = sectionContent.getNumberNodes("Pipe");

      for (int iPipe = 1; iPipe <= nPipe; ++iPipe) {
        GearDir pipeContent(sectionContent);
        pipeContent.append("Pipe", iPipe);

        // --- Collect pipe information
        string pipeName   = (format("%1%%2%%3%%4%") % name_ % streamName % sectionName % iPipe).str();
        double pipeLength = pipeContent.getParamLength("Length");     // full length
        double pipeAngle  = 0;
        double pipeRadEnd = pipePrevRadEnd;
        if (pipeContent.isParamAvailable("Angle")) pipeAngle = pipeContent.getParamAngle("Angle") / deg;
        if (pipeContent.isParamAvailable("RadiusEnd")) pipeRadEnd = pipeContent.getParamLength("RadiusEnd");

        if (streamDirZ == -1 && ((start_ && pipePrevEnd.z() <= start_) || (end_   &&  pipePrevEnd.z() - pipeLength >= end_)))
          break;
        if (streamDirZ == 1  && ((end_   && pipePrevEnd.z() >= end_) || (start_ &&  pipePrevEnd.z() + pipeLength <= start_)))
          break;

        // collect information about next pipe
        GearDir pipeNextContent(streamContent);
        double pipeNextAngle = 0;
        if (iPipe < nPipe) {
          pipeNextContent.append((format("Section[%1%]/Pipe[%2%]/") % (iSection) % (iPipe + 1)).str());
          if (pipeNextContent.isParamAvailable("Angle")) pipeNextAngle = pipeNextContent.getParamAngle("Angle") / deg;
        } else if (iSection < nSection) {
          pipeNextContent.append((format("Section[%1%]/Pipe[1]/") % (iSection + 1)).str());
          if (pipeNextContent.isParamAvailable("Angle")) pipeNextAngle = pipeNextContent.getParamAngle("Angle") / deg;
        }
        // calculate position
        double pipeRotY   = pipePrevRotY + pipeAngle;
        double sint       = sin(pipeRotY * deg);
        double cost       = cos(pipeRotY * deg);
        TVector3 pipeRelEnd  = 0.5 * TVector3(pipeLength * sint, 0., streamDirZ * pipeLength * cost);   // pipe end relative to its centre
        TVector3 pipePos  = pipePrevEnd + pipeRelEnd;
        // --- Add pipe
        double pipeRadInn = pipePrevRadEnd;
        double pipeRadOut = pipePrevRadEnd + sectionThickness;
        double pipeRadChange  = pipeRadEnd - pipePrevRadEnd;
        TGeoRotation* pipeRot = new TGeoRotation(pipeName.c_str(), 90., pipeRotY, -90.);
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
          int side;
          streamDirZ > 0 ? side = 0 : side = 1;

          // --- Get info from first crotch pipe
          if (iPipe == 1) {
            if (sectionCrotch == 1) {
              if (fabs(pipeAngle) > fabs(crotchMaxAngle[side]))
                crotchMaxAngle[side] = pipeAngle;
            } else {
              // --- Recalculate radius and position for asymmetric radii crotch pipe
              pipeRadInn = pipeContent.getParamLength("RadiusStart");
              pipeRadOut = pipeRadInn + sectionThickness;
              if (!pipeContent.isParamAvailable("RadiusEnd")) pipeRadEnd = pipeRadInn;
              pipeRadChange = pipeRadEnd - pipeRadInn;

              pipePrevEnd = TVector3((0.5 * length + offsetZ) * sint, 0., 0.5 * streamDirZ * length + offsetZ);
              pipePos    = pipePrevEnd + pipeRelEnd;
              pipeTrans->SetTranslation(pipePos.x(), 0.0, pipePos.z());
              //pipeTrans->RegisterYourself();
            }
          }

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
          crotchInn[side].push_back(CrotchInnShape);
          crotchOut[side].push_back(CrotchOutShape);
          crotchTrans[side].push_back(pipeTrans);
        }

        // store pipe information for next segment
        pipePrevRotY   = pipeRotY;
        pipePrevEnd    = pipePos + pipeRelEnd;
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
    B2INFO("Combining " << nCrotchPipe << " pipes for crotch at " << crotchName)

    // --- Create extention to IP Beampipe to meet with crotch pipe
    double extRadOut    = IPBeampipeRad + sectionThickness;
    double extLength    = extRadOut * tan(0.5 * fabs(crotchMaxAngle[side]) * deg);
    double extPosZ      = 0.5 * crotchDirZ * length + offsetZ - 0.5 * crotchDirZ * extLength;
    string extName      = crotchName + "Ext";
    string extInnName   = crotchName + "ExtInn";
    string extOutName   = crotchName + "ExtOut";

    //  slant angle to next pipe included by default in order to support asym. radius crotch type
    TGeoShape* extInnShape = GeoBelleIIPipe(extInnName.c_str(), 0.5 * extLength, 0, IPBeampipeRad, 0 , 0, crotchMaxAngle[side]);
    TGeoShape* extOutShape = GeoBelleIIPipe(extOutName.c_str(), 0.5 * extLength, 0, extRadOut, 0, 0, crotchMaxAngle[side]);

    TGeoCombiTrans* extTrans = new TGeoCombiTrans(extName.c_str(), 0.0, 0.0, extPosZ, crotchRot);
    extTrans->RegisterYourself();

    B2INFO("IP Chamber extension created: " << extName)

    // Add outer crotch shapes
    string arg = string(extOutShape->GetName()) + ":" + string(extTrans->GetName());
    for (int i = 0; i < nCrotchPipe; i++) {
      arg += "+" + string(crotchOut[side][i]->GetName()) + ":" + string(crotchTrans[side][i]->GetName());
    }
    // Subtract inner crotch shapes
    if (!solid_) {
      arg += "-" + string(extInnShape->GetName()) + ":" + string(extTrans->GetName());
      for (int i = 0; i < nCrotchPipe; i++)
        arg += "-" + string(crotchInn[side][i]->GetName()) + ":" + string(crotchTrans[side][i]->GetName());
    }

    B2INFO("Creating crotch pipe: " << crotchName << " with arguments:" << endl << arg)

    TGeoCompositeShape* crotchShape = new TGeoCompositeShape(crotchName.c_str(), arg.c_str());

    shape_.push_back(crotchShape);
    medium_.push_back(sectionMedium);
    trans_.push_back(new TGeoCombiTrans());
    IPChamberOverlap[side] = extLength;

    B2INFO("Crotch pipe defined. ")
  }
  return IPChamberOverlap;
}


void GeoIRBelleII::create(GearDir& content)
{
  // --- Collect global parameters
  double rotAngle = 0;
  double zMin     = 0;
  double zMax     = 0;
  if (content.isParamAvailable("Rotation")) rotAngle = content.getParamAngle("Rotation") / deg;
  if (content.isParamAvailable("RangeStart")) zMin = content.getParamLength("RangeStart");
  if (content.isParamAvailable("RangeEnd")) zMax = content.getParamLength("RangeEnd");
  if (zMin > zMax) swap(zMin, zMax);

  // volGrpBP is aligned and centred on BelleII IP
  TGeoRotation* geoRot = new TGeoRotation("BeamPipeRot", 90.0, rotAngle, -90.0);
  TGeoVolumeAssembly* volGrpBP = addSubdetectorGroup("IR", new TGeoCombiTrans(0.0, 0.0, 0.0, geoRot));


  //-----------------------------------------------------------------
  //                 Build IR chamber streams
  //-----------------------------------------------------------------

  vector<TGeoShape*>IRPipe;
  vector<TGeoMedium*>IRMed;
  vector<TGeoCombiTrans*>IRTrans;

  double *overlap = new double[2];
  overlap[0] = 0;
  overlap[1] = 0;
  try {
    overlap = createPipe("", IRPipe, IRMed, IRTrans, content, zMin, zMax, 0);
    B2INFO("IR Chamber defined: returned - " << overlap[0] << ": " << overlap[1])

    for (int i = 0; i < (int)IRPipe.size(); i++) {
      //TGeoVolume* IRPipeVol = new TGeoVolume(IRPipe[i]->GetName(), IRPipe[i], IRMed[i]);
      //IRPipeVol->SetLineColor(kTeal + 3);
      //volGrpBP->AddNode(IRPipeVol, 1, IRTrans[i]);

      //modify start nakano110307
      TGeoPcon* IRTube = new TGeoPcon(0, 360, 2);
      IRTube->DefineSection(0, -62.725, 0, 10);
      IRTube->DefineSection(1,  63.35, 0, 10);
      IRTube->SetName("IRTubename");
      string arg2 = string(IRPipe[i]->GetName()) + ":" + string(IRTrans[i]->GetName()) + "*IRTubename";
      TGeoCompositeShape* IRPipeComp = new TGeoCompositeShape(((format("IRPipeCompName%1%") % i).str()).c_str(), arg2.c_str());
      TGeoVolume* IRPipeCompVol = new TGeoVolume(((format("IRPipeCompVolName%1%") % i).str()).c_str(), IRPipeComp, IRMed[i]);
      IRPipeCompVol->SetLineColor(kTeal + 3);
      volGrpBP->AddNode(IRPipeCompVol, 1);
      //modify end nakano110307
    }
  } catch (runtime_error& exc) {
    B2ERROR("No IR chamber streams defined ! " << exc.what())
  }

  //-----------------------------------------------------------------
  //                 Build shields
  //-----------------------------------------------------------------

  /* //modify start nakano110308
  try {
    int nShield = content.getNumberNodes("Shields/Shield");
    for (int iShield = 1; iShield <= nShield; ++iShield) {
      GearDir shieldContent(content);
      shieldContent.append("Shields/Shield", iShield);

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
        segContent.append("Seg" , iSeg);
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

      createPipe("ShieldPipe", shieldPipe, shieldPipeMed, shieldPipeTrans, content, shieldStart, shieldEnd, 1);

      string arg = shieldBulkName + ":" + shieldTrans->GetName();
      for (int i = 0; i < (int)shieldPipe.size(); i++) {
        arg += "-" + string(shieldPipe[i]->GetName()) + ":" + string(shieldPipeTrans[i]->GetName());
      }
      TGeoCompositeShape* shield = new TGeoCompositeShape(shieldName.c_str(), arg.c_str());

      TGeoVolume* shieldVol = new TGeoVolume(shieldName.c_str(), shield, shieldMed);
      shieldVol->SetLineColor(kCyan + 4);
      volGrpBP->AddNode(shieldVol, 1);
    }
  } catch (runtime_error& exc) {
    B2ERROR("No shields defined ! " << exc.what())
  }
  */ //modify start nakano110308

  //-----------------------------------------------------------------
  //                 Build IP Chamber
  //-----------------------------------------------------------------

  try {
    double length     = content.getParamLength("Length");   // full length
    double offsetZ    = 0;
    if (content.isParamAvailable("OffsetZ")) offsetZ = content.getParamLength("OffsetZ");
    length -= (overlap[0] + overlap[1]);
    offsetZ -= 0.5 * (overlap[0] - overlap[1]);

    int nShell = content.getNumberNodes("Shells/Shell");
    for (int iShell = 1; iShell <= nShell; ++iShell) {
      GearDir shellContent(content);
      shellContent.append("Shells/Shell", iShell);

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
  } catch (runtime_error& exc) {
    B2ERROR("No IP chamber defined ! " << exc.what())
  }
}
