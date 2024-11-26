/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef ARICHTHPARAM_H
#define ARICHTHPARAM_H

// ROOT
#include <TObject.h>

namespace Belle2 {


  //! ARICHThParam class for storing photon hit information.
  /*! This class holds the ARICH hit information after digitization (obtained from ARICHSimHit after ARICHThParamizer module). Contains only module number, channel number and hit bitmap (4 bits).
  */

  class ARICHThParam : public TObject {

  public:

    //! Default constructor for ROOT IO.
    ARICHThParam() : m_index(-1), m_dth(-1), m_th0(0), m_nth(0)
    {
      /*! does nothing */
    }

    //! Constructor
    ARICHThParam(int index, double dth, double th0, int nth)
    {
      m_index = index;
      m_dth = dth;
      m_th0 = th0;
      m_nth = nth;
    }

    /**
     * Set ARICH hit information
     */
    void set(int index, double dth, double th0, int nth)
    {
      m_index = index;
      m_dth = dth;
      m_th0 = th0;
      m_nth = nth;
    }

    //! Destructor
    ~ARICHThParam()
    {
      /*! Does nothing */
    }

    /**
     * Get index
     */
    int getIndex() const { return m_index; }

    /**
     * Get Dth
     */
    double getDth() const { return m_dth; }

    /**
     * Get Th0
     */
    double getTh0() const { return m_th0; }

    /**
     * Get Vth
     */
    double getVth() const;

    /**
     * Get Nth
     */
    int getNth() const { return m_nth; }

    /**
     * Get value
     */
    int getVal() const;

  private:

    int m_index; /**< index */
    double m_dth; /**< dth */
    double m_th0; /**< th0 */
    int m_nth; /**< nth */
    ClassDef(ARICHThParam, 1); /**< the class title */

  };

} // end namespace Belle2

#endif
