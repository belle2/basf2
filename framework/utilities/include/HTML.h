/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TMatrixFBasefwd.h>
#include <TMatrixDBasefwd.h>

#include <string>

class TVector3;

namespace Belle2 {
  /** return information on objects in a nice format.
   *
   * @sa RelationsObject::getInfoHTML()
   */
  namespace HTML {
    /** get HTML table representing a matrix.
     *
     * @param matrix The matrix to be represented
     * @param precision The amount of significant digits to use
     * @param color If true, vary background colour depending on value.
     */
    std::string getString(const TMatrixFBase& matrix, int precision = 2, bool color = true);

    /** get HTML table representing a matrix (double precision).
     *
     * @param matrix The matrix to be represented
     * @param precision The amount of significant digits to use
     * @param color If true, vary background colour depending on value.
     */
    std::string getString(const TMatrixDBase& matrix, int precision = 2, bool color = true);

    /** get a string with vector coordinates: (x, y, z). (uses fixed-length output).
     * @param vec the vector to be printend
     * @param precision The amount of digits to use
     */
    std::string getString(const TVector3& vec, int precision = 2);

    /** get a string with vector coordinates: (x, y, z). (uses fixed-length output).
     * converts floating point value from standard framework unit to given unit.
     *
     * @param vec the vector to be printend
     * @param precision The amount of digits to use
     * @param unitType Defines the unit to convert the vector values from standard framework unit.
     */
    std::string getStringConvertToUnit(const TVector3& vec, int precision = 2,
                                       const std::string& unitType = "cm");

    /** get a string with a unit type to convert a vector, so that it is easily readable.
     * valid unit types are (um, cm) for now. the maximum of the vector entries defines the unit.
     */
    std::string chooseUnitOfLength(const TVector3& vec);

    /** Create hexdump of given buffer.
     *
     * @param buf the buffer
     * @param length size of buf in 32bit words
     */
    std::string getHexDump(const int* buf, int length);

    /** Reformat given HTML string into terminal-friendly plain text. */
    std::string htmlToPlainText(const std::string& html);

    /** Convert &, <, > etc. to entities. (not a complete list!)  */
    std::string escape(const std::string& str);
    /** inverse of escape() */
    std::string unescape(const std::string& str);
  }
}
