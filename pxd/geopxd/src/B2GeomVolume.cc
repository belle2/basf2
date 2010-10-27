#include <pxd/geopxd/B2GeomVolume.h>
using namespace Belle2;
using namespace std;

B2GeomVolume::B2GeomVolume()
{
  tVolume = NULL;
  tMedium = NULL;
  fMass = -1.;
  fDensityFactor = 1.;
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
  isReflectX = false;
  isReflectY = false;
  isReflectZ = false;
  isDensityCorrected = false;
}

Bool_t B2GeomVolume::initBasicParameters(GearDir& con)
{
  printf("Volume::initBasicParameters start\n");
  GearDir content(con);
  // How to position the volume in its mother volume?
  printf("Get position of the volume...\n");
  if (content.isParamAvailable("ReflectX")) isReflectX = true;
  if (content.isParamAvailable("ReflectY")) isReflectY = true;
  if (content.isParamAvailable("ReflectZ")) isReflectZ = true;
  if (content.isParamAvailable("WPosition")) fWPosition = content.getParamLength("WPosition");
  if (content.isParamAvailable("UPosition")) fUPosition = content.getParamLength("UPosition");
  if (content.isParamAvailable("VPosition")) fVPosition = content.getParamLength("VPosition");
  if (content.isParamAvailable("PhiLocal")) fPhiLocal = content.getParamLength("PhiLocal");
  if (content.isParamAvailable("ThetaLocal")) fThetaLocal = content.getParamLength("ThetaLocal");
  if (content.isParamAvailable("PsiLocal")) fPsiLocal = content.getParamLength("PsiLocal");
  // read density corrections
  if (content.isParamAvailable("DensityFactor")) fDensityFactor = content.getParamLength("DensityFactor");
  if (content.isParamAvailable("Mass")) fMass = content.getParamLength("Mass");

  // read material and dimensions, if parameter not found, look for it in parameters of the mother volume
  printf("Get material and dimensions of the volume...\n");
  while (true) {
    if (tMedium == NULL) {
      printf("Get material of the volume...\n");
      if (content.isParamAvailable("Material")) {
        string materialName  = content.getParamString("Material");
        tMedium = gGeoManager->GetMedium(materialName.c_str());
        printf("...material ok!\n");
      } else {
        printf("ERROR! No material provided in XML file\n");
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
    if (!goToParentNode(content)) break;
  }
  printf("Volume::initBasicParameters stop\n");
  return true;
}

Bool_t B2GeomVolume::correctDensity()
{
// check if the volume is defined
  if (tVolume == NULL) return false;
// don't correct density if volume is a TGeoVolumeAssembly
  if (strcmp(tVolume->ClassName(), "TGeoVolumeAssembly") == 0) return true;
// get volume without the daughter volumes!
  Double_t fVolume = tVolume->GetShape()->Capacity() - tVolume->Capacity();
// correct density
  if (fMass < 0) fMass = fVolume * tVolume->GetMaterial()->GetDensity();
  tVolume->GetMaterial()->SetDensity(fMass*fDensityFactor / fVolume);
  // make sure that the density is only changed once for the volume
  isDensityCorrected = false;
  return true;
}

TGeoHMatrix* B2GeomVolume::getPosition()
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
