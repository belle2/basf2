/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <arich/dbobjects/ARICHGeoBase.h>
#include <arich/dbobjects/ARICHAlignmentElement.h>
#include <string>
#include <TVector3.h>
#include <TVector2.h>
#include <TRotation.h>

namespace Belle2 {


  /**
   * Geometry parameters of ARICH Master volume (envelope)
   */


  class ARICHGlobalAlignment: public ARICHGeoBase {

  public:

    /**
     * Default constructor
     */
    ARICHGlobalAlignment()
    {}

    /**
     * Copy constructor
     */
    ARICHGlobalAlignment(const ARICHGlobalAlignment& align): ARICHGeoBase()
    {
      *this = align;
      m_rotation = 0;
      m_rotationInverse = 0;
      m_translation = 0;
    }


    /**
     * Assignment operator
     */
    ARICHGlobalAlignment& operator=(const ARICHGlobalAlignment& align)
    {
      if (this != &align) {
        ARICHGeoBase::operator=(align);
        m_alignPars = align.getAlignmentElement();
        if (m_rotation) delete m_rotation;
        if (m_rotationInverse) delete m_rotationInverse;
        if (m_translation) delete m_translation;
        m_rotation = 0;
        m_rotationInverse = 0;
        m_translation = 0;
      }
      return *this;
    }

    /**
     * Destructor
     */
    ~ARICHGlobalAlignment()
    {
      if (m_rotation) delete m_rotation;
      if (m_rotationInverse) delete m_rotationInverse;
      if (m_translation) delete m_translation;
    }

    /**
     * Sets alignment parameters (element)
     * @param align alignment element
     */
    void setAlignmentElement(const ARICHAlignmentElement& align)
    {
      m_alignPars = align;
    }

    /**
     * Returns alignment parameters (element)
     * @return alignment element
     */
    const ARICHAlignmentElement& getAlignmentElement() const
    {
      return m_alignPars;
    }


    /**
     * Print the content of the class
     * @param title title to be printed
     */
    void print(const std::string& title = "ARICH global alignment parameters") const;


    /**
     * Get position of ARICH master volume center point in global Belle II coordinates
     * @return center point of ARICH volume
     */
    const TVector3& getTranslation() const {if (!m_translation) setTransformation(); return *m_translation;}

    /**
     * Get rotation matrix of ARICH master volume in global Belle II coordinates
     * @return rotation matrix of ARICH master volume
     */
    const TRotation& getRotation() const
    {
      if (!m_rotation) setTransformation();
      return *m_rotation;
    }

    TVector3 pointToGlobal(const TVector3& point) const;
    TVector3 momentumToGlobal(const TVector3& momentum) const;
    TVector3 pointToLocal(const TVector3& point) const;
    TVector3 momentumToLocal(const TVector3& momentum) const;


  private:

    void setTransformation() const;

    ARICHAlignmentElement m_alignPars;

    mutable TRotation* m_rotation = 0 ;
    mutable TRotation* m_rotationInverse = 0;
    mutable TVector3*  m_translation = 0;

    ClassDef(ARICHGlobalAlignment, 1); /**< ClassDef */

  };

} // end namespace Belle2
