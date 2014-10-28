/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *               Alexandre Beaulieu                                       *
 * This software is provided "as is" without any warranty.                *
 *                                                                        *
 * 7/31: Poyuan                                                           *
 * 2014/10/24: ABeaulieu: adapted from the ECL package to BEAST CsI       *
 *                                                                        *
 **************************************************************************/
#ifndef CSIGEOMETRYPAR_H
#define CSIGEOMETRYPAR_H

#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <functional>
#include <map>
#include <G4String.hh>

#include "TVector3.h"
/** tyep define CsiIdentifier */
typedef int CsiIdentifier ;
/** tyep define CsiGeV */
typedef double     CsiGeV        ;
/** tyep define CsiGeV */
typedef double   CsiCM         ;
/** tyep define CsiCC */
typedef double     CsiCC         ;
/** tyep define CsiKG */
typedef double     CsiKG         ;
/** tyep define CsiRad */
typedef double     CsiRad        ;
/** tyep define CsiDeg */
typedef double     CsiDeg        ;


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

      //! Print some debug information
      void Print() const;

      //! Gets geometry parameters from gearbox.
      void read();

      /** Get Cell Id */
      int CsiVolNameToCellID(const G4String VolumeName);//Mapping from VolumeName to Crystal CellID

    private:
      /** The Theta angle of  crystal derection  for Endcap crystals */
      double m_ECThetaCrystal[132];
      /** The Phi angle of  crystal derection  for Endcap crystals */
      double m_ECPhiCrystal[132];
      /** The Theta angle of  crystal position  for Endcap crystals */
      double m_ECThetapos[132];
      /** The Phi angle of  crystal position  for Endcap crystals */
      double m_ECPhipos[132];
      /** The R angle of  crystal position  for Endcap crystals */
      double m_ECRpos[132];
      /** The Cell ID information*/
      int mPar_cellID;
      /** The Theta ID information*/
      int mPar_thetaID;
      /** The Phi ID information*/
      int mPar_phiID;
      /** The Theta Index information*/
      int mPar_thetaIndex;
      /** The Phi IndeX information*/
      int mPar_phiIndex;

      /** Pointer that saves the instance of this class. */
      static CsiGeometryPar* m_B4CsiGeometryParDB;
    };

  } // end of namespace csi
} // end of namespace Belle2


#endif
