/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Zbynek Drasal, Guofu Cao                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCCOLOURS_H
#define CDCCOLOURS_H

namespace Belle2 {

// If a terminal doesn't support any colors, comment following line
#define TERM_COLOR

#ifdef TERM_COLOR

#define DBLUE    "\x1b[36m"
#define DRED     "\x1b[31m"
#define DYELLOW  "\x1b[33m"
#define DGREEN   "\x1b[32m"
#define DUNDERL  "\x1b[4m"
#define ENDCOLOR "\x1b[m"

#else

#define DBLUE    ""
#define DRED     ""
#define DYELLOW  ""
#define DGREEN   ""
#define DUNDERL  ""
#define ENDCOLOR ""

#endif

} // Namespace

#endif // CDCCOLOURS_H
