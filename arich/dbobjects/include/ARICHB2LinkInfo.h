/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <TObject.h>

namespace Belle2 {

  /**
   * List of Belle2Link boards.
   */
  class ARICHB2LinkInfo: public TObject {

  public:

    /**
     * Default constructor.
     */
    ARICHB2LinkInfo(): m_id(0), m_address(0), m_port(0) {};

    /**
     * Constructor.
     */
    ARICHB2LinkInfo(int id, int address, int port): m_id(id), m_address(address), m_port(port) {};

    /**
     * Destructor.
     */
    ~ARICHB2LinkInfo() {};

    /**
     * Get Belle2Link identifier.
     * @return B2Link identifier.
     */
    int getB2LinkID() const {return m_id; }

    /**
     * Set Belle2Link identifier.
     * @param[in] id B2Link identifier.
     */
    void setB2LinkID(int id) {m_id = id; }

    /**
     * Get Belle2Link address.
     * @return B2Link address.
     */
    int getB2LinkAddress() const {return m_address; }

    /**
     * Set Belle2Link address.
     * @param[in] address B2Link address.
     */
    void setB2LinkAddress(int address) {m_address = address; }

    /**
     * Get Belle2Link port.
     * @return B2Link port.
     */
    int getB2LinkPort() const {return m_port; }

    /**
     * Set Belle2Link port.
     * @param[in] port B2Link port.
     */
    void setB2LinkPort(int port) {m_port = port; }


  private:

    int m_id;        /**< Belle2Link Identifier */
    int m_address;   /**< Belle2Link address */
    int m_port;      /**< integer Belle2Link port */

    ClassDef(ARICHB2LinkInfo, 1);  /**< ClassDef */
  };
} // end namespace Belle2
