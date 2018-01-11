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
#include <rawdata/modules/PrintDataTemplate.h>


namespace Belle2 {

  /*! Module to make a binary file from Raw*** events for input of wirte-dumhsbx */
  class MakeDumHSLBDataModule : public PrintDataTemplateModule {
    // Public functions
  public:

    //! Constructor / Destructor
    MakeDumHSLBDataModule();
    virtual ~MakeDumHSLBDataModule();

    //! initialization
    virtual void initialize();

    //! event module
    virtual void event();

    //! write data
    virtual void writeData(RawCOPPER* raw_copper, int i);

  protected :

    //! Output file name
    std::string m_out_fname;

    //! File descripter
    int m_filefd;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
