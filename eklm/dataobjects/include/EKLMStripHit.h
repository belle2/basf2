/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMSTRIPHIT_H
#define EKLMSTRIPHIT_H

#include <framework/datastore/DataStore.h>

#include  <eklm/dataobjects/EKLMHitBase.h>
#include  "globals.hh"
#include  "CLHEP/Vector/ThreeVector.h"

#include "G4VPhysicalVolume.hh"

#include <string>
namespace Belle2 {

  //! Main reconstruction hit class. Containes infromation about the
  //! hitted strips
  class EKLMStripHit : public EKLMHitBase {

  public:

    //! Constructor
    EKLMStripHit() {};

    //! Destructor
    ~EKLMStripHit() {};

    //! Print stip name and some other useful info
    void Print();



    //! returns number of photo electrons
    double getNumberPhotoElectrons() const;

    //! sets the number of photo electorns
    void setNumberPhotoElectrons(double);


    //! returns time of the hit
    double getTime() const;

    //! sets the time of the hit
    void setTime(double);



    //! return  PDG code of the leading ( (grand-)mother ) particle
    int getLeadingParticlePDGCode() const;

    //! set  PDG code of the leading ( (grand-)mother ) particle
    void setLeadingParticlePDGCode(int);



    //! returns physical volume
    const G4VPhysicalVolume *getVolume() const;

    //! set physical volume
    void setVolume(const G4VPhysicalVolume *pv);




    //! returns true if strips intersects (does not pay attention to the layer)
    bool doesIntersect(Belle2::EKLMStripHit *, CLHEP::Hep3Vector &);

    //! returns distance btw. the hit and SiPM
    double getLightPropagationLength(CLHEP::Hep3Vector &);



  private:

    //! number of photo electrons
    double m_NumberPhotoElectrons;

    //!  PDG code of the leading ( (grand-)mother ) particle
    int m_LeadingParticlePDGCode;

    //! hit time (the time of the first photo electorn)
    double m_Time;

    //! distance btw. the hit and SiPM
    double m_LightPropagationLength;

    //! Physical volume (for simulation)
    const G4VPhysicalVolume *m_pv;   //! {ROOT streamer directive}


    //! Makes objects storable
    ClassDef(Belle2::EKLMStripHit, 1);

  };

} // end of namespace Belle2

#endif //EKLMSTRIPHIT_H
