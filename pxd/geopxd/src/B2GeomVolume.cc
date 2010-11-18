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

void B2GeomVolume::resetBasicParameters()
{
  tVolume = NULL;
  tMedium = NULL;
  components = NULL;
  offset = new B2GeomOffset();
  isCalculatedOffset = false;
  nComponents = 0;
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
  isReset = true;
  isInitBasicParameters = false;
}

TGeoVolume* B2GeomVolume::getVol()
{
  //printf("B2GeomVolume::getVol start!\n");
  if (!isReset) {
    printf("B2GeomVolume: Please run resetBasicParameters() method before any other method!\n");
    return NULL;
  }
  correctDensity();
  //printf("B2GeomVolume::getVol stop!\n");
  return tVolume;
}

Bool_t B2GeomVolume::initOffsets(GearDir& content)
{
  if (!isReset) {
    printf("B2GeomVolume: Please run resetBasicParameters() method before any other method!\n");
    return false;
  }
  // read offsets of ideal position, if available
  if (content.isParamAvailable("OffsetW")) offset->fOffsetW = content.getParamLength("OffsetW");
  if (content.isParamAvailable("OffsetU")) offset->fOffsetU = content.getParamLength("OffsetU");
  if (content.isParamAvailable("OffsetV")) offset->fOffsetV = content.getParamLength("OffsetV");
  if (content.isParamAvailable("OffsetPhi")) offset->fOffsetPhi = TMath::RadToDeg() *  content.getParamAngle("OffsetPhi");
  if (content.isParamAvailable("OffsetTheta")) offset->fOffsetTheta = TMath::RadToDeg() * content.getParamAngle("OffsetTheta");
  if (content.isParamAvailable("OffsetPsi")) offset->fOffsetPsi = TMath::RadToDeg() * content.getParamAngle("OffsetPsi");
  return true;
}

Bool_t B2GeomVolume::initBasicParameters(GearDir& con)
{
  if (!isReset) {
    printf("B2GeomVolume: Please run resetBasicParameters() method before any other method!\n");
    return false;
  }
  isInitBasicParameters = true;
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
        printf("B2ERROR! No material provided in XML file\n");
        printf("Current XPath is: %s\n", content.getDirPath().c_str());
        return false;
      }
    }
    if ((fThickness < 0) && content.isParamAvailable("Thickness")) fThickness = content.getParamLength("Thickness");
    if (fThickness2 < 0) {
      if (content.isParamAvailable("Thickness2")) {
        fThickness2 = content.getParamLength("Thickness2");
      } else {
        if (fThickness > 0) fThickness2 = fThickness;
      }
    }
    if ((fWidth < 0) && content.isParamAvailable("Width")) fWidth = content.getParamLength("Width");
    if (fWidth2 < 0) {
      if (content.isParamAvailable("Width2")) {
        fWidth2 = content.getParamLength("Width2");
      } else {
        if (fWidth > 0) fWidth2 = fWidth;
      }
    }
    if ((fLength < 0) && content.isParamAvailable("Length")) fLength = content.getParamLength("Length");
    if ((fInnerRadius < 0) && content.isParamAvailable("InnerRadius")) fInnerRadius = content.getParamLength("InnerRadius");
    if ((fOuterRadius < 0) && content.isParamAvailable("OuterRadius")) fOuterRadius = content.getParamLength("OuterRadius");
    if ((iColor < 0) && content.isParamAvailable("Color")) iColor = content.getParamNumValue("Color");
    if (!goToParentNode(content)) break;
  }

  if (fMass > 0) {
    sprintf(name, "%s_Mass_%i_mg", name, (int)(fMass*1000));
  }

  if (fDensityFactor > 0) {
    sprintf(name, "%s_DensityFactor_%i_E-3", name, (int)(fDensityFactor*1000));
  }
  return true;
}

Bool_t B2GeomVolume::correctDensity()
{
  if (!isInitBasicParameters) {
    printf("B2GeomVolume: Please run initBasicParameters() before using correctDensity()!\n");
    return NULL;
  }
  //check if density has already been corrected
  if (isDensityCorrected) return true;

  // check if the volume is defined
  if (tVolume == NULL) return false;

  // don't correct density if volume is a TGeoVolumeAssembly
  if (strcmp(tVolume->ClassName(), "TGeoVolumeAssembly") == 0) return true;

  // correct density
  if ((fMass > 0) || (fDensityFactor > 0)) {

    // get volume without the daughter volumes!
    Double_t fVolume = tVolume->GetShape()->Capacity();
    if (tVolume->GetNdaughters()) fVolume = fVolume - tVolume->Capacity();

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

    tMedium = newMedium;
    tVolume->SetMedium(tMedium);
  }
  // make sure that the density is only changed once for the volume
  isDensityCorrected = true;
  return true;
}

