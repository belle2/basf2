#include "TGeoManager.h"
#include "TGeoVolume.h"
#include "TGeoMaterial.h"
#include "TGeoMatrix.h"
#include <framework/gearbox/GearDir.h>
#include <framework/datastore/Units.h>
#include <string>

#ifndef B2GEOMVOLUME_H_
#define B2GEOMVOLUME_H_
namespace Belle2 {
  class GearDir;
  class B2GeomVolume {
  protected:
    TGeoVolume* tVolume;
    TGeoMedium* tMedium;
    Double_t fMass;
    Double_t fDensityFactor;
    Bool_t correctDensity();
    Bool_t initBasicParameters(GearDir& content);
    // basic parameters
    Double_t fLength;
    Double_t fWidth;
    Double_t fWidth2;
    Double_t fThickness;
    Double_t fThickness2;
    Double_t fOuterRadius;
    Double_t fInnerRadius;
    Double_t fWPosition;
    Double_t fUPosition;
    Double_t fVPosition;
    Double_t fPhiLocal;
    Double_t fThetaLocal;
    Double_t fPsiLocal;
    Bool_t isReflectX;
    Bool_t isReflectY;
    Bool_t isReflectZ;
    //! true = correctDensity() has been already called
    bool isDensityCorrected;
  public:
    B2GeomVolume();
    TGeoVolume* getVol() {
      correctDensity();
      return tVolume;
    }
    bool goToParentNode(GearDir& con, bool isShowDebug = false);
    virtual TGeoHMatrix* getPosition();
  };
}
#endif
