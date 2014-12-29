#pragma once

namespace Belle2 {
  namespace xmldb {
    /** Constants for the conversions applied to CData before they are put
     *  in the database. */
    enum CDATAConversion {
      CONVERSION_BZIP2 = 1 /**< The data has been compressed by bzip2. */
    };
  }
}

