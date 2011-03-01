/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef OPTICALUSERINFO_H_
#define OPTICALUSERINFO_H_

#include <geometry/dataobjects/VolumeUserInfoBase.h>

#include <globals.hh>
#include <G4OpticalSurface.hh>
#include <G4SurfaceProperty.hh>

#include <map>
#include <string>


namespace Belle2 {

  /**
    * The OpticalUserInfo class.
    *
    * This class implements the optical surface user information. In order to
    * make a TGeoVolume available for optical surface effects, attach this class
    * to the TGeoVolume using its SetField() method.
    */
  class OpticalUserInfo : public VolumeUserInfoBase {

  public:

    typedef std::map<std::string, G4SurfaceType> SurfaceTypeNameMap;
    typedef std::map<std::string, G4OpticalSurfaceFinish> SurfaceFinishNameMap;
    typedef std::map<std::string, G4OpticalSurfaceModel> SurfaceModelNameMap;

    /** Default constructor for ROOT. */
    OpticalUserInfo():
        m_surfaceType(dielectric_metal), m_surfaceFinish(ground),
        m_surfaceModel(glisur) { fillEnumMaps(); }

    /** Copy constructor. */
    OpticalUserInfo(const OpticalUserInfo& opUInfo):
        m_surfaceType(opUInfo.m_surfaceType), m_surfaceFinish(opUInfo.m_surfaceFinish),
        m_surfaceModel(opUInfo.m_surfaceModel) { fillEnumMaps(); }

    /** Destructor */
    ~OpticalUserInfo() { }

    /** Assignment operator.*/
    OpticalUserInfo& operator=(const OpticalUserInfo& other);

    /** Sets the surface type of the optical surface.
     * @param surfaceType The type of the optical surface.
     */
    void setSurfaceType(const G4SurfaceType& surfaceType) { m_surfaceType = surfaceType; }

    /** Sets the surface type of the optical surface by a string identifier.
     * @param surfaceType The string representation of the type of the optical surface.
     */
    void setSurfaceType(const std::string& surfaceType);

    /** Sets the surface finish of the optical surface.
     * @param surfaceFinish The finish of the optical surface.
     */
    void setSurfaceFinish(const G4OpticalSurfaceFinish& surfaceFinish) { m_surfaceFinish = surfaceFinish; }

    /** Sets the surface finish of the optical surface by a string identifier.
     * @param surfaceFinish The string representation of the finish of the optical surface.
     */
    void setSurfaceFinish(const std::string& surfaceFinish);

    /** Sets the surface model of the optical surface.
     * @param surfaceModel The model of the optical surface.
     */
    void setSurfaceModel(const G4OpticalSurfaceModel& surfaceModel) { m_surfaceModel = surfaceModel; }

    /** Sets the surface model of the optical surface by a string identifier.
     * @param surfaceFinish The string representation of the model of the optical surface.
     */
    void setSurfaceModel(const std::string& surfaceModel);

    /** Returns the optical surface type.
     * @return The type of the optical surface.
     */
    const G4SurfaceType getSurfaceType() const { return m_surfaceType; }

    /** Returns the optical surface finish.
     * @return The finish of the optical surface.
     */
    const G4OpticalSurfaceFinish getSurfaceFinish() const { return m_surfaceFinish; }

    /** Returns the optical surface model.
     * @return The model of the optical surface.
     */
    const G4OpticalSurfaceModel getSurfaceModel() const { return m_surfaceModel; }


  protected:

    G4SurfaceType m_surfaceType;            /**< The type of the optical surface. */
    G4OpticalSurfaceFinish m_surfaceFinish; /**< The surface finish of the optical surface. */
    G4OpticalSurfaceModel m_surfaceModel;   /**< The model of the optical surface. */

    SurfaceTypeNameMap   m_surfaceTypeNameMap;   //! Maps the name of a surface type to its enumeration value.
    SurfaceFinishNameMap m_surfaceFinishNameMap; //! Maps the name of a surface finish to its enumeration value.
    SurfaceModelNameMap  m_surfaceModelNameMap;  //! Maps the name of a surface model to its enumeration value.

  private:

    /**
     * Fills the internal maps which connect the string representation of an enum with its enum value.
     */
    void fillEnumMaps();

    /** Class definition required for the creation of the ROOT dictionary. */
    ClassDef(OpticalUserInfo, 1)
  };

} // end namespace Belle2

#endif /* OPTICALUSERINFO_H_ */
