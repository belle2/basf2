/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef MAKEDUMHSLBDATA_H
#define MAKEDUMHSLBDATA_H

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
    virtual void initialize() override;

    //! event module
    virtual void event() override;

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
