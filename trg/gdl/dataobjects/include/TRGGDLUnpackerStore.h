//-----------------------------------------------------------
// $Id$
//-----------------------------------------------------------
// Filename : TRGGDLUnpackerStore.h
// Section  : TRG GDL
// Owner    :
// Email    :
//-----------------------------------------------------------
// Description : A class to represent TRG GDL.
//-----------------------------------------------------------
// $Log$
//-----------------------------------------------------------
#ifndef TRGGDLUNPACKERSTORE_H
#define TRGGDLUNPACKERSTORE_H

#include <TObject.h>

namespace Belle2 {

  //! Example Detector
  class TRGGDLUnpackerStore : public TObject {
  public:


    int  m_unpacker[320];

    char m_unpackername[320][100];


    /** the class title */
    ClassDef(TRGGDLUnpackerStore, 2);

  };

} // end namespace Belle2
#endif
