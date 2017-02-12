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
     * @param color If true, vary background colour depending on value.
     */
    std::string getString(const TMatrixFBase& matrix, int precision = 2, bool color = true);

    /** get HTML table representing a matrix (double precision).
     *
     * @param color If true, vary background colour depending on value.
     */
    std::string getString(const TMatrixDBase& matrix, int precision = 2, bool color = true);

    /** get a string with vector coordinates: (x, y, z). (uses fixed-length output). */
    std::string getString(const TVector3& vec, int precision = 2);

    /** get a string with vector coordinates: (x, y, z). (uses fixed-length output).
     * converts floating point value from standard framework unit to given unit.
     *
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
