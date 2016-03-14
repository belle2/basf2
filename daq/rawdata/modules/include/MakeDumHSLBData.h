//+
// File : MakeDUmHSLBData.h
// Description : Module to make a binary file from Raw*** events for input of wirte-dumhsb
//
// Author : Satoru Yamada, IPNS, KEK
// Date : Nov. 22, 2015
//-

#ifndef MAKEDUMHSLBDATA_H
#define MAKEDUMHSLBDATA_H
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <daq/rawdata/modules/PrintDataTemplate.h>


namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */
  class MakeDumHSLBDataModule : public PrintDataTemplateModule {
    // Public functions
  public:

    //! Constructor / Destructor
    MakeDumHSLBDataModule();
    virtual ~MakeDumHSLBDataModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void event();
    virtual void writeData(RawCOPPER* raw_copper, int i);

  protected :
    string m_out_fname;
    int m_filefd;
  };

} // end namespace Belle2

#endif // MODULEHELLO_H
