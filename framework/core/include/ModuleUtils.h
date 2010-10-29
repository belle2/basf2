/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MODULEUTILS_H_
#define MODULEUTILS_H_

#include <string>

namespace Belle2 {

  /**
   * Class that provides various utility methods for module developers.
   *
   * This class provides various, static defined methods to support users
   * in developing modules.
   */
  class ModuleUtils {

  public:
    //-------------------------------------------------------
    //          filepath related methods
    //-------------------------------------------------------

    /**
     * Checks if the given filename exists.
     *
     * @param filename The parameter can be either a directory or a path+filename.
     * @return True if the filename exists. Returns false if an error occurred.
     */
    static bool fileNameExists(const std::string& filename);

    /**
     * Checks if the given filepath exists.
     *
     * If the specified filepath is a path+filename combination, the filename is removed.
     *
     * @param filepath The parameter can be either a directory or a path+filename.
     * @return True if the filepath exists. Returns false if an error occurred.
     */
    static bool filePathExists(const std::string& filepath);

    /**
     * Checks if the given filename exists and if it is a file.
     *
     * @param filename The path+filename that should be checked if it points to a file.
     * @return True if the given filename points to a file.
     */
    static bool isFile(const std::string& filename);

    /**
     * Checks if the given filename exists and if it is a directory.
     *
     * @param filename The path that should be checked if it points to a directory.
     * @return True if the given filename points to a directory.
     */
    static bool isDirectory(const std::string& filename);

  };

}

#endif /* MODULEUTILS_H_ */
