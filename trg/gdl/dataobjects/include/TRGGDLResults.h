//-----------------------------------------------------------
// $Id$
//-----------------------------------------------------------
// Filename : TRGGDLResults.h
// Section  : TRG GDL
// Owner    : Chunhua Li
// Email    : chunhua.li@unimelb.edu.au
//-----------------------------------------------------------
// Description : A class to save TRG GDL information of event.
//-----------------------------------------------------------
// $Log$
//-----------------------------------------------------------
#pragma once
#include <TObject.h>
#include <TVector3.h>
namespace Belle2 {

  class TRGGDLResults : public TObject {
  public:

    /**! The Class title*/
    ClassDef(TRGGDLResults, 1); /*< the class title */
    TRGGDLResults(): m_L1TriggerResults(0) {}
    ~TRGGDLResults() {}


    /**set the trigger results*/
    void setL1TriggerRsults(int L1TriggerResults) {m_L1TriggerResults = L1TriggerResults;}

    /**get trigger result*/
    int getL1TriggerResults() const {return m_L1TriggerResults;}


  private:
    /**the trigger results of each trigger line*/
    int m_L1TriggerResults;

  };

} // end namespace Belle2

//#endif
