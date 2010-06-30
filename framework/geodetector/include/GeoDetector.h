/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEODETECTOR_H_
#define GEODETECTOR_H_

#include <gearbox/GearboxIOAbs.h>
#include <geodetector/SensitiveDetDBItem.h>
#include <geodetector/CreatorBase.h>
#include <geodetector/GDetExceptions.h>

#include <TGeoVolume.h>

#include <list>
#include <map>
#include <string>


namespace Belle2 {

  //!  The GeoDetector class.
  /*!
      This class represents the detector geometry. It knows about the basic structure
      of the parameter hierarchy (Global,MaterialSets,Subdetectors sections),
      the used creators to build the geometry and the sensitive volumes.
      It is designed as a singleton.
  */
  class GeoDetector {

  public:

    //! Static method to get a reference to the GeoDetector instance.
    /*!
      \return A reference to an instance of this class.
    */
    static GeoDetector& Instance();

    //! Creates a ROOT representation of the detector.
    void createDetector();

    //! Saves the created ROOT representation of the detector to the file given by the filename.
    /*!
      \param filename The filename and path of the ROOT file in which the detector should be saved.
    */
    void saveToRootFile(const std::string& filename);

    //! Returns a list of all creators which were called during the building process of the detector.
    /*!
      \return A list of the creator names  which were called during the building process of the detector.
    */
    const std::list<std::string>& getCalledCreators() const;

    //! Returns a list of the sensitive volumes found for the specified creatorName.
    /*!
      \param creatorName The name of the creator whose sensitive volumes should be returned.
      \return A list of the sensitive volumes found for the specified creatorName.
    */
    const std::list<TGeoVolume*>& getSensitiveVolumes(const std::string& creatorName) const
    throw(GDetExcCreatorNameEmpty, GDetExcCreatorNotExists);


  protected:


  private:

    std::list<std::string> m_supportedSections; /*!< List of all supported sections in the basic parameter hierarchy. */
    std::list<std::string> m_calledCreators;    /*!< List of all creators which were called during the building process of the detector. */

    std::map<std::string, SensitiveDetDBItem*> m_sensitiveDetDB; /*!< Maps the name of a Creator to the sensitive detectors of this subdetector. */
    int m_senDetNumber; /*!< The number of found sensitive volumes per subdetector. */

    //! Searches for sensitive volumes for the given creator carrying a name with the prefix given by the creator.
    /*!
      \param creator The creator for which the sensitive volumes should be searched for.
    */
    void findSensitiveDetectorByPrefix(CreatorBase& creator);

    //! Goes through the volumes recursively and adds the found sensitive volumes to the sensitive volume database.
    /*!
      \param volume The mother volume who is checked for being a sen. volume and whose daughters are checked recursively.
      \param dbItem The entry in the sensitive detector database to which the found sensitive volumes should be added.
      \param prefix The prefix which identifies sensitive volumes.
    */
    void setSensitiveDetectorRec(TGeoVolume* volume, SensitiveDetDBItem& dbItem, const std::string& prefix);

    //! Clears the sensitive detector database by freeing the memory.
    void clearSensitiveDetDB();

    //! The constructor is hidden to avoid that someone creates an instance of this class.
    GeoDetector();

    //! Disable/Hide the copy constructor
    GeoDetector(const GeoDetector&);

    //! Disable/Hide the copy assignment operator
    GeoDetector& operator=(const GeoDetector&);

    //! The destructor of the GeoDetector class.
    virtual ~GeoDetector();

    static GeoDetector* m_instance; /*!< Pointer that saves the instance of this class. */

    //! Destroyer class to delete the instance of the GeoDetector class when the program terminates.
    class SingletonDestroyer {
    public: ~SingletonDestroyer() {
        if (GeoDetector::m_instance != NULL) delete GeoDetector::m_instance;
      }
    };
    friend class SingletonDestroyer;

  };

}

#endif /* GEODETECTOR_H_ */
