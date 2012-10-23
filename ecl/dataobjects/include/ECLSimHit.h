/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLSIMHIT_H
#define ECLSIMHIT_H

#include <framework/datastore/DataStore.h>
#include <generators/dataobjects/SimHitBase.h>

#include <TObject.h>
#include <TVector3.h>
namespace Belle2 {

  /**
    * ClassSVDSimHit - Geant4 simulated hit for the ECL.
    *
    * This class holds particle hit data from geant4 simulation. As the simulated
    * hit classes are used to generate detector response, they contain _local_
    * information.
    */
  class ECLSimHit : public SimHitBase {


//namespace Belle2 {

    //! Example Detector
    //class ECLSimHit : public TObject {
  public:


    //! The cell id of this hit.
    int m_cellId;

    //! Deposited energy of this hit.
    double m_edep;

    //! Hit average time
    double m_TimeAve;

    //! The method to set cell id
    void setCellId(int cellId) { m_cellId = cellId; }

    //! The method to set deposited energy
    void setEnergyDep(double edep) { m_edep = edep; }

    //! The method to set hit average time
    void setTimeAve(double TimeAve) { m_TimeAve = TimeAve; }

    //! The method to get cell id
    int getCellId() const { return m_cellId; }

    //! The method to get deposited energy
    double getEnergyDep() const { return m_edep; }

    //! The method to get hit average time
    double getTimeAve() const {return m_TimeAve;}




    //! Empty constructor
    /*! Recommended for ROOT IO
    */
    ECLSimHit() {;}
    /** ROOT Macro.*/
    ClassDef(ECLSimHit, 2);

  };

} // end namespace Belle2

#endif
