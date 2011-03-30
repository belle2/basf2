/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Oswald                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef B2GEOMVOLUME_H_
#define B2GEOMVOLUME_H_

#include "TGeoManager.h"
#include "TGeoVolume.h"
#include "TGeoPcon.h"
#include "TGeoMaterial.h"
#include "TGeoMatrix.h"
#include "TMath.h"
#include "TROOT.h"
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <string>

namespace Belle2 {
  class GearDir;

  /** General class for building volumes. */
  class B2GeomVolume {
  protected:
    char name[200]; /** < Name of the volume in the ROOT geometry. */
    char shape[200]; /** < Shape of the volume (at the moment: Box, Trd2, Tube, Assembly possible). */
    TGeoVolume* tVolume; /** The TGeoVolume which is build by this class. */
    TGeoMedium* tMedium; /** < The medium (= Material) of this volume. */
    Double_t fMass; /** < The mass of this volume (necessary for density correction). */
    Double_t fDensityFactor; /** < Ex: If =2 the density of this volume will be doubled. */

    Double_t fLength; /** < Length of this volume. */
    Double_t fWidth; /** < Width of this volume. */
    Double_t fWidth2; /** < Width2 of this volume. */
    Double_t fThickness; /** < Thickness of this volume. */
    Double_t fThickness2; /** < Thickness2 of this volume. */
    Double_t fOuterRadius; /** < Outer Radius of this volume. */
    Double_t fInnerRadius; /** < Inner Radius of this volume. */
    Double_t fWPosition; /** < W (=x) Position of the volume in the coordinate system of the mother volume. */
    Double_t fUPosition; /** < U (=y) Position of the volume in the coordinate system of the mother volume. */
    Double_t fVPosition; /** < V (=z) Position of the volume in the coordinate system of the mother volume. */
    Double_t fPhiLocal; /** < Phi Angle of the volume in the coordinate system of the mother volume (TGeoRotation with Euler angles). */
    Double_t fThetaLocal; /** < Theta Angle of the volume in the coordinate system of the mother volume (TGeoRotation with Euler angles). */
    Double_t fPsiLocal; /** < Psi Angle of the volume in the coordinate system of the mother volume (TGeoRotation with Euler angles). */
    Double_t fInnerRadiusHER; /** First inner radius of this volume. */
    Double_t fOuterRadiusHER; /** First outer radius of this volume. */
    Double_t fInnerRadiusLER; /** Second inner radius of this volume. */
    Double_t fOuterRadiusLER; /** Second outer radius of this volume. */
    Double_t fZ0; /** Position of zeroth section of this volume (position where the volume starts). */
    Double_t fZ1; /** Position of first section of this volume. */
    Double_t fZ2; /** Position of second section of this volume (position where the volume ends). */

    Int_t iColor; /** < The color as integer from the ROOT color wheel. */
    Bool_t isReflectX; /** Reflect the volume about W (=x) axis of the mother volume. */
    Bool_t isReflectY; /** Reflect the volume about U (=y) axis of the mother volume. */
    Bool_t isReflectZ; /** Reflect the volume about V (=z) axis of the mother volume. */
    bool isDensityCorrected; /** < true = correctDensity() has been already called. */

    bool isInitBasicParameters; /** < true = initBasicParameters() has been called. */

    /** Correct density of this volume according to fMass and fDensityFactor */
    Bool_t correctDensity();
    /** Read all variables from the GearBox, if available. */
    Bool_t initBasicParameters(GearDir& content);
    /** Calls init function of given component, relative GearDir path has to be supplied. */
    template <class T>
    Bool_t initComponent(T** component, GearDir& content, std::string relativePath) {
      //B2METHOD();
      // create new object for component
      if ((*component) == NULL)(*component) = new T();
      // check if the given sub directory is available in GearDir path
      if (content.isParamAvailable(relativePath)) {
        // create copy of GearDir and go to corresponding relative path
        GearDir myContent(content);
        myContent.append(relativePath);
        myContent.append("/");

        if (!(*component)->init(myContent)) {
          B2WARNING("Parameter reading for sub component " << relativePath << " of " << std::string(name) << " failed!");
          delete(*component) ;
          (*component)  = NULL;
          return false;
        }
        return true;
      } else {
        delete(*component) ;
        (*component)  = NULL;
        return false;
      }
    }
    /** Search and replace character sequence in component name.
     *  In general this method returns the input string, but it can be overwritten by classes
     *  inheriting from this class (for example B2GeomVXDVolume::searchAndReplaceInName
     *  replaces %Layer by B2GeomVXDVolume::iLayer).
     */
    /** Build this volume according the dimensions and the shape. */
    Bool_t makeGeneric();
    /** Call the make() function of the subcomponent and position it in this volume*/
    Bool_t makeAndAddComponent(B2GeomVolume* component);

  public:
    /** Reset all variables of this class. */
    B2GeomVolume();
    /** Destructor. */
    virtual ~B2GeomVolume() {}
    /** Calls correctDensity() and then returns tVolume. */
    TGeoVolume* getVol();
    /** Goes one node up in GearDir. */
    bool goToParentNode(GearDir& con);
    /** returns the position of the volume with relation to the mother volume. */
    virtual TGeoHMatrix* getPosition();
    /** sets the position of the volume with relation to the mother volume. */
    void setPosition(TGeoHMatrix* newPosition);
    /** init this volume with parameters from GearDir. */
    virtual Bool_t init(GearDir& content) {
      return initBasicParameters(content);
    }
    /** create TGeoVolume from the parameters. */
    virtual Bool_t make();
    /** returns Euler angle Phi in local coordinate system (see ROOT documentation TGeoRotation). */
    Double_t getPhiLocal() {
      return fPhiLocal;
    }
    /** returns Euler angle Theta in local coordinate system (see ROOT documentation TGeoRotation). */
    Double_t getThetaLocal() {
      return fThetaLocal;
    }
    /** returns Euler angle Psi in local coordinate system (see ROOT documentation TGeoRotation). */
    Double_t getPsiLocal() {
      return fPsiLocal;
    }
  };

