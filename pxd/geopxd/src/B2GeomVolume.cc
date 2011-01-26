/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Oswald            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/geopxd/B2GeomVolume.h>
using namespace Belle2;
using namespace std;

B2GeomOffset::B2GeomOffset()
{
  fOffsetW = 0;
  fOffsetU = 0;
  fOffsetV = 0;
  fOffsetPhi = 0;
  fOffsetTheta = 0;
  fOffsetPsi = 0;
}

B2GeomOffset::~B2GeomOffset()
{

}

B2GeomVolume::B2GeomVolume()
{
  tVolume = NULL;
  tMedium = NULL;
  fMass = -1.;
  fDensityFactor = -1.;
  fLength = -1;
  fWidth = -1;
  fWidth2 = -1;
  fThickness = -1;
  fThickness2 = -1;
  fOuterRadius = -1;
  fInnerRadius = -1;
  fThickness = -1;
  fWPosition = 0;
  fUPosition = 0;
  fVPosition = 0;
  fPhiLocal = 0;
  fThetaLocal = 0;
  fPsiLocal = 0;
  iColor = -1;
  isReflectX = false;
  isReflectY = false;
  isReflectZ = false;
  isDensityCorrected = false;
  isInitBasicParameters = false;
  sprintf(name, "NULL");
}

TGeoVolume* B2GeomVolume::getVol()
{
  ////B2METHOD();
  // check if parameters have been initialized from GearBox
  if (!isInitBasicParameters) {
    B2ERROR("Please run initBasicParameters() method before getVol()!\n");
    return NULL;
  }
  correctDensity();
  return tVolume;
}

Bool_t B2GeomVolume::initBasicParameters(GearDir& con)
{
  //B2METHOD();
  // 'Copy' GearDir con for further manipulation
  GearDir content(con);

  // How to position the volume in its mother volume?
  if (content.isParamAvailable("ReflectX")) isReflectX = true;
  if (content.isParamAvailable("ReflectY")) isReflectY = true;
  if (content.isParamAvailable("ReflectZ")) isReflectZ = true;
  if (content.isParamAvailable("WPosition")) fWPosition = content.getParamLength("WPosition");
  if (content.isParamAvailable("UPosition")) fUPosition = content.getParamLength("UPosition");
  if (content.isParamAvailable("VPosition")) fVPosition = content.getParamLength("VPosition");
  if (content.isParamAvailable("PhiLocal")) fPhiLocal = TMath::RadToDeg() * content.getParamAngle("PhiLocal");
  if (content.isParamAvailable("ThetaLocal")) fThetaLocal = TMath::RadToDeg() * content.getParamAngle("ThetaLocal");
  if (content.isParamAvailable("PsiLocal")) fPsiLocal = TMath::RadToDeg() * content.getParamAngle("PsiLocal");

  // read density corrections
  if (content.isParamAvailable("DensityFactor")) fDensityFactor = content.getParamLength("DensityFactor");
  if (content.isParamAvailable("Mass")) fMass = content.getParamLength("Mass");

  // read shape of component
  if (content.isParamAvailable("Shape")) sprintf(shape, "%s", content.getParamString("Shape").c_str());

  // read name of component
  if (content.isParamAvailable("Name")) sprintf(name, "%s", content.getParamString("Name").c_str());

  // read material and dimensions, if parameter not found, look for it in parameters of the mother volume
  while (true) {
    if (tMedium == NULL) {
      if (content.isParamAvailable("Material")) {
        string materialName  = content.getParamString("Material");
        tMedium = gGeoManager->GetMedium(materialName.c_str());
      } else {
        B2ERROR("B2ERROR! No material provided in XML file");
        B2DEBUG(10, "Current XPath is:" << content.getDirPath());
        return false;
      }
    }

    // get thickness if not defined before
    if ((fThickness < 0) && content.isParamAvailable("Thickness")) fThickness = content.getParamLength("Thickness");
    // check if thickness2 has been defined before
    if (fThickness2 < 0) {
      // check if thickness2 is available in XML file
      if (content.isParamAvailable("Thickness2")) {
        // take thickness2 from XML file
        fThickness2 = content.getParamLength("Thickness2");
      } else {
        // if thickness is already defined, take same value for thickness2
        if (fThickness > 0) fThickness2 = fThickness;
      }
    }

    // get width if not defined before
    if ((fWidth < 0) && content.isParamAvailable("Width")) fWidth = content.getParamLength("Width");
    // check if width2 has been defined before
    if (fWidth2 < 0) {
      // check if with2 is available in XML file
      if (content.isParamAvailable("Width2")) {
        // take width2 form XML file
        fWidth2 = content.getParamLength("Width2");
      } else {
        // if width is already defined, take same value for width2
        if (fWidth > 0) fWidth2 = fWidth;
      }
    }
    // get length if not defined before
    if ((fLength < 0) && content.isParamAvailable("Length")) fLength = content.getParamLength("Length");
    // get inner radius if not defined before
    if ((fInnerRadius < 0) && content.isParamAvailable("InnerRadius")) fInnerRadius = content.getParamLength("InnerRadius");
    // get outer radius if not defined before
    if ((fOuterRadius < 0) && content.isParamAvailable("OuterRadius")) fOuterRadius = content.getParamLength("OuterRadius");
    // get color if not defined before
    if ((iColor < 0) && content.isParamAvailable("Color")) iColor = static_cast<Int_t>(content.getParamNumValue("Color"));
    // go one node up in GearDir or exit this loop
    if (!goToParentNode(content)) break;
  }

  // change the name of this volume if any density correction is applied
  if (fMass > 0) {
    sprintf(name, "%s_Mass_%i_mg", name, (int)(fMass*1000));
  }
  if (fDensityFactor > 0) {
    sprintf(name, "%s_DensityFactor_%ie-3", name, (int)(fDensityFactor*1000));
  }
  isInitBasicParameters = true;
  return true;
}

