#pragma once

#include <string>

#include <TMatrixFBasefwd.h>

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

    /** Reformat given HTML string into terminal-friendly plain text. */
    std::string htmlToPlainText(const std::string& html);
  }
}
