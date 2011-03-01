/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef VOLUMEUSERINFOBASE_H_
#define VOLUMEUSERINFOBASE_H_

#include <geometry/dataobjects/MaterialPropertyList.h>

#include <G4LogicalVolume.hh>
#include <TObject.h>

namespace Belle2 {

  /**
    * VolumeUserInfo base class.
    *
    * The base class for all classes, which are attached to TGeoVolumes and carry user information.
    * It stores information about the material property table of the TGeoVolume and the Geant4 step size.
    */
  class VolumeUserInfoBase : public TObject {

  public:

    /** Default constructor for ROOT. */
    VolumeUserInfoBase():
        m_stepSize(-1.0) {}

    /** Destructor */
    ~VolumeUserInfoBase() {}

    /** Sets the Geant4 step size for the volume in [cm].
     * @param stepSize The step size for Geant4 in [cm].
     */
    void setStepSize(double stepSize) { m_stepSize = stepSize; }

    /** Returns the Geant4 step size for the volume in [cm].
     * @return The step size for Geant4 [cm].
     */
    double getStepSize() const { return m_stepSize; }

    /** This method is called by the post TGeo to Geant4 conversion step.
     * It allows the developer to set the Geant4 specific settings of the user information.
     * Please note: Make sure to call the updateG4Volume() method of the base class of your inherited class !!!
     * @param g4Volume Pointer to the Geant4 volume.
     */
    virtual void updateG4Volume(G4LogicalVolume* g4Volume);


  protected:

    double m_stepSize; /**< The Geant4 step size [cm] which was set for the volume. A negative value will be used to tell Geant4 to automatically choose the step size.*/


  private:

    ClassDef(VolumeUserInfoBase, 1)

  };

} // end namespace Belle2

#endif /* VOLUMEUSERINFOBASE_H_ */
