/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BFIELDMAP_H_
#define BFIELDMAP_H_

#include <TVector3.h>

#include <string>

namespace Belle2 {

  /**
   * The BFieldMap class.
   *
   * This class represents the magnetic field of the Belle II detector.
   * Its main method is getBField() which returns the Bfield vector at the
   * specified space point.
   * It is designed as a singleton.
   */
  class BFieldMap {

  public:

    /**
     * Static method to get a reference to the BFieldMap instance.
     *
     * @return A reference to an instance of this class.
     */
    static BFieldMap& Instance();

    /**
     * Returns the magnetic field of the Belle II detector at the specified space point.
     * The space point is given in Cartesian coordinates (x,y,z) in [cm].
     *
     * @param point The space point in Cartesian coordinates.
     * @return A three vector of the magnetic field in [T] at the specified space point.
     */
    const TVector3 getBField(const TVector3& point) const;


  protected:


  private:

    /** The constructor is hidden to avoid that someone creates an instance of this class. */
    BFieldMap();

    /** Disable/Hide the copy constructor. */
    BFieldMap(const BFieldMap&);

    /** Disable/Hide the copy assignment operator. */
    BFieldMap& operator=(const BFieldMap&);

    /** The destructor of the BFieldMap class. */
    virtual ~BFieldMap();

    static BFieldMap* m_instance; /**< Pointer that saves the instance of this class. */

    /** Destroyer class to delete the instance of the BFieldMap class when the program terminates. */
    class SingletonDestroyer {
    public: ~SingletonDestroyer() {
        if (BFieldMap::m_instance != NULL) delete BFieldMap::m_instance;
      }
    };
    friend class SingletonDestroyer;

  };

} //end of namespace Belle2

#endif /* BFIELDMAP_H_ */
