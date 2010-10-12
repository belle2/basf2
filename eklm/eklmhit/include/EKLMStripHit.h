/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMSTRIPHIT_H
#define EKLMSTRIPHIT_H

#include <framework/datastore/DataStore.h>
#include <TObject.h>

#include  <eklm/eklmhit/EKLMHitBase.h>
#include  "globals.hh"

#include <string>
namespace Belle2 {

  class EKLMStripHit : public EKLMHitBase  {

  public:

    //! Constructor
    EKLMStripHit() {};

    //! Destructor
    ~EKLMStripHit() {};

    void Print();

    inline void setNumberPhotoElectrons(const G4double &npe)
    {m_NumberPhotoElectrons = npe;};

    inline void setName(const std::string &name)
    {m_Name = name;};

    inline void setTime(const G4double  &time)
    {m_Time = time;};

    inline void setLeadingParticlePDGCode(const int  &pdg)
    {m_LeadingParticlePDGCode = pdg;};



    inline std::string getName()const
    {return m_Name;};


    inline G4double getTime()const
    {return m_Time;};


    inline G4double getNumberPhotoElectrons()const
    {return m_NumberPhotoElectrons;};


    inline G4int getLeadingParticlePDGCode()const
    {return m_LeadingParticlePDGCode;};




  private:

    G4double m_NumberPhotoElectrons;
    G4int m_LeadingParticlePDGCode;
    G4double m_Time;
    std::string m_Name;

    ClassDef(EKLMStripHit, 1);


  };



} // end of namespace Belle2

#endif //EKLMSTRIPHIT_H
