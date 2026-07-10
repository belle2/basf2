#!/usr/bin/env python3
"""
Generate an annotated SVG transverse (X-Y) cross-section of the Belle II VTX geometry.
This script parses the VTX XML files and creates a 2D X-Y view (the "windmill" structure).
"""

import os
import sys
import math
import xml.etree.ElementTree as ET
from pathlib import Path


def convert_to_mm(value, unit):
    v = float(value)
    if unit == 'mm':
        return v
    elif unit == 'cm':
        return v * 10.0
    elif unit == 'm':
        return v * 1000.0
    elif unit == 'um':
        return v / 1000.0
    return v


def parse_vtx_components(xml_file):
    tree = ET.parse(xml_file)
    root = tree.getroot()

    sensors = {}
    for elem in root.findall('.//Sensor'):
        if 'type' not in elem.attrib:
            continue
        if elem.find('width') is None:
            continue

        stype = elem.attrib['type']
        width = convert_to_mm(elem.find('width').text, elem.find('width').get('unit', 'mm'))
        length = convert_to_mm(elem.find('length').text, elem.find('length').get('unit', 'mm'))
        height = convert_to_mm(elem.find('height').text, elem.find('height').get('unit', 'mm'))
        mat_elem = elem.find('Material')
        col_elem = elem.find('Color')

        sensors[stype] = {
            'width': width,
            'length': length,
            'height': height,
            'material': mat_elem.text if mat_elem is not None else 'Si',
            'color': col_elem.text if col_elem is not None else '#006699'
        }

    ladder_types = {}
    for elem in root.findall('.//Ladder'):
        layer = int(elem.attrib['layer'])
        shift = convert_to_mm(elem.find('shift').text, elem.find('shift').get('unit', 'mm'))
        radius = convert_to_mm(elem.find('radius').text, elem.find('radius').get('unit', 'mm'))

        placements = []
        for s_elem in elem.findall('Sensor'):
            sid = s_elem.attrib['id']
            stype = s_elem.attrib['type']
            z = convert_to_mm(s_elem.text, s_elem.get('unit', 'mm'))
            placements.append({'id': sid, 'type': stype, 'z': z})

        ladder_types[layer] = {
            'shift': shift,
            'radius': radius,
            'sensors': placements
        }

    return sensors, ladder_types


def parse_vtx_main(xml_file):
    tree = ET.parse(xml_file)
    root = tree.getroot()

    ladders = []

    for shell in root.findall('.//HalfShell'):
        shell_name = shell.attrib.get('name', 'Unknown')

        for layer in shell.findall('.//Layer'):
            layer_id = int(layer.attrib['id'])
            for ladder in layer.findall('Ladder'):
                ladder_id = int(ladder.attrib['id'])
                phi_elem = ladder.find('phi')
                phi = float(phi_elem.text)

                ladders.append({
                    'shell': shell_name,
                    'layer': layer_id,
                    'ladder': ladder_id,
                    'phi_deg': phi,
                    'phi_rad': math.radians(phi)
                })

    return ladders


