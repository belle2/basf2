/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMRECON_H
#define EKLMRECON_H

#include<vector>
#include<eklm/eklmhit/EKLMStripHit.h>
#include<eklm/eklmhit/EKLMSectorHit.h>



namespace Belle2 {

  // class for creation 1d and 2d hits
  class EKLMRecon   {

  public:
    EKLMRecon() {};
    ~EKLMRecon() {};

    void readStripHits();
    void createSectorHits();
    void create2dHits();
  private:
    std::vector <EKLMStripHit*> m_StripHitVector;
    std::vector <EKLMSectorHit*> m_SectorHitVector;
    //    std::vector <EKLMHit2d> m_Hit2dVector;
  };

} // end of namespace Belle2

#endif //EKLMRECON_H
