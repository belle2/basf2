/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <arich/dbobjects/ARICHGeoBase.h>
#include <arich/dbobjects/ARICHPositionElement.h>
#include <string>
#include <Math/Vector3D.h>
#include <Math/Rotation3D.h>

namespace Belle2 {

  /**
   * Geometry parameters of ARICH Master volume (envelope).
   */
  class ARICHGlobalAlignment: public ARICHGeoBase {

  public:

    /**
     * Default constructor.
     */
    ARICHGlobalAlignment()
    {}

    /**
     * Copy constructor.
     */
    ARICHGlobalAlignment(const ARICHGlobalAlignment& align): ARICHGeoBase()
    {
      *this = align;
      m_rotation = nullptr;
      m_rotationInverse = nullptr;
      m_translation = nullptr;
    }

    /**
     * Assignment operator.
     */
    ARICHGlobalAlignment& operator=(const ARICHGlobalAlignment& align)
    {
      if (this != &align) {
        ARICHGeoBase::operator=(align);
        m_alignPars = align.getAlignmentElement();
        if (m_rotation) delete m_rotation;
        if (m_rotationInverse) delete m_rotationInverse;
        if (m_translation) delete m_translation;
        m_rotation = nullptr;
        m_rotationInverse = nullptr;
        m_translation = nullptr;
      }
      return *this;
    }

    /**
     * Destructor.
     */
    ~ARICHGlobalAlignment()
    {
      if (m_rotation) delete m_rotation;
      if (m_rotationInverse) delete m_rotationInverse;
      if (m_translation) delete m_translation;
    }

    /**
     * Sets alignment parameters (element).
     * @param[in] align Alignment element.
     */
    void setAlignmentElement(const ARICHPositionElement& align)
    {
      m_alignPars = align;
    }

    /**
     * Returns alignment parameters (element).
     * @return Alignment element
     */
    const ARICHPositionElement& getAlignmentElement() const
    {
      return m_alignPars;
    }


    /**
     * Print the content of the class.
     * @param[in] title Title to be printed.
     */
    void print(const std::string& title = "ARICH global alignment parameters") const;


    /**
     * Get position of ARICH master volume center point in global
     * Belle II coordinates.
     * @return Center point of ARICH volume.
     */
    const ROOT::Math::XYZVector& getTranslation() const {if (!m_translation) setTransformation(); return *m_translation;}

    /**
     * Get rotation matrix of ARICH master volume in global
     * Belle II coordinates.
     * @return Rotation matrix of ARICH master volume.
     */
    const ROOT::Math::Rotation3D& getRotation() const
    {
      if (!m_rotation) setTransformation();
      return *m_rotation;
    }

    /**
     * Transform local point into global Belle II coordinate system via rotation and translation
     * @param point point to be transformed
     * @return transformed point
     */
    ROOT::Math::XYZVector pointToGlobal(const ROOT::Math::XYZVector& point) const;

    /**
     * Rotate local momentum into global Belle II coordinate system
     * @param momentum momentum vector to be rotated
     * @return rotated momentum vector
     */
    ROOT::Math::XYZVector momentumToGlobal(const ROOT::Math::XYZVector& momentum) const;

    /**
     * Transform global point into ARICH reference system via inverse rotation and translation
     * @param point point to be transformed
     * @return transformed point
     */
    ROOT::Math::XYZVector pointToLocal(const ROOT::Math::XYZVector& point) const;

    /**
     * Rotate global point into ARICH reference system via inverse rotation
     * @param momentum momentum vector to be rotated
     * @return rotated momentum vector
     */
    ROOT::Math::XYZVector momentumToLocal(const ROOT::Math::XYZVector& momentum) const;

  private:

    /**
     * Set rotation matrix and center point of ARICH master volume based on alignment parameters
     */
    void setTransformation() const;

    ARICHPositionElement m_alignPars; /**< alignment parameters */

    mutable ROOT::Math::Rotation3D* m_rotation = nullptr; /**< rotation matrix of ARICH master volume */
    mutable ROOT::Math::Rotation3D* m_rotationInverse = nullptr; /**< inverse rotation matrix of ARICH master volume */
    mutable ROOT::Math::XYZVector*  m_translation = nullptr; /**< position of ARICH master volume center point */

    ClassDef(ARICHGlobalAlignment, 2); /**< ClassDef */

  };

} // end namespace Belle2
