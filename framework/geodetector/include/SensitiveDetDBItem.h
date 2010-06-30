/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SENSITIVEDETDBITEM_H_
#define SENSITIVEDETDBITEM_H_

#include <TGeoVolume.h>

#include <string>
#include <list>

namespace Belle2 {

  //! The SensitiveDetDBItem class.
  /*!
    Defines a single item in the sensitive detector
    database. The geometry library is able to manage
    the sensitive volumes of a subdetector, which
    are described by volumes whose name starts with
    a prefix given by each Creator.
  */
  class SensitiveDetDBItem {

  public:

    //! Constructor of the SensitiveDetDBItem class.
    /*!
        \param geoPathName The name of the subdetector group in the TGeo hierarchy.
    */
    SensitiveDetDBItem(const std::string& geoPathName);

    //! The destructor of the SensitiveDetDBItem class.
    virtual ~SensitiveDetDBItem();

    //! Adds a new sensitive volume to the list.
    /*!
        \param volume Pointer to the sensitive volume which should be added to the list.
    */
    void addSensitiveVolume(TGeoVolume* volume);

    //! Returns the name of the subdetector group in the TGeo hierarchy.
    /*!
        \return The name of the subdetector group in the TGeo hierarchy.
    */
    const std::string& getGeoPathName() const { return m_geoPathName; };

    //! Returns the list of sensitive volumes of the subdetector.
    /*!
        \return The list of sensitive volumes of the subdetector.
    */
    const std::list<TGeoVolume*>& getSensitiveVolumes() const { return m_volumeList; };


  protected:


  private:

    std::string m_geoPathName;           /*!< The name of the subdetector group in the TGeo hierarchy. */
    std::list<TGeoVolume*> m_volumeList; /*!< The list of pointers to all sensitive volumes in this detector. */

  };

} //end of namespace Belle2

#endif /* SENSITIVEDETDBITEM_H_ */
