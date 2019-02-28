/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* Belle2 headers. */
#include <bklm/dataobjects/BKLMHit2d.h>
#include <eklm/dataobjects/EKLMHit2d.h>

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
    bool inBKLM() const;

    /**
     * Get BKLM hit.
     */
    BKLMHit2d* getBKLMHit2d() const;

    /**
     * Get EKLM hit.
     */
    EKLMHit2d* getEKLMHit2d() const;

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
