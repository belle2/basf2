/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/* h2m.h */


#ifndef H2M_H
#define H2M_H


struct h2m_header_t {
  unsigned int h_n_words;           /* network byte order */
  unsigned int h_n_words_in_header; /* network byte order */
  unsigned int h_hltout_id;
  unsigned int h_reserved[1];
  unsigned int h_marker;            /* 0x5f5f5f5f */
};


struct h2m_footer_t {
  unsigned int f_reserved[2];
  unsigned int f_checksum;          /* checksum (currently 0) */
  unsigned int f_marker;            /* 0xa0a0a0a0 */
};


#endif /* H2M_H */

