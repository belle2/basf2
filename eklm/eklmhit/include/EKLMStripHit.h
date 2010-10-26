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
#include  "CLHEP/Vector/ThreeVector.h"


#include <string>
namespace Belle2 {

  class EKLMStripHit : public EKLMHitBase  {

  public:

    //! Constructor
    EKLMStripHit() {};

    //! Constructor with name
    EKLMStripHit(const char *);

    //! Constructor with name
    EKLMStripHit(std::string &);

    //! Destructor
    ~EKLMStripHit() {};

    void Print();

    inline void setNumberPhotoElectrons(const double &npe)
    {m_NumberPhotoElectrons = npe;};

    inline void setTime(const double  &time)
    {m_Time = time;};

    inline void setLeadingParticlePDGCode(const int  &pdg)
    {m_LeadingParticlePDGCode = pdg;};



    inline std::string getName()const
    {return m_Name;};


    inline double getTime()const
    {return m_Time;};


    inline double getNumberPhotoElectrons()const
    {return m_NumberPhotoElectrons;};


    inline int getLeadingParticlePDGCode()const
    {return m_LeadingParticlePDGCode;};


    bool doesIntersect(EKLMStripHit *, CLHEP::Hep3Vector &);

    double getLightPropagationLength(CLHEP::Hep3Vector &);

  private:

    double m_NumberPhotoElectrons;
    int m_LeadingParticlePDGCode;
    double m_Time;
    double m_LightPropagationLength;

    ClassDef(EKLMStripHit, 1);


  };



} // end of namespace Belle2

#endif //EKLMSTRIPHIT_H
