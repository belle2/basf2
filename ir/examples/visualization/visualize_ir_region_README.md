<!--
##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
-->

# IR Region Geometry Visualization Script

## Overview

The `visualize_ir_region.py` script generates an annotated SVG file showing a radial cross-section (R-Z view) of the Belle II IR Region geometry from the `ir/data/IR Region.xml` file.

## Features

- **Radial cross-section view**: Shows the geometry in the R-Z plane (radial vs. longitudinal)
- **Color-coded materials**: Different materials are shown with distinct colors
- **Interactive hover annotations**: Hover over any component to see a popup showing:
  - Component name (e.g., "A1spc1")
  - Description (e.g., "Right HER Beam pipe space 1")
  - Material (e.g., "Vacuum")
  - Mother and daughter volumes
- **Click to activate**: Click on any component to:
  - Highlight it with a red outline
  - Show a detailed information panel on the right side
  - Display a table of all planes with Z, R_inner, and R_outer coordinates
  - Click again to deactivate
- **Click material legend**: Click on any material in the legend to:
  - Highlight all components made of that material with red outlines
  - No information panel shown (useful for overview)
  - Click again to deactivate
- **Click volume list**: Click on any volume name in the list to:
  - Activate that volume with full details panel
  - All 106 volumes listed alphabetically in multiple columns
  - Compact 9px font with truncated names for space efficiency
- **Visual feedback**: Components highlight with bold outline when hovered
- **Symmetric display**: Shows both positive and negative R sides
- **Grid and axes**: Includes coordinate grid and axis labels
- **Material legend**: Lists all materials used in the geometry
- **Volume list**: Alphabetically sorted list of all 106 volumes with click-to-activate

## Usage

### Basic usage:
```bash
python3 visualize_ir_region.py
```

This reads `ir/data/IR Region.xml` and generates `shielding_study_vtx_beampipe_shield_no_ip.svg` in the current directory.

### Specify input and output files:
```bash
python3 visualize_ir_region.py path/to/IR Region.xml output.svg
```

### View with interactive features:
Open `view_ir_region.html` in a web browser to view the SVG with an enhanced user interface that includes:
- Instructions for using the hover annotations
- Zoom controls
- Information about the geometry
- Better layout for viewing

Alternatively, open `shielding_study_vtx_beampipe_shield_no_ip.svg` directly in a web browser.

**Note**: The SVG is 1900px wide to accommodate the volume list (3 columns on the right side).
Make sure your browser window is wide enough to see all three columns of the volume list.

## Output

The script generates an SVG file with:
- **Width**: 1600 pixels
- **Height**: 800 pixels
- **View**: R-Z cross section showing:
  - Z axis (horizontal): Longitudinal position along the beam
  - R axis (vertical): Radial distance from the beam axis
  - Both positive and negative R sides (symmetric)

## Viewing the Output

The generated SVG file can be opened with:
- **Web browsers** (Chrome, Firefox, Safari, Edge) - **Recommended for interactive features**
  - Hover over components to see name, description, and material
  - Components highlight on mouseover
- Image viewers (GIMP, Inkscape) - static view only
- Embedded in HTML documents - interactive features work
- Included in LaTeX documents (with appropriate packages) - static view only

**Note**: The interactive hover annotations require JavaScript support, which is available in web browsers but not in static image viewers.

## Material Colors

The script automatically assigns colors based on material type:
- **Vacuum/Air**: Light blue/gray
- **Steel (SUS316L)**: Silver
- **Titanium (Ti)**: Gray
- **Beryllium (Be)**: Light gray
- **Tantalum (Ta)**: Dark gray
- **Tungsten (W)**: Medium gray
- **Copper (Cu)**: Copper/orange
- **Iron (Fe)**: Red
- **Gold (Au)**: Gold
- **Superconducting coil (SC_COIL)**: Blue
- **G-10 (Cryo_G-10)**: Green
- **NbTi**: Purple
- **Paraffin**: Light orange

## Volumes Displayed

The script parses and displays all polycone volumes in the IR Region.xml file, including:

### Right side (positive Z):
- **A components**: Right HER (High Energy Ring) beam pipe and magnets
- **B components**: Right LER (Low Energy Ring) beam pipe and magnets
- **C components**: IR Region vessel, shields, and superconducting magnets

### Left side (negative Z):
- **D components**: Left HER beam pipe and magnets
- **E components**: Left LER beam pipe and magnets
- **F components**: IR Region vessel, shields, and superconducting magnets

### Limit tubes:
- **TubeR/TubeR2**: Right side boundary volumes
- **TubeL**: Left side boundary volumes

## Interactive Features

The SVG includes JavaScript-based interactivity:

### Hover Annotations
- **Component tooltip**: When you hover over any component, a white tooltip box appears showing:
  - Component name in bold (e.g., "A1spc1")
  - Component description (e.g., "Right HER Beam pipe space 1")
  - Material type (e.g., "Material: Vacuum")
  - Mother volume (e.g., "Mother: A1spc1") - if applicable
  - Daughter volumes (e.g., "Daughters: A2wal1, A3wal1, A3wal2, ...") - if any
- **Vertex tooltip**: When hovering near a vertex (plane boundary), a second tooltip appears showing:
  - Plane number (e.g., "Plane 5 (outer)")
  - Z coordinate (e.g., "Z: 69.51 cm")
  - R coordinate (e.g., "R: 2.40 cm")
  - Detection threshold: 15 pixels from vertex
