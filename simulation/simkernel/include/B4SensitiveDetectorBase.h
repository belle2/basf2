/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef B4SENSITIVEDETECTORBASE_H
#define B4SENSITIVEDETECTORBASE_H

#include "G4VSensitiveDetector.hh"
#include "G4String.hh"
#include "G4VHitsCollection.hh"
#include "G4HCofThisEvent.hh"

namespace Belle2 {

//! A base class for sensitive detector
  /*! All sensitive sub-detector class should inherit from this class.
      But, now this class is not used.
  */

  class B4SensitiveDetectorBase : public G4VSensitiveDetector {

  public:

    //! Constructor
    /*! Use this base class to drive your sensitive detector.
        \param name The name of sensitive detector.
    */
    B4SensitiveDetectorBase(const G4String name);

    //! Pure virtal destructor
    virtual ~B4SensitiveDetectorBase() {;};

    //! The method to load event from an ASCII file.
    virtual void LoadEvent(FILE*) {
      G4cout << "Warning, sensitive detector "
      << SensitiveDetectorName
      << " has not yet a LoadEvent(File* ) method" << G4endl;
    }

    //! This pure virtual function should create a hit collection.
    virtual void CreateCollection(void) = 0;

    //! This function clears m_collection and adds Collection to HCTE.
    /*!
        \param HCTE The pointer of hits collection in this event.
    */
    virtual void Initialize(G4HCofThisEvent* HCTE);

    //! Write this function to add the background hits for one bg event.
    /*!
        \param add If background hits should be added.
    */
    virtual void AddbgOne(bool add);

    //! Register a new collection to sensitive detector manager.
    /*!
        \param name The collection name.
        \return This collection id.
    */
    G4int RegisterNewCollection(const G4String &name);

    //! The function to get hits collection.
    /*!
        \param i The index of hits collection.
        \return A pointer of template T, and class T should be one of sub-detector hits collection.
    */
    template <class T>
    T *getHcdc(G4int i = 0) {
      if (i >= 0 && i < GetNumberOfCollections()) {
        return dynamic_cast< T* >(m_collection[i]);
      }
      return NULL;
    }

    //! An overloaded function to get hits collection
    /*!
        \param name The hits collection name.
        \return A pointer of template T, and class T should be one of sub-detector hits collection.
    */
    template <class T>
    T *getHcdc(const G4String & name) {
      for (int i = 0; i < GetNumberOfCollections(); ++i) {
        if (collectionName[i] == name) {
          return dynamic_cast< T* >(m_collection[i]);
        }
      }
      return NULL;
    }

    //! The function to get hits collection, and a constant pointer will be returned.
    /*!
        \param i The index of hits collection.
        \return A constant pointer of template T, and class T should be one of sub-detector hits collection.
    */
    template <class T>
    const T *getHcdc(G4int i = 0) const {
      if (i >= 0 && i < GetNumberOfCollections()) {
        return dynamic_cast< const T* >(m_collection[i]);
      }
      return NULL;
    }

    //! The overloaded function to get hits collection, and a constant pointer will be returned.
    /*!
        \param name The hits collection name.
        \return A constant pointer of template T, and class T should be one of sub-detector hits collection.
    */
    template <class T>
    const T *getHcdc(const G4String & name) const {
      for (int i = 0; i < GetNumberOfCollections(); ++i) {
        if (collectionName[i] == name) {
          return dynamic_cast< const T* >(m_collection[i]);
        }
      }
      return NULL;
    }

  protected:

    std::vector<G4int> m_hcid;                      /*!< The vector to store hits collection ids. */
    std::vector<G4VHitsCollection*> m_collection;   /*!< The vector to store hits collection pointers. */

  private:

  };

} // end namespace Belle2

#endif /* B4SENSITIVEDETECTORBASE_H */
