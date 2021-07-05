##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# For safety since we encountered a hanging process
# when this file is executed with basf2
from ROOT import Belle2  # make Belle2 namespace available

import os


from ROOT import gSystem

gSystem.Load('libtracking_trackFindingCDC')


Plotter = Belle2.TrackFindingCDC.SVGPrimitivePlotter
AttributeMap = Belle2.TrackFindingCDC.PrimitivePlotter.AttributeMap

plotter = Plotter()

# Attach a style to a group
# Gets inherited by all sub elements
groupAttributes = AttributeMap()
groupAttributes['stroke'] = 'black'
groupAttributes['stroke-width'] = '5'

# Start a group
plotter.startGroup(groupAttributes)

# Draw an arrow
plotter.drawLine(0, 0, 100, 100)

# Elements may also have attributes, which override the defaults from the group
lineAttributes = AttributeMap()
lineAttributes['stroke'] = 'green'

# Draw a line with a styling
plotter.drawLine(0, 100, 100, 0, lineAttributes)

# Terminate the group
plotter.endGroup()

# Draw a circle arc
startX = 10
startY = 50

endX = 85
endY = 50

radius = 40
longArc = False
sweepFlag = False

arcAttributes = AttributeMap()
arcAttributes["fill"] = "none"
arcAttributes["stroke"] = "violet"
arcAttributes["stroke-width"] = "5"

plotter.drawCircleArc(startX, startY, endX, endY, radius, longArc, sweepFlag, arcAttributes)

# Draw an arrow
arrowAttributes = AttributeMap()
arrowAttributes["stroke"] = "yellow"
arrowAttributes["stroke-width"] = "5"

plotter.drawArrow(50, 90, 50, 10, arrowAttributes)


# Now test the animation behaviour.
# Only changing the visibility from hidden
# to visible is supported currently.
# Therefore the special attribute _showAt
# is used. In general attributes starting with
# _ are not propagated to the svg but have
# a special meaning

# Draw a row of circles that pop up one after the other
circleAttributes = AttributeMap()
circleAttributes["stroke"] = "green"
circleAttributes["stroke-width"] = "5"
circleAttributes["fill"] = "white"
circleAttributes["_showAt"] = "0s"

y = 25
r = 15

for secs, x in enumerate((25, 40, 55, 70)):
    circleAttributes["_showAt"] = str(secs) + "s"
    plotter.drawCircle(x, y, r, circleAttributes)


testFileName = "test.svg"

plotter.save(testFileName)

plotter.clear()

# Output the lines as a test case
with open(testFileName) as input_file:
    for line in input_file:
        print(line, end=' ')

os.remove(testFileName)