- **Smart positioning**: Tooltips automatically reposition to stay within the viewport
- **Visual highlight**: The hovered component gets:
  - Bolder outline (stroke-width increases from 0.5 to 2)
  - Full opacity (changes from 0.7 to 1.0)
  - Darker stroke color
  - Cursor changes to pointer

### Click Activation
- **Click on any volume** to activate it:
  - Volume gets a red outline (stroke-width 3)
  - Information panel appears on the right side
  - Panel shows:
    - Volume name (bold title)
    - Description, material, mother, daughters
    - Number of planes
    - **Complete table** of all plane coordinates:
      - Plane number (0 to N-1)
      - Z coordinate (cm)
      - R_inner coordinate (cm)
      - R_outer coordinate (cm)
    - Shows up to 30 planes, with indicator if more exist
- **Click again** on the same volume to deactivate it
- **Close button** (×) in top-right corner of panel
- Only one volume can be active at a time

### Material Legend Interaction
- **Click on any material** in the legend (right side) to:
  - Highlight all volumes made of that material
  - All matching volumes get red outlines
  - No information panel is shown (for quick overview)
  - Useful for understanding material distribution
- **Click again** on the same material to deactivate
- Clicking a volume deactivates material selection
- Clicking a material deactivates volume selection

### Volume List Navigation
- **Clickable volume list** appears below the material legend:
  - All 106 volumes listed alphabetically
  - Displayed in multiple columns (3 columns with 40 rows each)
  - Compact font size (9px) with truncated names (12 characters)
  - Blue color (#0066cc) indicates clickability
- **Click on any volume name** to:
  - Activate that volume with red outline
  - Show full information panel with complete details
  - Display coordinate table for all planes
- **Hover tooltip** shows full volume name, description, and material
- Provides quick navigation without searching in the visualization

### Volume Hierarchy
The tooltips show the parent-child relationships between volumes:
- **Mother volume**: The volume that contains this component (e.g., A2wal1 is inside A1spc1)
- **Daughter volumes**: All volumes contained within this component
- This helps understand the nested structure of the geometry

### Vertex Detection
When you get close to a vertex (corner/plane of a volume):
- A second tooltip appears showing the exact coordinates
- Distinguishes between "outer" (outer radius) and "inner" (inner radius) vertices
- Shows the plane number (0 to N-1) for the polycone
- Useful for debugging geometry or understanding exact dimensions
- Detection radius: 15 pixels from cursor position

### Browser Compatibility
The interactive features work in all modern browsers:
- Chrome/Chromium
- Firefox
- Safari
- Edge
- Opera

The SVG also includes fallback `<title>` elements for basic tooltip support.

## Technical Details

### Geometry Parsing

The script parses XML elements with the following structure:
```xml
<ElementName>
  <N>number_of_planes</N>
  <sec name="Z0" unit="cm">value</sec>
  <sec name="Z1" unit="cm">value</sec>
  ...
  <sec name="R0" unit="cm">value</sec>
  <sec name="R1" unit="cm">value</sec>
  ...
  <sec name="r0" unit="cm">value</sec>  <!-- Inner radius (optional) -->
  <sec name="r1" unit="cm">value</sec>
  ...
  <Material>material_name</Material>
  <Desc>description</Desc>
</ElementName>
```

### Coordinate System

- **Z axis**: Longitudinal direction along the beam line
  - Positive Z: Forward direction (right side in Belle II)
  - Negative Z: Backward direction (left side in Belle II)
- **R axis**: Radial distance from the beam axis
  - Positive R: Upward direction
  - Negative R: Downward direction (mirrored for symmetry)

### Scale

The script automatically scales the geometry to fit within the plot area while maintaining aspect ratio.

## Requirements

- Python 3
- Standard library modules only (no external dependencies):
  - `xml.etree.ElementTree`
  - `sys`
  - `collections`

## Example Output Statistics

For the Belle II IR Region geometry:
- **Number of volumes**: 106
- **Number of materials**: 10 unique materials
- **Volume list items**: 106 clickable entries in 3 columns
- **Z range**: Approximately -792 cm to +876 cm
- **R range**: Approximately ±82.5 cm
- **Total file size**: ~344 KB (includes coordinate data, click handlers, material selection,
  volume list, and info panel)

## Customization

To modify the visualization, edit the following parameters in the script:

```python
# SVG dimensions
width = 1600
height = 800

# Margins
margin_left = 100
margin_right = 100
margin_top = 50
margin_bottom = 100

# Colors
colors = {
    'Vacuum': '#e8f4f8',
    'SUS316L': '#c0c0c0',
    # ... add or modify colors
}
```

## Troubleshooting

### No volumes found
- Check that the XML file path is correct
- Verify the XML file contains `<Content>` element with volume definitions

### Empty output
- Ensure volume elements have `<N>` (number of planes) defined
- Check that Z and R values are not empty

### Distorted geometry
- Verify units in XML are correctly specified (cm, mm, or m)
- Check that Z and R arrays have matching lengths

## Related Files

- **Input**: `ir/data/IR Region.xml` - Geometry definition
- **Script**: `visualize_ir_region.py` - Python script to generate SVG
- **Output**: `shielding_study_vtx_beampipe_shield_no_ip.svg` - Generated visualization
- **Viewer**: `view_ir_region.html` - HTML wrapper for enhanced viewing experience
- **C++ Creator**: `ir/geometry/src/GeoIR RegionCreator.cc` - Geometry implementation
