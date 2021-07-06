/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef CSIGEOMETRYPAR_H
#define CSIGEOMETRYPAR_H

#include <vector>
#include <G4String.hh>
#include <G4Material.hh>

#include <CLHEP/Geometry/Transform3D.h>
#include <CLHEP/Vector/ThreeVector.h>
#include <TVector3.h>

/** type define CsiIdentifier */
typedef int CsiIdentifier ;

typedef CLHEP::HepRotation RotationMatrix;
typedef CLHEP::Hep3Vector ThreeVector;

typedef HepGeom::Transform3D Transform3D;
//typedef HepGeom::Rotate3D  Rotate3D;
//typedef HepGeom::RotateX3D RotateX3D;
typedef HepGeom::RotateY3D RotateY3D;
typedef HepGeom::RotateZ3D RotateZ3D;

typedef HepGeom::Translate3D Translate3D;
//typedef HepGeom::TranslateX3D TranslateX3D;
//typedef HepGeom::TranslateY3D TranslateY3D;
//typedef HepGeom::TranslateZ3D TranslateZ3D;


namespace Belle2 {
  namespace csi {

//! The Class for CSI Geometry Parameters
    /*! This class provides CSI gemetry paramters for simulation, reconstruction and so on.
        These parameters are gotten from gearbox.
    */
    class CsiGeometryPar {

    public:

      //! Constructor
      CsiGeometryPar();

      //! Destructor
      virtual ~CsiGeometryPar();

      //! Static method to get a reference to the CsiGeometryPar instance.
      /*!
          \return A reference to an instance of this class.
      */
      static CsiGeometryPar* Instance();

      //! Clears
      void clear();

      //! Print crystal information
      void Print(const int cid, int debuglevel = 80);

      //! Print all crystals information
      void PrintAll(int debuglevel = 80);


      //! Gets geometry parameters from gearbox.
      void read();

      /** Get Cell Id */
      int CsiVolNameToCellID(const G4String VolumeName);//Mapping from VolumeName to Crystal CellID

      /** Get pointer to the Geant4 Material */
      G4Material* GetMaterial(int cid);

      /** Get the position of the crystal*/
      ThreeVector GetPosition(int cid) { return m_Position.at(cid); };

      /** Get the position of the crystal*/
      ThreeVector GetOrientation(int cid) { return m_Orientation.at(cid); };

      /** Get the position of the crystal in a root TVector3*/
      TVector3 GetPositionTV3(int cid) { return m_PositionTV3.at(cid); };

      /** Get the orientation of the crystal in a root TVector3*/
      TVector3 GetOrientationTV3(int cid) { return m_OrientationTV3.at(cid); };

      /** Converts to a ROOT TVector3 */
      TVector3 ConvertToTVector3(ThreeVector _hepTV)
      {
        TVector3 pos(_hepTV.x(), _hepTV.y(), _hepTV.z());
        return pos;
      };

      /** Get Enclosure ID from cell ID*/
      int GetEnclosureID(int cid) { return m_BoxID.at(cid); };

      /** Get Slot ID in the Enclosure from cell ID*/
      int GetSlotID(int cid) { return m_SlotID.at(cid); };

      /** Get material property*/
      double GetMaterialProperty(int cid, const char* propertyname);

      /** Get crystal fast time constant*/
      double GetTauFast(int cid) { return GetMaterialProperty(cid, "FASTTIMECONSTANT"); };

      /** Get crystal slow time constant*/
      double GetTauSlow(int cid) { return GetMaterialProperty(cid, "SLOWTIMECONSTANT"); };


    private:


      /** The Cell ID information*/
      int m_cellID;

      /** The Theta ID information*/
      std::vector<int> m_thetaID;

      /** The Phi ID information*/
      std::vector<int> m_phiID;

      /** The index of the enclosure*/
      std::vector<int> m_BoxID;

      /** The slot index of the crystal in the enclosure*/
      std::vector<int> m_SlotID;

      /** Position of the nominal centre of the crystal (as a HEP ThreeVector)*/
      std::vector<ThreeVector> m_Position;
      /** Position of the nominal centre of the crystal (as a ROOT TVector3)*/
      std::vector<TVector3> m_PositionTV3;

      /** Orientation of the crystal (as a HEP ThreeVector)*/
      std::vector<ThreeVector> m_Orientation;
      /** Orientation of the crystal (as ROOT TVector3)*/
      std::vector<TVector3> m_OrientationTV3;

      /** Pointer that saves the instance of this class. */
      static CsiGeometryPar* m_B4CsiGeometryParDB;
    };

  } // end of namespace csi
} // end of namespace Belle2


#endif
