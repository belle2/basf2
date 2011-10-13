/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 *                                                                        *
 * 7/31: Poyuan                                                           *
 *                                                                        *
 *                                                                        *
 **************************************************************************/
#ifndef ECLGEOMETRYPAR_H
#define ECLGEOMETRYPAR_H

#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <functional>
#include <map>


#include "TVector3.h"
typedef int EclIdentifier ;
typedef double     EclGeV        ;
///
typedef double   EclCM         ;
///
typedef double     EclCC         ;
///
typedef double     EclKG         ;
///
typedef double     EclRad        ;
///
typedef double     EclDeg        ;


namespace Belle2 {

//! The Class for ECL Geometry Parameters
  /*! This class provides ECL gemetry paramters for simulation, reconstruction and so on.
      These parameters are gotten from gearbox.
  */
  class ECLGeometryPar {

  public:

    //! Constructor
    ECLGeometryPar();

    //! Destructor
    virtual ~ECLGeometryPar();

    //! Static method to get a reference to the ECLGeometryPar instance.
    /*!
        \return A reference to an instance of this class.
    */
    static ECLGeometryPar* Instance();

    //! Clears
    void clear();

    //! Print some debug information
    void Print() const;

    //! Gets geometry parameters from gearbox.
    void read();

    //! Mapping theta, phi Id
    void Mapping(int cid);

    //! Get Crystal Position
    TVector3 GetCrystalPos(int cid);  /** The Postion of crystal*/

    TVector3 GetCrystalVec(int cid);  /** The dection of crystal*/

    int GetCellID(int ThetaId, int PhiId);//! Get Cell Id

    int GetCellID() {return mPar_cellID;};//! Get Cell Id
    int GetThetaID() {return mPar_thetaID;};//! Get Theta Id
    int GetPhiID() {return mPar_phiID;};//! Get Phi Id

  private:

    double m_BLThetaCrystal[46];  /** The Theta angle of  crystal derection for Barrel crystals */
    double m_BLPhiCrystal[46];    /** The Phi angle of  crystal derection  for Barrel crystals */
    double m_BLPreppos[46];       /** The Prep of  crystal derection  for Barrel crystals */
    double m_BLPhipos[46];        /** The Theta angle of  crystal position  for Barrel crystals */
    double m_BLZpos[46];          /** The Z of  crystal position  for Barrel crystals */

    double m_ECThetaCrystal[132]; /** The Theta angle of  crystal derection  for Endcap crystals */
    double m_ECPhiCrystal[132];   /** The Phi angle of  crystal derection  for Endcap crystals */
    double m_ECThetapos[132];     /** The Theta angle of  crystal position  for Endcap crystals */
    double m_ECPhipos[132];       /** The Phi angle of  crystal position  for Endcap crystals */
    double m_ECRpos[132];         /** The R angle of  crystal position  for Endcap crystals */

    int mPar_cellID;           /** The Cell ID information*/
    int mPar_thetaID;          /** The Theta ID information*/
    int mPar_phiID;            /** The Phi ID information*/
    int mPar_thetaIndex;       /** The Theta Index information*/
    int mPar_phiIndex;         /** The Phi IndeX information*/

    static ECLGeometryPar* m_B4ECLGeometryParDB; /*!< Pointer that saves the instance of this class. */
  };
} // end of namespace Belle2

#endif
