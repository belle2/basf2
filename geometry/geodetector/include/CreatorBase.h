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

#include <framework/core/FrameworkExceptions.h>
#include <framework/gearbox/GearDir.h>
#include <geometry/geodetector/CreatorManager.h>

#include <TGeoMatrix.h>
#include <TGeoVolume.h>
#include <G4VSensitiveDetector.hh>
#include <TG4RootDetectorConstruction.h>

#include <string>
#include <map>


namespace Belle2 {

  /**
   * The CreatorBase class.
   *
   * Defines the base class for all creators (e.g. geometry, material).
   *
   * A creator implements the source code to create the ROOT objects needed in
   * the geometry library. Examples are TGeo geometry or ROOT materials.
   *
   * A creator is identified by its name. Therefore the name of a creator
   * has to be unique among all creators.
   */
  class CreatorBase {

  public:

    /**
     * Constructor of the CreatorBase class.
     *
     * Throws an exception of type GeometryCreatorNameEmptyError if the name of the creator is empty.
     *
     * @param name The unique name of the Creator.
     */
    CreatorBase(const std::string& name) throw(CreatorManager::GeometryCreatorNameEmptyError);

    /**
     * The destructor of the CreatorBase class.
     */
    virtual ~CreatorBase();

    /**
     * Creates the ROOT Objects.
     *
     * All creators have to implement this method with the ROOT object
     * creation source code.
     *
     * @param content A reference to the content part of the parameter description, which should to be used to create the ROOT objects.
     */
    virtual void create(GearDir& content) = 0;

    /**
     * This method is called after the geometry was converted from ROOT to Geant4.
     *
     * Implement this method to add functionality to the Geant4 native objects like connecting
     * Geant4 objects to the geometry (such as: sensitive detectors, user limits, magnetic field, ...).
     *
     * @param detConstruct This class gives access to the Geant4 native volumes.
     */
    virtual void initializeGeant4(TG4RootDetectorConstruction* detConstruct) {};

    /**
     * Searches for sensitive volumes given by their prefix and assigns the appropriate sensitive detector class.
     *
     * The method searches for all volumes starting with the prefixes given
     * in the sensitive detector map. The found volumes are then connected to
     * their appropriate sensitive detector handling class.
     */
    void assignSensitiveVolumes(TG4RootDetectorConstruction* detConstruct);

    /**
     * Returns the unique name of the creator.
     *
     * @return The unique name of the creator.
     */
    const std::string& getName() const {return m_name; };

    /**
     * Returns the (optional) description of the creator.
     *
     * @return The (optional) description of the creator.
     */
    const std::string& getDescription() const {return m_description; };

    /**
     * Returns the group name of the subdetector in the TGeo hierarchy.
     *
     * @return The group name of the subdetector in the TGeo hierarchy.
     */
    const std::string& getGeoGroupName() const {return m_geoGroupName; };


  protected:

    /**
     * Sets the optional description of the creator.
     *
     * @param description The optional description of the creator.
     */
    void setDescription(const std::string& description);

    /**
     * Adds a new subdetector group to the TGeo hierarchy.
     *
     * @param groupName The name of the subdetector group. Usually the subdetector name.
     * @param groupTrafo A global transformation which should be applied to the whole subdetector.
     * @return Pointer to the created subdetector group, to be used for adding the subdetector volumes to.
     */
    TGeoVolumeAssembly* addSubdetectorGroup(const std::string& groupName, TGeoMatrix* groupTrafo = 0);

    /** Add a new sensitive detector handling class to the geometry.
     *
     * The sensitive detector handling class deals with the output of the Geant4 simulation.
     * In order to automatically assign the sensitive detector classes to the volumes,
     * a naming scheme based on prefixes is used.
     * By using different prefixes, one Creator can assign multiple sensitive detector classes
     * to the volumes.
     *
     * @param prefix The prefix which is used to identify the sensitive volumes.
     * @param sensitiveDetector A pointer to a sensitive detector handling class. The ownership if this class is taken !
     */
    void addSensitiveDetector(const std::string prefix, G4VSensitiveDetector* sensitiveDetector);


  private:

    std::string m_name;          /**< The unique name of the creator. */
    std::string m_description;   /**< A description of the creator (optional). */
    std::string m_geoGroupName;  /**< The group name of the subdetector in the TGeo hierarchy. */
    bool m_hasGroupName;         /**< True if the group name of the subdetector was set. */
    unsigned int m_senDetNumber; /**< The number of sensitive volumes which were found. */

    std::map<std::string, G4VSensitiveDetector*> m_sensitiveDetMap; /**< Maps a prefix to its sensitive detector handling class. */

    /**
     * Loops through the volumes recursively and connects the found sensitive volumes to the sensitive detector classes.
     *
     * @param volume The mother volume who is checked for being a sensitive volume and whose daughters are checked recursively.
     * @param detConstruct The pointer of TG4RootDetectorConstruction in g4root which provides methods to access GEANT4 created objects corresponding to the TGeo ones.
     */
    void assignSensitiveVolumesRec(TGeoVolume* volume, TG4RootDetectorConstruction* detConstruct);
  };

} //end of namespace Belle2

#endif /* CREATORBASE_H_ */
