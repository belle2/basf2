/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Oswald                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "TGeoManager.h"
#include "TGeoVolume.h"
#include "TGeoMaterial.h"
#include "TGeoMatrix.h"
#include "TMath.h"
#include "TROOT.h"
#include <framework/gearbox/GearDir.h>
#include <framework/datastore/Units.h>
#include <string>

#ifndef B2GEOMVOLUME_H_
#define B2GEOMVOLUME_H_
namespace Belle2 {
  class GearDir;

  class B2GeomOffset {
  public:
    // offsets according to local coordinates of component (in cm, radians)
    Double_t fOffsetW;
    Double_t fOffsetU;
    Double_t fOffsetV;
    Double_t fOffsetPhi;
    Double_t fOffsetTheta;
    Double_t fOffsetPsi;
    B2GeomOffset();
    ~B2GeomOffset();
  };

  class B2GeomVolume {
  protected:
    char name[200];
    char shape[200];
    TGeoVolume* tVolume;
    // the sub components of this volume
    // this = ladder => components = Sensors
    // this = layer => components = Ladders
    B2GeomVolume** components;
    B2GeomOffset* offset;
    // number of sub components (= numbers of sensors, ladders)
    Int_t nComponents;
    TGeoMedium* tMedium;
    Double_t fMass;
    Double_t fDensityFactor;

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
    //! The color as integer from the ROOT color wheel
    Int_t iColor;
    Bool_t isReflectX;
    Bool_t isReflectY;
    Bool_t isReflectZ;
    //! true = correctDensity() has been already called
    bool isDensityCorrected;
    //! true = getOffset() has been called once
    bool isCalculatedOffset;
    //! true = resetBasicParameters() has been called
    bool isReset;
    //! true = initBasicParameters() has been called
    bool isInitBasicParameters;

    Bool_t correctDensity();
    void resetBasicParameters();
    Bool_t initBasicParameters(GearDir& content);
    Bool_t initOffsets(GearDir& con);
    Bool_t makeGeneric();
  public:
    B2GeomVolume() {resetBasicParameters();}
    TGeoVolume* getVol();
    bool goToParentNode(GearDir& con, bool isShowDebug = false);
    //! returns the position of the volume with relation to the mother volume
    TGeoHMatrix* getPosition();
    //! sets the position of the volume with relation to the mother volume
    void setPosition(TGeoHMatrix* newPosition);
    //! gives the idel position of the volume with relation to its mother volume
    TGeoHMatrix* getIdealPosition();
    //! init this volume with parameters from GearDir
    virtual Bool_t init(GearDir& content, std::string subDir);
    //! init this volume with parameters from GearDir
    virtual Bool_t init(GearDir& content) {return false;}
    //! create TGeoVolume from the parameters
    virtual Bool_t make();
    // returns the offset from the ideal position
    B2GeomOffset* getOffset();
    Double_t getPhiLocal() {return fPhiLocal;}
    Double_t getThetaLocal() {return fThetaLocal;}
    Double_t getPsiLocal() {return fPsiLocal;}
  };
}
#endif
