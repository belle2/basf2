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

#ifndef ECLGEOMETRYMAPPING_H
#define ECLGEOMETRYMAPPING_H

#include <vector>
#include <string>

#include "TVector3.h"


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


    TVector3 GetCrystalPos();

    TVector3 GetCrystalVec();

  private:

    double m_BLThetaCrystal[46];
    double m_BLPhiCrystal[46];
    double m_BLPreppos[46];
    double m_BLPhipos[46];
    double m_BLZpos[46];

    double m_ECThetaCrystal[132];
    double m_ECPhiCrystal[132];
    double m_ECThetapos[132];
    double m_ECPhipos[132];
    double m_ECRpos[132];

    int mPar_cellID;
    int mPar_thetaID;
    int mPar_phiID;
    int mPar_thetaIndex;
    int mPar_phiIndex;

    static ECLGeometryPar* m_B4ECLGeometryParDB; /*!< Pointer that saves the instance of this class. */
  };


} // end of namespace Belle2

#endif