Bool_t B2GeomVolume::correctDensity()
{
  // check if parameters are initialized
  if (!isInitBasicParameters) {
    B2ERROR("Please run initBasicParameters() before using correctDensity()!\n");
    return false;
  }

  //check if density has already been corrected
  if (isDensityCorrected) return true;

  // check if the volume is defined
  if (tVolume == NULL) return false;

  // don't correct density if volume is a TGeoVolumeAssembly
  if (strcmp(tVolume->ClassName(), "TGeoVolumeAssembly") == 0) return true;

  // correct density
  if ((fMass > 0) || (fDensityFactor > 0)) {

    // get volume of THIS node
    Double_t fVolume = tVolume->Capacity();

    // subtract the volume  of all daughter nodes
    TObjArray* tDaughterNodeArray = tVolume->GetNodes();
    if (tDaughterNodeArray != NULL) {
      for (int iDaughter = 0; iDaughter < tDaughterNodeArray->GetEntriesFast(); iDaughter++) {
        TGeoNode* tDaughterNode = (TGeoNode*) tDaughterNodeArray->At(iDaughter);
        fVolume = fVolume - tDaughterNode->GetVolume()->Capacity();
      }
    }

    // calculate correct density
    Double_t fDensity = tVolume->GetMaterial()->GetDensity();
    if (fMass > 0) fDensity = fMass / fVolume;
    if (fDensityFactor > 0) fDensity = fDensityFactor * fDensity;

    // define material name according to new density
    char newMaterialName[200];
    sprintf(newMaterialName, "%s_Density_%i_mg_cm3", tVolume->GetMaterial()->GetName(), int(fDensity*1000));
    TGeoMaterial* newMaterial = gGeoManager->GetMaterial(newMaterialName);
    if (!newMaterial) {
      newMaterial = (TGeoMaterial*) tVolume->GetMaterial()->Clone(newMaterialName);
      newMaterial->SetDensity(fDensity);
      gGeoManager->AddMaterial(newMaterial);
    }

    // check if same medium has been defined before
    TGeoMedium* newMedium = gGeoManager->GetMedium(newMaterialName);
    if (!newMedium) newMedium = new TGeoMedium(newMaterialName, 1, newMaterial);

    // set the medium of this volume to the new medium (with corrected density)
    tMedium = newMedium;
    tVolume->SetMedium(tMedium);
  }
  // make sure that the density is only changed once for the volume
  isDensityCorrected = true;
  return true;
}

TGeoHMatrix* B2GeomVolume::getPosition()
{
  // //B2METHOD();
  if (!isInitBasicParameters) {
    B2ERROR("Please run initBasicParameters() method before getPosition()!\n");
    return NULL;
  }

  TGeoHMatrix hmaHelp;
  hmaHelp = gGeoIdentity;
  // first rotate volume to right position
  TGeoRotation rot("rot", fPhiLocal, fThetaLocal, fPsiLocal);
  hmaHelp = rot * hmaHelp;
  // second, mirror volume if necessary
  if (isReflectX) hmaHelp.ReflectX(true);
  if (isReflectY) hmaHelp.ReflectY(true);
  if (isReflectZ) hmaHelp.ReflectZ(true);
  // third go to position of this volume in its mother volume
  TGeoTranslation tra(fWPosition, fUPosition, fVPosition);
  hmaHelp = tra * hmaHelp;
  TGeoHMatrix* hmaPosition = new TGeoHMatrix(hmaHelp);
  return hmaPosition;
}

