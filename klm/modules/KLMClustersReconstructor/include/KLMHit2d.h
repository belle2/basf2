/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* Belle 2 headers. */
#include <klm/dataobjects/bklm/BKLMHit2d.h>
#include <klm/dataobjects/eklm/EKLMHit2d.h>

namespace Belle2 {

  /**
   * Class for simultaneous handling of the BKLM and EKLM 2d hits.
   */
  class KLMHit2d {

  public:

    /**
     * Constructor.
     * @param[in] bklmHit2d BKLM 2d hit.
     */
    explicit KLMHit2d(BKLMHit2d* bklmHit2d);

    /**
     * Constructor.
     * @param[in] eklmHit2d EKLM 2d hit.
     */
    explicit KLMHit2d(EKLMHit2d* eklmHit2d);

    /**
     * Destructor.
     */
    ~KLMHit2d();

    /**
     * Whether hit is in BKLM.
     */
    bool inBKLM() const
    {
      return m_bklmHit;
    }

    /**
     * Get BKLM hit.
     */
    BKLMHit2d* getBKLMHit2d() const
    {
      return m_bklmHit2d;
    }

    /**
     * Get EKLM hit.
     */
    EKLMHit2d* getEKLMHit2d() const
    {
      return m_eklmHit2d;
    }

    /**
     * Get hit position.
     */
    TVector3 getPosition() const;

    /**
     * Get hit time.
     */
    float getTime() const;

    /**
     * Get hit layer.
     */
    int getLayer() const;

  private:

    /** True if BKLM hit, false - EKLM. */
    bool m_bklmHit;

    /** BKLM hit. */
    BKLMHit2d* m_bklmHit2d;

    /** EKLM hit. */
    EKLMHit2d* m_eklmHit2d;

  };

}
