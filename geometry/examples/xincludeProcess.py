#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This script processes the input.xml , processes the xinclude statements and outputs the merged xml

import os
import libxml2
from optparse import OptionParser

uri = 'data/geometry/Belle2.xml'
LOG = 'Belle2-merged.xml'

dir = os.getenv('BELLE2_LOCAL_DIR')
if dir is None:
    dir = os.getenv('BELLE2_RELEASE_DIR')
if dir is None:
    print 'BELLE2_LOCAL_DIR or BELLE2_RELEASE_DIR is not set\n'
    print 'Set option arguyment -i FILE \n'

URI = dir + '/' + uri

parser = OptionParser()
parser.add_option(
    '-o',
    '--output',
    dest='filename',
    default=LOG,
    help='write merged xml to FILE',
    metavar='FILE',
    )
parser.add_option(
    '-i',
    '--input',
    dest='xmluri',
    default=URI,
    help='read xml from FILE',
    metavar='FILE',
    )

(options, args) = parser.parse_args()

log = open(options.filename, 'w')

try:
    print 'Reading %s' % options.xmluri
    doc = libxml2.parseFile(options.xmluri)
except:
    doc = None

if doc != None:
    res = doc.xincludeProcess()
    if res >= 0:
        result = doc.serialize()
        log.write('%s' % doc)
        doc.freeDoc()
        print 'Writing to %s ' % options.filename
    else:
        print 'Failed to parse %s' % options.xmluri

log.close()
