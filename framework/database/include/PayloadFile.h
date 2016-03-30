/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <string>


namespace Belle2 {
  /**
   * A wrapper class for payload files used by the Database and DBStore classes.
   */
  class PayloadFile: public TObject {
  public:
    /**
     * Constructor.
     * @param fileName       Name of the payload file obtained from the database.
     */
    explicit PayloadFile(const std::string& fileName): m_fileName(fileName) {};

    /**
     * Get the name of the downloaded payload file.
     * @return    Local file name.
     */
    std::string getFileName() const {return m_fileName;};

    /**
     * Read the content of the payload (text) file.
     * @return    The content of the payload file
     */
    std::string getContent() const;

  private:
    std::string m_fileName; /**< Name of the payload file. **/
  };
}
