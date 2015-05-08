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

    /** Reformat given HTML string into terminal-friendly plain text. */
    std::string htmlToPlainText(const std::string& html);
  }
}