void B2GeomVolume::setPosition(TGeoHMatrix* newPosition)
{
  if (newPosition->IsTranslation()) {
    Double_t* fXYZ;
    fXYZ = newPosition->GetTranslation();
    fWPosition = fXYZ[0];
    fUPosition = fXYZ[1];
    fVPosition = fXYZ[2];
  }
  if (newPosition->IsRotation()) {
    TGeoRotation rot("rot");
    rot.SetMatrix(newPosition->GetRotationMatrix());
    rot.GetAngles(fPhiLocal, fThetaLocal, fPsiLocal);
  }
}

bool B2GeomVolume::goToParentNode(GearDir& content)
{
  string dirPath = content.getDirPath();
  B2DEBUG(110, "---\n" << dirPath << "\n");
  dirPath.erase(dirPath.end() - 1);
  int iParentNodePos = dirPath.find_last_of("/");
  if (iParentNodePos > 0) {
    content.setDirPath(dirPath.substr(0, iParentNodePos + 1));
    dirPath = content.getDirPath();
    B2DEBUG(110, dirPath << "\n---\n")
    return true;
  } else {
    return false;
  }
}

Bool_t B2GeomVolume::makeGeneric()
{
  //B2METHOD();
  tVolume = (TGeoVolume*) gROOT->FindObjectAny(name);
  B2DEBUG(110, "shape:" << string(shape) << "\n");
  if (tVolume) return true;
  if (strncmp(shape, "Trd2", 4) == 0) {
    tVolume = gGeoManager->MakeTrd2(name, tMedium,
                                    0.5 * fThickness,
                                    0.5 * fThickness2,
                                    0.5 * fWidth,
                                    0.5 * fWidth2,
                                    0.5 * fLength);
    if (iColor > 0) tVolume->SetLineColor(iColor);
    return true;
  } else if (strncmp(shape, "Box", 3) == 0) {
    tVolume = gGeoManager->MakeBox(name, tMedium,
                                   0.5 * fThickness,
                                   0.5 * fWidth,
                                   0.5 * fLength);
    if (iColor > 0) tVolume->SetLineColor(iColor);
    return true;
  } else if (strncmp(shape, "Tube", 4) == 0) {
    tVolume = gGeoManager->MakeTube(name, tMedium,
                                    0.5 * fInnerRadius,
                                    0.5 * fOuterRadius,
                                    0.5 * fLength);
    if (iColor > 0) tVolume->SetLineColor(iColor);
    return true;
  } else if (strncmp(shape, "Assembly", 8) == 0) {
    tVolume = new TGeoVolumeAssembly(name);
    return true;
  } else {
    B2WARNING("Could not build " << string(name) << "!");
    return false;
  }
}


Bool_t B2GeomVolume::make()
{
  //B2METHOD();
  return makeGeneric();
}

Bool_t B2GeomVolume::makeAndAddComponent(B2GeomVolume* component)
{
  //B2METHOD();
  if (component != NULL) {
    if (component->make()) {
      tVolume->AddNode(component->getVol(), 1, component->getPosition());
    } else {
      B2WARNING("Could not build subcomponent of " << string(name));
      return false;
    }
  }
  return true;
}

//==========================================================================================
B2GeomVXDVolume::B2GeomVXDVolume()
{
  iLayer = -1;
  iLadder = -1;
  iSensor = -1;
}

string B2GeomVXDVolume::searchAndReplaceInName(string inputString)
{
  char text[10];
  // position in the string
  size_t pos = 0;

  pos = inputString.find("%Layer");
  if (pos != string::npos) {
    sprintf(text, "%i", iLayer);
    inputString.replace(pos, 6, string(text));
  }

  pos = inputString.find("%Ladder");
  if (pos != string::npos) {
    sprintf(text, "%i", iLadder);
    inputString.replace(pos, 7, string(text));
  }

  pos = inputString.find("%Sensor");
  if (pos != string::npos) {
    sprintf(text, "%i", iSensor);
    inputString.replace(pos, 7, string(text));
  }
  return inputString;
}

void B2GeomVXDVolume::setId(Int_t iLay, Int_t iLad, Int_t iSen)
{
  iLayer = iLay;
  iLadder = iLad;
  iSensor = iSen;
  sprintf(name, "%s", (searchAndReplaceInName(string(name))).c_str());
}
