#!/usr/bin/env python
# -*- coding: utf-8 -*-

from ROOT import gSystem
gSystem.Load('libframework')
gSystem.Load('libdataobjects')
gSystem.Load('libtracking')

from ROOT import Belle2
from ROOT import TClass
import sys

inputfile = sys.argv[1]

# print "Input: ", inputfile
data = ''
with open(inputfile, 'r') as inputfile:
    lines = inputfile.readlines()
    for line in lines[1:-1]:
        data += line
    # get everything but first and last line

data = data.replace(']]]]><![CDATA[>', ']]>')
data = data.replace('<![CDATA[', '', 1)  # remove first
data = data[:-4]  # remove newline and ]]>
# print data

# force loading class
TClass.GetClass('Belle2::VXDTFSecMap')

obj = Belle2.Stream.deserializeXML(data)
base64 = Belle2.Stream.serializeAndEncode(obj)
escaped = Belle2.Stream.escapeXML(base64)

print (lines[0])[:-1] + '\n' + escaped + '\n' + lines[-1]

# for testing equivalence of serialized data
# obj2 = Belle2.Stream.deserializeEncodedRawData(base64)
# xml2 = Belle2.Stream.serializeXML(obj2)
# xml_wrapped = Belle2.Stream.escapeXML(xml2)
#
##print lines[0][:-1] + base64 + lines[-1]
# print lines[0][:-1] + xml_wrapped + lines[-1]
