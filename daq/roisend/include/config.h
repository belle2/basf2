/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/* config.h */


#ifndef CONFIG_H
#define CONFIG_H


#define ROI_MQ_NAME               ("/roi")
#define ROI_MQ_DEPTH              (20)
#define ROI_MAX_PACKET_SIZE       (16384) /* bytes */
#define ROI_IO_TIMEOUT            (-1)     /* seconds (0 or negative specifies forever wait) */

#define NETWORK_ESTABLISH_TIMEOUT (-1)     /* seconds (0 or negative specifies forever wait) */
#define NETWORK_IO_TIMEOUT        (-1)     /* seconds (0 or negative specifies forever wait) */

#define MM_MAX_HLTOUT             (10)


#endif /* CONFIG_H */

