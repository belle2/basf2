/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef ARICHHIT_H
#define ARICHHIT_H

#include <framework/datastore/RelationsObject.h>
#include <Math/Vector3D.h>

namespace Belle2 {


  //! Datastore class that holds photon hits. Input to the reconstruction.
  class ARICHHit : public RelationsObject {
  public:

    //! Empty constructor
    /*! Recommended for ROOT IO */
    ARICHHit():
      m_x(0.0),
      m_y(0.0),
      m_z(0.0),
      m_mod(0),
      m_ch(0)
    {
      /*! does nothing */
    }

    //! Useful Constructor
    /*!
      \param position vector of track position on aerogel plane
    */
    ARICHHit(ROOT::Math::XYZVector position, int mod, int ch)
    {
      m_x = (float) position.X();
      m_y = (float) position.Y();
      m_z = (float) position.Z();
      m_mod = mod;
      m_ch = ch;
    }

    //! Get photon hit position
    ROOT::Math::XYZVector getPosition() const { return ROOT::Math::XYZVector(m_x, m_y, m_z); }

    //! Get channel ID
    int getChannel() const {return m_ch;}

    //! Get module ID
    int getModule() const {return m_mod;}

  private:

    float  m_x;        /**< hit x position */
    float  m_y;        /**< hit y position */
    float  m_z;        /**< hit z position */
    int m_mod;           /**< hit module ID */
    int m_ch;            /**< hit channels ID */


    ClassDef(ARICHHit, 1); /**< the class title */

  };

} // end namespace Belle2

#endif  // ARICHHIT_H
