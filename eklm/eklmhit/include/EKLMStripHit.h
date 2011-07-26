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

#include  <eklm/eklmhit/EKLMHitBase.h>
#include  "globals.hh"
#include  "CLHEP/Vector/ThreeVector.h"



#include <string>
namespace Belle2 {

  //! Main reconstruction hit class. Containes infromation about the hitted strips
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

    //! Print stip name and some other useful info
    void Print();

    /*     //! sets histograms */
    /*     inline void setHistogramm( TH1D * his)  */
    /*     {m_fitHistograms = his;}; */

    //! sets the number of photo electorns
    inline void setNumberPhotoElectrons(const double &npe)
    {m_NumberPhotoElectrons = npe;};

    //! sets the time of the hit
    inline void setTime(const double  &time)
    {m_Time = time;};

    //! set  PDG code of the leading ( (grand-)mother ) particle
    inline void setLeadingParticlePDGCode(const int  &pdg)
    {m_LeadingParticlePDGCode = pdg;};

    //! returns name of the strip (the same as Volume name)
    inline std::string getName()const
    {return m_Name;};

    //! returns time of the hit
    inline double getTime()const
    {return m_Time;};

    //! returns number of photo electrons
    inline double getNumberPhotoElectrons()const
    {return m_NumberPhotoElectrons;};

    //! return  PDG code of the leading ( (grand-)mother ) particle
    inline int getLeadingParticlePDGCode()const
    {return m_LeadingParticlePDGCode;};

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

    /*     //! fit results */
    /*     TH1D * m_fitHistograms; */



    //! Makes objects storable
    ClassDef(Belle2::EKLMStripHit, 1);


  };



} // end of namespace Belle2

#endif //EKLMSTRIPHIT_H
