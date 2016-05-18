/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jo-Frederik Krohn                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef KlId_H
#define KlId_H

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  class KlId : public RelationsObject {

  public:

    KlId();

    KlId(float klid, float bkgProb, bool isKLM, bool isECL);

    virtual ~KlId() {};

    /** get Klong ID */
    inline float getKlId() const
    {return m_KlId;}

    /** get Klong ID */
    inline float getBkgProb() const
    {return m_bkgProb;}

    /** get Klong ID */
    inline bool isKLM() const
    {return m_isKLM;}

    /** get Klong ID */
    inline bool isECL() const
    {return m_isECL;}



  private:

    /** K long id of corresponding cluster */
    float m_KlId;
    /** output of beambkg classification */
    float m_bkgProb;
    /**  is from a  KLM cluister */
    bool  m_isKLM;
    /**  is from an ECL cluster */
    bool  m_isECL;


    /** Needed to make root object storable? */
    ClassDef(KlId, 1)  //TODO function finden und verstehen

  };
}
#endif