TGeoHMatrix* B2GeomVolume::getPosition()
{
  //printf("B2GeomVolume::getPosition start!\n");
  if (!isReset) {
    printf("B2GeomVolume: Please run resetBasicParameters() method before any other method!\n");
    return NULL;
  }

  TGeoHMatrix hmaHelp;
  hmaHelp = gGeoIdentity;
  // first rotate volume to right position
  TGeoRotation rot("rot", fPhiLocal, fThetaLocal, fPsiLocal);
  hmaHelp = rot * hmaHelp;
  // second rotate about the offset
  TGeoRotation rotOffset("rotoffset", getOffset()->fOffsetPhi, getOffset()->fOffsetTheta, getOffset()->fOffsetPsi);
  hmaHelp = rotOffset * hmaHelp;
  // third, mirror volume if necessary
  if (isReflectX) hmaHelp.ReflectX(true);
  if (isReflectY) hmaHelp.ReflectY(true);
  if (isReflectZ) hmaHelp.ReflectZ(true);
  // forth do (local) offset in WUV
  TGeoTranslation traOffset(getOffset()->fOffsetW, getOffset()->fOffsetU, getOffset()->fOffsetV);
  hmaHelp = traOffset * hmaHelp;
  // fith go to position of this volume in its mother volume
  TGeoTranslation tra(fWPosition, fUPosition, fVPosition);
  hmaHelp = tra * hmaHelp;
  TGeoHMatrix* hmaPosition = new TGeoHMatrix(hmaHelp);
  //printf("B2GeomVolume::getPosition stop!\n");
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

TGeoHMatrix* B2GeomVolume::getIdealPosition()
{
  // rotate the volume to the right coordinate system
  TGeoRotation rot1("name", fPhiLocal, fThetaLocal, fPsiLocal);
  // position the volume
  TGeoTranslation tra1(fWPosition, fUPosition, fVPosition);
  TGeoHMatrix hmaHelp;
  hmaHelp = gGeoIdentity;
  hmaHelp = rot1 * hmaHelp;
  if (isReflectX) hmaHelp.ReflectX(true);
  if (isReflectY) hmaHelp.ReflectY(true);
  if (isReflectZ) hmaHelp.ReflectZ(true);
  hmaHelp = tra1 * hmaHelp;
  TGeoHMatrix* hmaPosition = new TGeoHMatrix(hmaHelp);
  return hmaPosition;
}


B2GeomOffset* B2GeomVolume::getOffset()
{
  if (!isReset) {
    printf("B2GeomVolume: Please run resetBasicParameters() method before any other method!\n");
    return NULL;
  }
  // calculate the offset from the offset of the (sub components)
  // offset of this volume = average of offsets of sub components
  // first the average of the LOCAL offsets of the components is calculated in the coordinates of THIS volume
  // then the average offset is subtracted from each component in its LOCAL coordinates
  if (!isCalculatedOffset) {
    for (int iComponent = 0; iComponent < nComponents; iComponent++) {
      // rotate the axis of THIS coordinate system to the LOCAL coordinatesystem of the component
      TGeoRotation rotComponent("rotComponent", components[iComponent]->getPhiLocal(), components[iComponent]->getThetaLocal(), components[iComponent]->getPsiLocal());
      // rotation about the component offset angles in the LOCAL coordinate system of the component
      TGeoRotation rotComponentOffsetLocal("rotComponentOffsetLocal", components[iComponent]->getOffset()->fOffsetPhi, components[iComponent]->getOffset()->fOffsetTheta, components[iComponent]->getOffset()->fOffsetPsi);
      // calculate the offset angles in the coordinate system of THIS volume
      TGeoRotation rotComponentOffsetThis("rotComponentOffsetThis");

      rotComponentOffsetThis = rotComponent;
      rotComponentOffsetThis = rotComponentOffsetLocal * rotComponentOffsetThis;

      rotComponentOffsetThis = rotComponent.Inverse() * rotComponentOffsetThis;
      Double_t fComponentOffsetPhi, fComponentOffsetTheta, fComponentOffsetPsi;
      rotComponentOffsetThis.GetAngles(fComponentOffsetPhi, fComponentOffsetTheta, fComponentOffsetPsi);

      // add rotation offset of this component
      offset->fOffsetPhi = offset->fOffsetPhi + fComponentOffsetPhi / nComponents;
      offset->fOffsetTheta = offset->fOffsetTheta + fComponentOffsetTheta  / nComponents;
      offset->fOffsetPsi = offset->fOffsetPsi +  fComponentOffsetPsi / nComponents;

      // get translation offset of this component in its local coordinate system
      Double_t fComponentTransOffsetsLocal[3];
      fComponentTransOffsetsLocal[0] = components[iComponent]->getOffset()->fOffsetW;
      fComponentTransOffsetsLocal[1] = components[iComponent]->getOffset()->fOffsetU;
      fComponentTransOffsetsLocal[2] = components[iComponent]->getOffset()->fOffsetV;

      // transform the translation offset from LOCAL coordinate system to THIS coordinatesystem
      Double_t fComponentTransOffsetsThis[3];
      rotComponent.LocalToMasterVect(fComponentTransOffsetsLocal, fComponentTransOffsetsThis);

      offset->fOffsetW = offset->fOffsetW + fComponentTransOffsetsThis[0] / nComponents;
      offset->fOffsetU = offset->fOffsetU + fComponentTransOffsetsThis[1] / nComponents;
      offset->fOffsetV = offset->fOffsetV + fComponentTransOffsetsThis[2] / nComponents;
    }
    for (int iComponent = 0; iComponent < nComponents; iComponent++) {
      // rotate the axis of THIS coordinate system to the LOCAL coordinatesystem of the component
      TGeoRotation rotComponent("rotComponent", components[iComponent]->getPhiLocal(), components[iComponent]->getThetaLocal(), components[iComponent]->getPsiLocal());
      // rotation about the component offset angles in the LOCAL coordinate system of the component
      TGeoRotation rotComponentOffsetThis("rotComponentOffsetThis", offset->fOffsetPhi, offset->fOffsetTheta, offset->fOffsetPsi);
      // calculate the offset angles in the coordinate system of the LOCAL coordinate system of the component
      TGeoRotation rotComponentOffsetLocal("rotComponentOffsetLocal");

      rotComponentOffsetLocal = rotComponent.Inverse();
      rotComponentOffsetLocal =  rotComponentOffsetThis * rotComponentOffsetLocal;

      rotComponentOffsetLocal = rotComponent * rotComponentOffsetLocal;
      Double_t fComponentOffsetPhi, fComponentOffsetTheta, fComponentOffsetPsi;
      rotComponentOffsetLocal.GetAngles(fComponentOffsetPhi, fComponentOffsetTheta, fComponentOffsetPsi);

      // subtract the offset taken by THIS volume from the offset of the component
      components[iComponent]->getOffset()->fOffsetPhi = components[iComponent]->getOffset()->fOffsetPhi - fComponentOffsetPhi;
      components[iComponent]->getOffset()->fOffsetTheta = components[iComponent]->getOffset()->fOffsetTheta - fComponentOffsetTheta;
      components[iComponent]->getOffset()->fOffsetPsi = components[iComponent]->getOffset()->fOffsetPsi - fComponentOffsetPsi;

      // transform the translation offset from LOCAL coordinate system to THIS coordinatesystem
      Double_t fTransOffsetsThis[3];
      fTransOffsetsThis[0] = offset->fOffsetW;
      fTransOffsetsThis[1] = offset->fOffsetU;
      fTransOffsetsThis[2] = offset->fOffsetV;

      // subtract the offset taken by THIS volume from the offset of the components
      Double_t fTransOffsetsLocal[3];
      rotComponent.MasterToLocalVect(fTransOffsetsThis, fTransOffsetsLocal);
      components[iComponent]->getOffset()->fOffsetW = components[iComponent]->getOffset()->fOffsetW - fTransOffsetsLocal[0];
      components[iComponent]->getOffset()->fOffsetU = components[iComponent]->getOffset()->fOffsetU - fTransOffsetsLocal[1];
      components[iComponent]->getOffset()->fOffsetV = components[iComponent]->getOffset()->fOffsetV - fTransOffsetsLocal[2];
    }
    isCalculatedOffset = true;
  }
  return offset;
}

bool B2GeomVolume::goToParentNode(GearDir& content, bool isShowDebug)
{
  string dirPath = content.getDirPath();
  if (isShowDebug) printf("---\n");
  if (isShowDebug) printf("%s\n", dirPath.c_str());
  dirPath.erase(dirPath.end() - 1);
  int iParentNodePos = dirPath.find_last_of("/");
  if (iParentNodePos > 0) {
    content.setDirPath(dirPath.substr(0, iParentNodePos + 1));
    dirPath = content.getDirPath();
    if (isShowDebug) printf("%s\n", dirPath.c_str());
    if (isShowDebug) printf("---\n");
    return true;
  } else {
    if (isShowDebug) printf("***\n");
    return false;
  }
}

Bool_t B2GeomVolume::init(GearDir& content, string subDir)
{
  GearDir myContent(content);
  myContent.append(subDir);
  initBasicParameters(myContent);
  return true;
}


Bool_t B2GeomVolume::makeGeneric()
{
  tVolume = (TGeoVolume*) gROOT->FindObjectAny(name);
  //printf("shape: %s\n", shape);
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
    return false;
  }
}

Bool_t B2GeomVolume::make()
{
  return makeGeneric();
}

