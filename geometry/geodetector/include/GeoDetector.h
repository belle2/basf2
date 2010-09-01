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

#include <framework/gearbox/GearboxIOAbs.h>
#include <geometry/geodetector/CreatorBase.h>
#include <geometry/geodetector/GDetExceptions.h>

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


  protected:


  private:

    std::list<std::string> m_supportedSections; /*!< List of all supported sections in the basic parameter hierarchy. */
    std::list<std::string> m_calledCreators;    /*!< List of all creators which were called during the building process of the detector. */

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
