/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CREATORBASE_H_
#define CREATORBASE_H_

#include <framework/geodetector/GDetExceptions.h>
#include <framework/gearbox/GearDir.h>

#include <TGeoMatrix.h>
#include <TGeoVolume.h>

#include <string>


namespace Belle2 {

  //!  The CreatorBase class.
  /*!
    Defines the base class for all creators (e.g. geometry, material).

    A creator implements the source code to create the ROOT objects needed in
    the geometry library. Examples are TGeo geometry or ROOT materials.

    A creator is identified by its name. Therefore the name of a creator
    has to be unique among all creators.
  */
  class CreatorBase {

  public:

    //! Constructor of the CreatorBase class.
    /*!
      Throws an exception of type GDetExcCreatorNameEmpty if the name of the creator is empty.
      \param name The unique name of the Creator.
    */
    CreatorBase(const std::string& name) throw(GDetExcCreatorNameEmpty);

    //! The destructor of the CreatorBase class.
    virtual ~CreatorBase();

    //! Creates the ROOT Objects.
    /*!
      All creators have to implement this method with the ROOT object
      creation source code.
      \param content A reference to the content part of the parameter description, which should to be used to create the ROOT objects.
    */
    virtual void create(GearDir& content) = 0;

    //! Returns the unique name of the creator.
    /*!
      \return The unique name of the creator.
    */
    const std::string& getName() const {return m_name; };

    //! Returns the (optional) description of the creator.
    /*!
      \return The (optional) description of the creator.
    */
    const std::string& getDescription() const {return m_description; };

    //! Returns the group name of the subdetector in the TGeo hierarchy.
    /*!
      \return The group name of the subdetector in the TGeo hierarchy.
    */
    const std::string& getGeoGroupName() const {return m_geoGroupName; };

    //! Returns if this Creator creates sensitive volumes with a name prefix.
    /*!
      \return True if this creator creates sensitive volumes with a name prefix.
    */
    bool hasAutoSensitiveVolumes() const { return m_hasSensitiveVolumes; };

    //! Returns the name prefix for sensitive volumes.
    /*!
      \return TThe name prefix for sensitive volumes.
    */
    const std::string& getAutoSenVolPrefix() const { return m_senVolPrefix; };


  protected:

    //! Sets the optional description of the creator.
    /*!
      \param description The optional description of the creator.
    */
    void setDescription(const std::string& description);

    //! Activate the auto translation of sensitive volumes according to their name prefix.
    /*!
      \param prefix The prefix which is used to identify sensitive volumes.
    */
    void activateAutoSensitiveVolumes(const std::string prefix = "SD_");

    //! Adds a new subdetector group to the TGeo hierarchy.
    /*!
      \param groupName The name of the subdetector group. Usually the subdetector name.
      \param groupTrafo A global transformation which should be applied to the whole subdetector.
      \return Pointer to the created subdetector group, to be used for adding the subdetector volumes to.
    */
    TGeoVolumeAssembly* addSubdetectorGroup(const std::string& groupName, TGeoMatrix* groupTrafo = 0);


  private:

    std::string m_name;         /*!< The unique name of the creator. */
    std::string m_description;  /*!< A description of the creator (optional). */
    std::string m_geoGroupName; /*!< The group name of the subdetector in the TGeo hierarchy. */
    bool m_hasGroupName;        /*!< True if the group name of the subdetector was set. */

    bool m_hasSensitiveVolumes; /*!< If set to true this Creator creates sensitive volumes having a "SD_" as name prefix. Default is false. */
    std::string m_senVolPrefix; /*!< The prefix of the sensitive volumes. Default is "SD_". */

  };

} //end of namespace Belle2

#endif /* CREATORBASE_H_ */