  /** This is at the moment only used for storing the values of the offsets. */
  class B2GeomOffset {
  public:
    // offsets according to local coordinates of component (in cm, radians)
    Double_t fOffsetW; /** < Offset in W (=x) direction (local coordinates) */
    Double_t fOffsetU; /** < Offset in U (=y) direction (local coordinates) */
    Double_t fOffsetV; /** < Offset in V (=z) direction (local coordinates) */
    Double_t fOffsetPhi; /** < Offset in Phi angle */
    Double_t fOffsetTheta; /** < Offset in Theta angle */
    Double_t fOffsetPsi; /** < Offset in Psi angle */
    B2GeomOffset();
    ~B2GeomOffset();
  };

  /** Special class for VXD components (layer, ladder and sensor number). */
  class B2GeomVXDVolume : public B2GeomVolume {
  protected:
    Int_t iLayer; /** < Layer number. */
    Int_t iLadder; /** < Ladder number. */
    Int_t iSensor; /** < Sensor number. */
  public:
    /** Constructor. */
    B2GeomVXDVolume();
    /** Destructor. */
    virtual ~B2GeomVXDVolume() {}
    /** Searches for %iLayer, %iLadder, %iSensor in string inputString and replaces them by the corresponding number */
    std::string searchAndReplaceInName(std::string inputString);
    /** Set the variables iLayer, iLadder and iSensor */
    void setId(Int_t iLay = -1, Int_t iLad = -1, Int_t iSen = -1);
    /** Calls init function of given component, relative GearDir path has to be supplied.
      * Important remark: only classes derivated from B2GeomVXDVolume are allowed as "T"
      */
    template <class T>
    Bool_t initComponent(T** component, GearDir& content, std::string relativePath) {
      // create new object for component
      if (*component != NULL) B2WARNING("Overwriting object for " << relativePath);
      (*component) = new T();
      (*component)->setId(iLayer, iLadder, iSensor);
      if (!B2GeomVolume::initComponent<T>(component, content, relativePath)) return false;
      (*component)->setId(iLayer, iLadder, iSensor);
      return true;
    }
  };



  /** Class for building the VXD structure including offsets.
   *  This is a template class. ComponentClass specifies the class of the
   *  sub components (for example: (this = Ladder) => (ComponentClass = Sensor).
   *  NB: The 'export' keyword is not yet implemented in most C++ compilers. This
   *  is the reason while the full definition of this class is in the header file!
   *  Moving it to the .cc file will result in a linking error!
   */
  template <class ComponentClass>
  class B2GeomVXDStructVolume : public B2GeomVXDVolume {
  protected:
    ComponentClass** components; /** < The sub components of this volume (ex: this = ladder => components = Sensors). */
    Int_t nComponents; /** < number of sub components (ex numbers of  sensors, ladders). */
    B2GeomOffset* offset; /** < The offset of this component in local coordinates. */
    bool isCalculatedOffset; /** < true = getOffset() has been already called once. */
  public:
    /** Read offsets from GearBox. */
    Bool_t initOffsets(GearDir& content) {
      // read offsets of ideal position, if available
      if (content.isParamAvailable("OffsetW")) offset->fOffsetW = content.getParamLength("OffsetW");
      if (content.isParamAvailable("OffsetU")) offset->fOffsetU = content.getParamLength("OffsetU");
      if (content.isParamAvailable("OffsetV")) offset->fOffsetV = content.getParamLength("OffsetV");
      if (content.isParamAvailable("OffsetPhi")) offset->fOffsetPhi = TMath::RadToDeg() *  content.getParamAngle("OffsetPhi");
      if (content.isParamAvailable("OffsetTheta")) offset->fOffsetTheta = TMath::RadToDeg() * content.getParamAngle("OffsetTheta");
      if (content.isParamAvailable("OffsetPsi")) offset->fOffsetPsi = TMath::RadToDeg() * content.getParamAngle("OffsetPsi");
      return true;
    }

    /** Constructor. */
    B2GeomVXDStructVolume() {
      components = NULL;
      offset = new B2GeomOffset();
      isCalculatedOffset = false;
      nComponents = 0;
    }
    /** Destructor. */
    virtual ~B2GeomVXDStructVolume() { }
    /** returns the position of the volume with relation to the mother volume. */
    TGeoHMatrix* getPosition()  {
      // //B2METHOD();
      if (!isInitBasicParameters) {
        B2ERROR("Please run initBasicParameters() method before any other method!");
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
      return hmaPosition;
    }

    /** Gives the offset from the ideal position.
     * calculate the offset from the offset of the (sub components)
     * offset of this volume = average of offsets of sub components
     */
    B2GeomOffset* getOffset() {
      if (!isCalculatedOffset) {
        // FIRST the average of the LOCAL offsets of the components is calculated in the coordinates of THIS volume
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
        //
        // THEN the average offset is subtracted from each component in its LOCAL coordinates
        //
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
  };
}
#endif
