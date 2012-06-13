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
#include<eklm/dataobjects/EKLMStripHit.h>
#include<eklm/dataobjects/EKLMSectorHit.h>
#include<framework/datastore/StoreArray.h>



namespace Belle2 {

  //! class for creation 1d, 2d hits and clusters identification
  class EKLMRecon   {

  public:
    //! constructor with default values
    EKLMRecon();


    //! destructor
    ~EKLMRecon() {};

    //! read StripHits from the datastore
    void readStripHits();

    //! creates Sector hits and fill it with StripHits
    void createSectorHits();

    //! Creates 2d hits in each SectorHit
    void create2dHits();


  private:

    /**
     * Storage
     */
    StoreArray<EKLMHit2d>m_hit2dArray;


    //! vector of StripHits
    std::vector <EKLMStripHit*> m_StripHitVector;

    //! vector of SectorHits
    std::vector <EKLMSectorHit*> m_SectorHitVector;

    //! vector of  2d hits
    std::vector <EKLMHit2d*> m_hit2dVector;

    /**
     * returns true if strip is along X
     */
    bool CheckStripOrientationX(const G4VPhysicalVolume*);


    /**
     * returns true strips have intersection
     * determins crossing point in the global rest frame
     * calculates chisq of the hit
     * and hittime
     */
    bool doesIntersect(const EKLMStripHit* , const EKLMStripHit* ,
                       TVector3&, double&, double&);

    /**
     * Adds trip hit to sector, return  true on success
     */
    bool addStripHitToSector(EKLMSectorHit* , EKLMStripHit*);



    /**
     * Speed of the first photon
     */
    double m_firstPhotonlightSpeed;

    /**
     * Time smearing
     */
    double m_sigmaT;

  };

} // end of namespace Belle2

#endif //EKLMRECON_H
