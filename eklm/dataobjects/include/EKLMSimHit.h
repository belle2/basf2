/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMSIMHIT_H
#define EKLMSIMHIT_H


#include <TObject.h>


#include "G4VPhysicalVolume.hh"
#include <eklm/dataobjects/EKLMHitBase.h>
#include <eklm/dataobjects/EKLMStepHit.h>


#include <string>
namespace Belle2 {

  /**
   * Class to handle simulation hits
   */

  class EKLMSimHit : public EKLMHitBase  {

  public:

    /**
     * default constructor needed to make the class storable
     */
    EKLMSimHit();

    /**
     *  Constructor with StepHit
     */
    EKLMSimHit(const EKLMStepHit* stepHit);

    /**
     * Destructor
     */
    ~EKLMSimHit() {};

    /**
     *returns physical volume
     */
    const G4VPhysicalVolume* getVolume()  const;

    /**
     * set physical volume
     */
    void setVolume(const G4VPhysicalVolume*);

    /**
     * returns volume type (needed for background study mode)
     */
    bool  getVolType() const;

    /**
     * sets volume type (needed for background study mode)
     */
    void  setVolType(int);

    /**
     * returns  particle momentum
     */
    const TVector3*   getMomentum() const;

    /**
     * sets  particle momentum
     */
    void  setMomentum(const TVector3& p);

    /**
     * sets  particle momentum
     */
    void  setMomentum(const TVector3* p);


    /**
     * returns  particle energy
     */
    double  getEnergy() const;

    /**
     * sets  particle energy
     */
    void  setEnergy(double);



    //! dumps hit into ASCII file
    void Save(char* filename);


    /**
     * Get plane number.
     */
    int getPlane() const;

    /**
     * Set plane number.
     */
    void setPlane(int Plane);

    /**
     * Get strip number.
     */
    int getStrip() const;

    /**
     * Set strip number.
     */
    void setStrip(int Strip);


    virtual EKLMStripID getID() const;


  private:

    /**
     *particle energy
     */
    double m_energy;

    /**
     * particle momentum
     */
    TVector3 m_momentum;


    /**
     *Physical volume
     */
    const  G4VPhysicalVolume* m_pv; //! {ROOT streamer directive}


    /**
     * Number of plane.
     */
    int m_Plane;

    /**
     * Number of strip.
     */
    int m_Strip;


    /**
     * Name of the volume
     */
    std::string m_pvName;


    /**
     * Volume type (for Background studies)
     */
    G4int m_volType;


    // Needed to make root object storable
    ClassDef(Belle2::EKLMSimHit, 1);

  };


} // end of namespace Belle2

#endif //EKLMSIMHIT_H
