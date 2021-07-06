/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_NSMDataPaket_h
#define _Belle2_NSMDataPaket_h

namespace Belle2 {

  struct NSMDataPaket {

    static const unsigned short BUFFER_MAX = 492;
    static const unsigned int PORT = 9021;

    struct Header {
      unsigned short paketid;
      unsigned short flag;
      unsigned short id;
      unsigned short revision;
      unsigned int max;
      unsigned int offset;
      unsigned int size;
    } hdr;
    char buf[BUFFER_MAX];
  };

}

#endif