def generate_svg(sensors, ladder_types, ladders, output_file='vtx_xy_cross_section.svg'):
    # We want to draw an X-Y view of all sensors.
    # Because all sensors in a ladder perfectly overlap in X-Y, we just draw the ladder footprint once.

    # Bounding box
    max_r = 0
    for lad in ladder_types.values():
        r = lad['radius']
        w = 0 if not lad['sensors'] else sensors[lad['sensors'][0]['type']]['width']
        h = 0 if not lad['sensors'] else sensors[lad['sensors'][0]['type']]['height']
        s = lad['shift']

        # Max distance from origin in local coords
        dist = math.sqrt((s + w/2)**2 + (r + h/2)**2)
        if dist > max_r:
            max_r = dist

    max_r *= 1.2  # Add margin

    width = 1000
    height = 800
    cx = width / 2
    cy = height / 2
    scale = (min(width, height) / 2) / max_r

    def to_svg(x, y):
        # SVG y is down, so invert y
        return cx + x * scale, cy - y * scale

    svg_lines = [
        '<?xml version="1.0" encoding="UTF-8"?>',
        f'<svg width="{width}" height="{height}" xmlns="http://www.w3.org/2000/svg">',
        '<defs>',
        '<style>',
        '.volume { stroke: #333; stroke-width: 0.5; cursor: pointer; }',
        '.volume:hover { stroke: #000; stroke-width: 2; opacity: 1.0 !important; }',
        '.volume-active { stroke: #ff0000; stroke-width: 3; opacity: 1.0 !important; }',
        '.tooltip-box { fill: white; stroke: #333; stroke-width: 1; pointer-events: none; }',
        '.tooltip-text { font-family: Arial, sans-serif; font-size: 12px; fill: #000; pointer-events: none; }',
        (
            '.tooltip-text-bold { font-family: Arial, sans-serif; font-size: 13px; '
            'font-weight: bold; fill: #000; pointer-events: none; }'
        ),
        '</style>',
        '<script type="text/javascript"><![CDATA[',
        '  var tooltip = null;',
        '  function createTooltipElement() {',
        '    var svgNS = "http://www.w3.org/2000/svg";',
        '    tooltip = document.createElementNS(svgNS, "g");',
        '    tooltip.setAttribute("id", "tooltip");',
        '    var rect = document.createElementNS(svgNS, "rect");',
        '    rect.setAttribute("class", "tooltip-box");',
        '    rect.setAttribute("id", "tooltip-rect");',
        '    rect.setAttribute("x", "0");',
        '    rect.setAttribute("y", "0");',
        '    rect.setAttribute("width", "250");',
        '    rect.setAttribute("height", "100");',
        '    rect.setAttribute("rx", "5");',
        '    tooltip.appendChild(rect);',
        '    for (var i = 0; i < 6; i++) {',
        '      var text = document.createElementNS(svgNS, "text");',
        '      text.setAttribute("class", i === 0 ? "tooltip-text-bold" : "tooltip-text");',
        '      text.setAttribute("x", "10");',
        '      text.setAttribute("y", (22 + i * 15));',
        '      text.setAttribute("id", "tooltip-line" + i);',
        '      tooltip.appendChild(text);',
        '    }',
        '    document.documentElement.appendChild(tooltip);',
        '  }',
        '  function showTooltip(evt, lines) {',
        '    if (!tooltip) createTooltipElement();',
        '    for (var i = 0; i < 6; i++) {',
        '      var elem = document.getElementById("tooltip-line" + i);',
        '      if (i < lines.length) {',
        '        elem.textContent = lines[i];',
        '        elem.style.display = "block";',
        '      } else {',
        '        elem.style.display = "none";',
        '      }',
        '    }',
        '    var height = 25 + lines.length * 15 + 5;',
        '    document.getElementById("tooltip-rect").setAttribute("height", height);',
        '    var x = evt.clientX + 10;',
        '    var y = evt.clientY + 10;',
        '    var svgRect = evt.target.ownerSVGElement.getBoundingClientRect();',
        '    x = x - svgRect.left;',
        '    y = y - svgRect.top;',
        '    if (x + 260 > ' + str(width) + ') x = x - 270;',
        '    if (y + height + 10 > ' + str(height) + ') y = y - height - 20;',
        '    tooltip.setAttribute("transform", "translate(" + x + "," + y + ")");',
        '    tooltip.style.display = "block";',
        '  }',
        '  function hideTooltip() {',
        '    if (tooltip) tooltip.style.display = "none";',
        '  }',
        ']]></script>',
        '</defs>',
        f'<rect x="0" y="0" width="{width}" height="{height}" fill="white"/>',
        (
            f'<text x="{width/2}" y="30" font-family="Arial" font-size="16" '
            'font-weight="bold" text-anchor="middle">Belle II VTX - Transverse '
            'Cross Section (X-Y view)</text>'
        ),
        (
            f'<text x="{width/2}" y="50" font-family="Arial" font-size="12" '
            'fill="#666" text-anchor="middle">Looking down the beam pipe '
            '(+Z direction)</text>'
        ),
    ]

    # Axes
    svg_lines.append(f'<line x1="{cx - max_r*scale}" y1="{cy}" x2="{cx + max_r*scale}" y2="{cy}" stroke="#ddd" stroke-width="1"/>')
    svg_lines.append(f'<line x1="{cx}" y1="{cy - max_r*scale}" x2="{cx}" y2="{cy + max_r*scale}" stroke="#ddd" stroke-width="1"/>')

    # Concentric circles for scale (every 20mm)
    for r_mm in range(20, int(max_r)+20, 20):
        r_px = r_mm * scale
        svg_lines.append(f'<circle cx="{cx}" cy="{cy}" r="{r_px}" fill="none" stroke="#eee" stroke-width="1"/>')
        svg_lines.append(f'<text x="{cx + r_px + 2}" y="{cy - 2}" font-family="Arial" font-size="10" fill="#999">{r_mm}mm</text>')

    svg_lines.append('<!-- VTX Ladders -->')

    # Draw ladders
    for lad in ladders:
        layer = lad['layer']
        ltype = ladder_types.get(layer)
        if not ltype:
            continue

        radius = ltype['radius']
        shift = ltype['shift']
        phi = lad['phi_rad']

        # Dimensions of the sensor profile
        if not ltype['sensors']:
            continue
        stype = ltype['sensors'][0]['type']
        sinfo = sensors.get(stype)
        if not sinfo:
            continue

        w = sinfo['width']
        h = sinfo['height']

        # Local coordinates corners
        # x is tangential, y is radial.
        # The sensor center is at x=shift, y=radius
        lx = [shift - w/2, shift + w/2, shift + w/2, shift - w/2]
        ly = [radius + h/2, radius + h/2, radius - h/2, radius - h/2]

        # Rotate by phi to global
        pts = []
        for i in range(4):
            gx = lx[i] * math.cos(phi) - ly[i] * math.sin(phi)
            gy = lx[i] * math.sin(phi) + ly[i] * math.cos(phi)
            px, py = to_svg(gx, gy)
            pts.append(f"{px},{py}")

        color = sinfo['color']

        # Tooltip data
        lines = [
            f"Layer {layer} Ladder {lad['ladder']} ({lad['shell']})",
            f"Phi: {lad['phi_deg']:.1f}°",
            f"Radius: {radius:.1f} mm, Shift: {shift:.1f} mm",
            f"Sensors: {len(ltype['sensors'])} x {stype}",
            f"Dim: {w:.1f} (W) x {h:.2f} (H) x {sinfo['length']:.1f} (L) mm",
            f"Material: {sinfo['material']}"
        ]

        lines_json = "[" + ",".join([f"'{line}'" for line in lines]) + "]"

        svg_lines.append(f'<polygon points="{" ".join(pts)}" class="volume" fill="{color}" opacity="0.8" ')
        svg_lines.append(f'  onmousemove="showTooltip(evt, {lines_json})" onmouseout="hideTooltip()"/>')

    svg_lines.append('</svg>')

    with open(output_file, 'w') as f:
        f.write('\n'.join(svg_lines))

    print(f"Generated {output_file} successfully.")
    print(f"Max Radius: {max_r:.1f} mm")


def generate_rz_svg(sensors, ladder_types, output_file='vtx_rz_cross_section.svg'):
    # Z limits
    min_z = 0
    max_z = 0
    max_r = 0

    # We will compute the bounding boxes in R-Z for each sensor type/placement
    shapes = []

    for layer, lad in ladder_types.items():
        radius = lad['radius']
        shift = lad['shift']

        for sinfo in lad['sensors']:
            stype = sinfo['type']
            z_center = sinfo['z']

            sdef = sensors.get(stype)
            if not sdef:
                continue

            w = sdef['width']
            h = sdef['height']
            length_mm = sdef['length']

            # Local X, Y corners
            lx = [shift - w/2, shift + w/2, shift + w/2, shift - w/2]
            ly = [radius + h/2, radius + h/2, radius - h/2, radius - h/2]

            # Radii of corners
            rs = [math.sqrt(x**2 + y**2) for x, y in zip(lx, ly)]
            r_min = min(rs)
            r_max = max(rs)

            z_min_local = z_center - length_mm/2
            z_max_local = z_center + length_mm/2

            if z_min_local < min_z:
                min_z = z_min_local
            if z_max_local > max_z:
                max_z = z_max_local
            if r_max > max_r:
                max_r = r_max

            shapes.append({
                'layer': layer,
                'type': stype,
                'z_min': z_min_local,
                'z_max': z_max_local,
                'r_min': r_min,
                'r_max': r_max,
                'z_center': z_center,
                'color': sdef['color'],
                'material': sdef['material'],
                'dimensions': f"{w:.1f} (W) x {h:.2f} (H) x {length_mm:.1f} (L) mm",
                'shift': shift,
                'radius': radius
            })

    # Margins and scale
    max_r *= 1.2
    range_z = max_z - min_z
    min_z -= range_z * 0.1
    max_z += range_z * 0.1

    width = 1200
    height = 600

    margin_left = 80
    margin_right = 80
    margin_top = 50
    margin_bottom = 80

    plot_width = width - margin_left - margin_right
    plot_height = height - margin_top - margin_bottom

    scale_z = plot_width / (max_z - min_z)
    scale_r = plot_height / (2 * max_r)

    def z_to_x(z):
        return margin_left + (z - min_z) * scale_z

    def r_to_y(r):
        return margin_top + plot_height / 2 - r * scale_r

    svg_lines = [
        '<?xml version="1.0" encoding="UTF-8"?>',
        f'<svg width="{width}" height="{height}" xmlns="http://www.w3.org/2000/svg">',
        '<defs>',
        '<style>',
        '.volume { stroke: #333; stroke-width: 0.5; cursor: pointer; }',
        '.volume:hover { stroke: #000; stroke-width: 2; opacity: 1.0 !important; }',
        '.axis { stroke: #666; stroke-width: 1; }',
        '.axis-label { font-family: Arial, sans-serif; font-size: 12px; fill: #666; }',
        '.grid { stroke: #ddd; stroke-width: 0.5; }',
        '.tooltip-box { fill: white; stroke: #333; stroke-width: 1; pointer-events: none; }',
        '.tooltip-text { font-family: Arial, sans-serif; font-size: 12px; fill: #000; pointer-events: none; }',
        (
            '.tooltip-text-bold { font-family: Arial, sans-serif; font-size: 13px; '
            'font-weight: bold; fill: #000; pointer-events: none; }'
        ),
        '</style>',
        '<script type="text/javascript"><![CDATA[',
        '  var tooltip = null;',
        '  function createTooltipElement() {',
        '    var svgNS = "http://www.w3.org/2000/svg";',
        '    tooltip = document.createElementNS(svgNS, "g");',
        '    tooltip.setAttribute("id", "tooltip");',
        '    var rect = document.createElementNS(svgNS, "rect");',
        '    rect.setAttribute("class", "tooltip-box");',
        '    rect.setAttribute("id", "tooltip-rect");',
        '    rect.setAttribute("x", "0");',
        '    rect.setAttribute("y", "0");',
        '    rect.setAttribute("width", "250");',
        '    rect.setAttribute("height", "100");',
        '    rect.setAttribute("rx", "5");',
        '    tooltip.appendChild(rect);',
        '    for (var i = 0; i < 6; i++) {',
        '      var text = document.createElementNS(svgNS, "text");',
        '      text.setAttribute("class", i === 0 ? "tooltip-text-bold" : "tooltip-text");',
        '      text.setAttribute("x", "10");',
        '      text.setAttribute("y", (22 + i * 15));',
        '      text.setAttribute("id", "tooltip-line" + i);',
        '      tooltip.appendChild(text);',
        '    }',
        '    document.documentElement.appendChild(tooltip);',
        '  }',
        '  function showTooltip(evt, lines) {',
        '    if (!tooltip) createTooltipElement();',
        '    for (var i = 0; i < 6; i++) {',
        '      var elem = document.getElementById("tooltip-line" + i);',
        '      if (i < lines.length) {',
        '        elem.textContent = lines[i];',
        '        elem.style.display = "block";',
        '      } else {',
        '        elem.style.display = "none";',
        '      }',
        '    }',
        '    var height = 25 + lines.length * 15 + 5;',
        '    document.getElementById("tooltip-rect").setAttribute("height", height);',
        '    var x = evt.clientX + 10;',
        '    var y = evt.clientY + 10;',
        '    var svgRect = evt.target.ownerSVGElement.getBoundingClientRect();',
        '    x = x - svgRect.left;',
        '    y = y - svgRect.top;',
        '    if (x + 260 > ' + str(width) + ') x = x - 270;',
        '    if (y + height + 10 > ' + str(height) + ') y = y - height - 20;',
        '    tooltip.setAttribute("transform", "translate(" + x + "," + y + ")");',
        '    tooltip.style.display = "block";',
        '  }',
        '  function hideTooltip() {',
        '    if (tooltip) tooltip.style.display = "none";',
        '  }',
        ']]></script>',
        '</defs>',
        f'<rect x="0" y="0" width="{width}" height="{height}" fill="white"/>',
        (
            f'<text x="{width/2}" y="30" font-family="Arial" font-size="16" '
            'font-weight="bold" text-anchor="middle">Belle II VTX - Radial '
            'Cross Section (R-Z view)</text>'
        ),
    ]

    # Axes and Grid
    y_axis = r_to_y(0)
    svg_lines.append(f'<line x1="{margin_left}" y1="{y_axis}" x2="{width-margin_right}" y2="{y_axis}" class="axis"/>')
    svg_lines.append(f'<text x="{width/2}" y="{height-20}" class="axis-label" text-anchor="middle">Z (mm)</text>')

    x_axis = z_to_x(0)
    svg_lines.append(f'<line x1="{x_axis}" y1="{margin_top}" x2="{x_axis}" y2="{height-margin_bottom}" class="axis"/>')
    svg_lines.append(
        f'<text x="20" y="{height/2}" class="axis-label" text-anchor="middle" transform="rotate(-90 20 {height/2})">R (mm)</text>')

    # Draw Z grid lines every 50 mm
    for z in range(int(min_z/50)*50, int(max_z/50)*50 + 50, 50):
        if min_z <= z <= max_z:
            x = z_to_x(z)
            svg_lines.append(f'<line x1="{x}" y1="{margin_top}" x2="{x}" y2="{height-margin_bottom}" class="grid"/>')
            svg_lines.append(f'<text x="{x}" y="{height-margin_bottom+20}" class="axis-label" text-anchor="middle">{z}</text>')

    # Draw R grid lines every 20 mm
    for r in range(0, int(max_r) + 20, 20):
        y_pos = r_to_y(r)
        y_neg = r_to_y(-r)
        svg_lines.append(f'<line x1="{margin_left}" y1="{y_pos}" x2="{width-margin_right}" y2="{y_pos}" class="grid"/>')
        svg_lines.append(f'<line x1="{margin_left}" y1="{y_neg}" x2="{width-margin_right}" y2="{y_neg}" class="grid"/>')
        svg_lines.append(f'<text x="{margin_left-10}" y="{y_pos+4}" class="axis-label" text-anchor="end">{r}</text>')
        if r > 0:
            svg_lines.append(f'<text x="{margin_left-10}" y="{y_neg+4}" class="axis-label" text-anchor="end">{-r}</text>')

    svg_lines.append('<!-- VTX Sensors -->')

    for s in shapes:
        x1 = z_to_x(s['z_min'])
        x2 = z_to_x(s['z_max'])

        # Upper (positive R)
        y1_up = r_to_y(s['r_max'])
        y2_up = r_to_y(s['r_min'])

        # Lower (negative R)
        y1_dn = r_to_y(-s['r_min'])
        y2_dn = r_to_y(-s['r_max'])

        color = s['color']
        lines = [
            f"Layer {s['layer']}, {s['type']}",
            f"Z: {s['z_center']:.1f} mm",
            f"R inner: {s['r_min']:.2f} mm, R outer: {s['r_max']:.2f} mm",
            f"Dim: {s['dimensions']}",
            f"Shift: {s['shift']:.1f} mm, Radius: {s['radius']:.1f} mm",
            f"Material: {s['material']}"
        ]

        lines_json = "[" + ",".join([f"'{line}'" for line in lines]) + "]"

        # Draw upper rectangle
        svg_lines.append(
            f'<rect x="{x1}" y="{y1_up}" width="{x2-x1}" height="{y2_up-y1_up}" class="volume" fill="{color}" opacity="0.8" ')
        svg_lines.append(f'  onmousemove="showTooltip(evt, {lines_json})" onmouseout="hideTooltip()"/>')

        # Draw lower rectangle
        svg_lines.append(
            f'<rect x="{x1}" y="{y1_dn}" width="{x2-x1}" height="{y2_dn-y1_dn}" class="volume" fill="{color}" opacity="0.8" ')
        svg_lines.append(f'  onmousemove="showTooltip(evt, {lines_json})" onmouseout="hideTooltip()"/>')

    svg_lines.append('</svg>')

    with open(output_file, 'w') as f:
        f.write('\n'.join(svg_lines))

    print(f"Generated {output_file} successfully.")
    print(f"Z range: {min_z:.1f} to {max_z:.1f} mm")


def main():
    repo_root = Path(os.path.dirname(os.path.abspath(__file__))).parents[1]
    vtx_main = repo_root / 'vtx/data/VTX-CMOS5.xml'
    vtx_comp = repo_root / 'vtx/data/VTX-Components-CMOS5.xml'

    if len(sys.argv) > 1:
        vtx_main = Path(sys.argv[1])
    if len(sys.argv) > 2:
        vtx_comp = Path(sys.argv[2])

    print(f"Parsing components from {vtx_comp}...")
    sensors, ladder_types = parse_vtx_components(vtx_comp)

    print(f"Parsing geometry from {vtx_main}...")
    ladders = parse_vtx_main(vtx_main)

    out_file = 'vtx_xy_cross_section.svg'
    generate_svg(sensors, ladder_types, ladders, out_file)

    out_file_rz = 'vtx_rz_cross_section.svg'
    generate_rz_svg(sensors, ladder_types, out_file_rz)


if __name__ == '__main__':
    main()
