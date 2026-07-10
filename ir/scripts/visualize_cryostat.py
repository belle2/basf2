#!/usr/bin/env python3
"""
Generate an annotated SVG radial cross-section of the Belle II Cryostat geometry.
This script parses the Cryostat.xml file and creates a 2D R-Z view.
"""

import json as json_module
from pathlib import Path
import xml.etree.ElementTree as ET
import sys


def convert_to_cm(value, unit):
    """Convert value to cm based on unit."""
    if unit == 'mm':
        return float(value) / 10.0
    elif unit == 'cm':
        return float(value)
    elif unit == 'm':
        return float(value) * 100.0
    return float(value)


def parse_polycone(element):
    """Parse a polycone volume (with N planes, Z, R, r arrays)."""
    name = element.tag
    if name == 'Straight':
        name = element.get('name', 'Unknown')

    # Get description
    desc_elem = element.find('Desc')
    desc = desc_elem.text if desc_elem is not None else ""

    # Get material
    mat_elem = element.find('Material')
    material = mat_elem.text if mat_elem is not None else ""

    # Get mother volume
    mother_elem = element.find('MotherVolume')
    mother = mother_elem.text if mother_elem is not None else ""

    # Get number of planes
    n_elem = element.find('N')
    if n_elem is None:
        return None
    n_planes = int(n_elem.text)

    # Parse Z, R, r arrays
    z_values = []
    r_outer = []
    r_inner = []

    for i in range(n_planes):
        # Z values
        z_elem = element.find(f"sec[@name='Z{i}']")
        if z_elem is not None and z_elem.text:
            unit = z_elem.get('unit', 'cm')
            z_values.append(convert_to_cm(z_elem.text, unit))

        # Outer radius
        r_elem = element.find(f"sec[@name='R{i}']")
        if r_elem is not None and r_elem.text:
            unit = r_elem.get('unit', 'cm')
            r_outer.append(convert_to_cm(r_elem.text, unit))

        # Inner radius
        r_elem = element.find(f"sec[@name='r{i}']")
        if r_elem is not None and r_elem.text:
            unit = r_elem.get('unit', 'cm')
            r_inner.append(convert_to_cm(r_elem.text, unit))
        else:
            r_inner.append(0.0)

    if not z_values or not r_outer:
        return None

    return {
        'name': name,
        'desc': desc,
        'material': material,
        'mother': mother,
        'z': z_values,
        'r_outer': r_outer,
        'r_inner': r_inner,
        'n_planes': n_planes
    }


def get_color_for_material(material):
    """Assign colors based on material type."""
    colors = {
        'Vacuum': '#e8f4f8',
        'Air': '#f0f0f0',
        'SUS316L': '#c0c0c0',
        'Ti': '#b8b8b8',
        'Be': '#d0d0d0',
        'Ta': '#a0a0a0',
        'W': '#808080',
        'Cu': '#d4a574',
        'Fe': '#cc6666',
        'Au': '#ffd700',
        'SC_COIL': '#4488ff',
        'Cryo_G-10': '#88cc88',
        'Paraffin': '#ffddaa',
        'NbTi': '#6666cc',
    }

    for key, color in colors.items():
        if key in material:
            return color
    return '#cccccc'


def parse_envelope_outline(xml_file, component_name, color):
    """Parse *-Envelope.xml and return outer border (and optional inner border)."""
    tree = ET.parse(xml_file)
    root = tree.getroot()

    outer = root.find('.//OuterPoints')
    if outer is None:
        return None

    z_outer = []
    r_outer = []
    for point in outer.findall('point'):
        z_elem = point.find('z')
        x_elem = point.find('x')
        if z_elem is None or x_elem is None or z_elem.text is None or x_elem.text is None:
            continue
        z_outer.append(convert_to_cm(z_elem.text, z_elem.get('unit', 'cm')))
        r_outer.append(abs(convert_to_cm(x_elem.text, x_elem.get('unit', 'cm'))))

    if not z_outer or not r_outer:
        return None

    outline = {
        'name': component_name,
        'color': color,
        'z': z_outer,
        'r_outer': r_outer,
    }

    inner = root.find('.//InnerPoints')
    if inner is not None:
        z_inner = []
        r_inner = []
        for point in inner.findall('point'):
            z_elem = point.find('z')
            x_elem = point.find('x')
            if z_elem is None or x_elem is None or z_elem.text is None or x_elem.text is None:
                continue
            z_inner.append(convert_to_cm(z_elem.text, z_elem.get('unit', 'cm')))
            r_inner.append(abs(convert_to_cm(x_elem.text, x_elem.get('unit', 'cm'))))
        if z_inner and r_inner:
            outline['z_inner'] = z_inner
            outline['r_inner'] = r_inner

    return outline


def parse_heavymetalshield_outlines(xml_file, color):
    """Parse HeavyMetalShield.xml planes and return border outlines for both shields."""
    tree = ET.parse(xml_file)
    root = tree.getroot()
    outlines = []

    for shield in root.findall('.//Shield'):
        name = shield.get('name', 'Shield')
        z_values = []
        r_outer = []
        r_inner = []
        for plane in shield.findall('Plane'):
            z_elem = plane.find('posZ')
            ri_elem = plane.find('innerRadius')
            ro_elem = plane.find('outerRadius')
            if z_elem is None or ro_elem is None or z_elem.text is None or ro_elem.text is None:
                continue
            z_values.append(convert_to_cm(z_elem.text, z_elem.get('unit', 'cm')))
            r_outer.append(abs(convert_to_cm(ro_elem.text, ro_elem.get('unit', 'cm'))))
            if ri_elem is not None and ri_elem.text is not None:
                r_inner.append(abs(convert_to_cm(ri_elem.text, ri_elem.get('unit', 'cm'))))
            else:
                r_inner.append(0.0)

        if z_values and r_outer:
            outlines.append({
                'name': f'HeavyMetalShield.{name}',
                'color': color,
                'z': z_values,
                'r_outer': r_outer,
                'z_inner': z_values,
                'r_inner': r_inner,
            })

    return outlines


def parse_beampipe_outline(xml_file, color):
    """Parse BeamPipe.xml and build the actual Lv1SUS profile used in GeoBeamPipeCreator.

    This follows the same Z/R construction as in ir/geometry/src/GeoBeamPipeCreator.cc
    for `geo_Lv1SUS_name`, which is the central beam pipe section around the IP.
    """
    tree = ET.parse(xml_file)
    root = tree.getroot()
    lv1 = root.find('.//Lv1SUS')
    if lv1 is None:
        return None

    def g(name):
        sec = lv1.find(f"sec[@name='{name}']")
        if sec is None or sec.text is None:
            raise ValueError(f"Lv1SUS missing {name}")
        return convert_to_cm(sec.text, sec.get('unit', 'cm'))

    # L and R values used by the C++ creator.
    L = {f"L{i}": g(f"L{i}") for i in range(1, 17)}
    R = {f"R{i}": g(f"R{i}") for i in range(1, 12)}

    # Reproduce Lv1SUS_Z[0..20] from GeoBeamPipeCreator.cc (in cm here).
    z = [0.0] * 21
    z[0] = -sum(L[f"L{i}"] for i in range(1, 9))
    z[1] = z[0] + L["L1"]
    z[2] = z[1]
    z[3] = z[2] + L["L2"]
    z[4] = z[3] + L["L3"]
    z[5] = z[4]
    z[6] = z[5] + L["L4"]
    z[7] = z[6] + L["L5"]
    z[8] = z[7] + L["L6"]
    z[9] = z[8] + L["L7"]
    z[10] = z[9] + L["L8"]
    z[11] = z[10] + L["L9"]
    z[12] = z[11] + L["L10"]
    z[13] = z[12] + L["L11"]
    z[14] = z[13] + L["L12"]
    z[15] = z[14] + L["L13"]
    z[16] = z[15]
    z[17] = z[16] + L["L14"]
    z[18] = z[17] + L["L15"]
    z[19] = z[18]
    z[20] = z[19] + L["L16"]

    # Reproduce Lv1SUS_rO[0..20] from GeoBeamPipeCreator.cc.
    ro = [0.0] * 21
    ro[0] = R["R1"]
    ro[1] = ro[0]
    ro[2] = R["R2"]
    ro[3] = ro[2]
    ro[4] = R["R3"]
    ro[5] = R["R4"]
    ro[6] = ro[5]
    ro[7] = R["R5"]
    ro[8] = ro[7]
    ro[9] = R["R6"]
    ro[10] = ro[9]
    ro[11] = ro[10]
    ro[12] = R["R7"]
    ro[13] = ro[12]
    ro[14] = R["R8"]
    ro[15] = ro[14]
    ro[16] = R["R9"]
    ro[17] = R["R10"]
    ro[18] = ro[17]
    ro[19] = R["R11"]
    ro[20] = ro[19]

    return {
        'name': 'BeamPipe.Lv1SUS (actual)',
        'color': color,
        'z': z,
        'r_outer': ro,
    }


def generate_svg(volumes_c, volumes_a, volumes_b, crossing_angle,
                 output_file='cryostat_cross_section.svg', extra_outlines=None):
    """Generate SVG file with radial cross-section."""

    if extra_outlines is None:
        extra_outlines = []

    # Combine all volumes for bounds calculation
    all_volumes = volumes_c + volumes_a + volumes_b

    # Determine plot bounds
    z_min = min(min(v['z']) for v in all_volumes if v['z'])
    z_max = max(max(v['z']) for v in all_volumes if v['z'])
    r_max = max(max(v['r_outer']) for v in all_volumes if v['r_outer'])

    for outline in extra_outlines:
        if outline.get('z'):
            z_min = min(z_min, min(outline['z']))
            z_max = max(z_max, max(outline['z']))
        if outline.get('r_outer'):
            r_max = max(r_max, max(outline['r_outer']))
        if outline.get('r_inner'):
            r_max = max(r_max, max(outline['r_inner']))

    # Add margins
    z_range = z_max - z_min
    z_min -= z_range * 0.05
    z_max += z_range * 0.05
    r_max *= 1.1

    # SVG dimensions and scaling
    width = 1900  # Increased to accommodate volume list columns
    height = 800
    margin_left = 100
    margin_right = 100
    margin_top = 50
    margin_bottom = 100

    plot_width = width - margin_left - margin_right
    plot_height = height - margin_top - margin_bottom

    # Scale factors
    scale_z = plot_width / (z_max - z_min)
    scale_r = plot_height / (2 * r_max)

    def z_to_x(z):
        return margin_left + (z - z_min) * scale_z

    def r_to_y(r):
        return margin_top + plot_height / 2 - r * scale_r

    # Start SVG
    svg_lines = [
        '<?xml version="1.0" encoding="UTF-8"?>',
        f'<svg width="{width}" height="{height}" xmlns="http://www.w3.org/2000/svg">',
        '<defs>',
        '<style>',
        '.volume { stroke: #333; stroke-width: 0.5; cursor: pointer; }',
        '.volume:hover { stroke: #000; stroke-width: 2; opacity: 1.0 !important; }',
        '.volume-active { stroke: #ff0000; stroke-width: 3; opacity: 1.0 !important; }',
        '.label { font-family: Arial, sans-serif; font-size: 10px; fill: #333; }',
        '.title { font-family: Arial, sans-serif; font-size: 16px; font-weight: bold; fill: #000; }',
        '.axis { stroke: #666; stroke-width: 1; }',
        '.axis-label { font-family: Arial, sans-serif; font-size: 12px; fill: #666; }',
        '.grid { stroke: #ddd; stroke-width: 0.5; }',
        '.tooltip-box { fill: white; stroke: #333; stroke-width: 1; pointer-events: none; }',
        '.tooltip-text { font-family: Arial, sans-serif; font-size: 11px; fill: #000; pointer-events: none; }',
        '.tooltip-text-bold { font-family: Arial, sans-serif; font-size: 12px; '
        'font-weight: bold; fill: #000; pointer-events: none; }',
        '.info-panel { fill: white; stroke: #333; stroke-width: 2; }',
        '.info-panel-title { font-family: Arial, sans-serif; font-size: 14px; '
        'font-weight: bold; fill: #000; }',
        '.info-panel-text { font-family: Arial, sans-serif; font-size: 10px; fill: #000; }',
        '.info-panel-header { font-family: Arial, sans-serif; font-size: 10px; '
        'font-weight: bold; fill: #000; }',
        '.close-button { cursor: pointer; }',
        '.close-button:hover { fill: #ff0000; }',
        '.component-outline { fill: none; stroke-width: 2.0; stroke-linejoin: round; }',
        '.component-outline-inner { fill: none; stroke-width: 1.2; stroke-dasharray: 4 3; opacity: 0.8; }',
        '</style>',
        '<script type="text/javascript"><![CDATA[',
        '  var tooltip = null;',
        '  var vertexTooltip = null;',
        '  function createTooltipElement() {',
        '    var svgNS = "http://www.w3.org/2000/svg";',
        '    tooltip = document.createElementNS(svgNS, "g");',
        '    tooltip.setAttribute("id", "tooltip");',
        '    var rect = document.createElementNS(svgNS, "rect");',
        '    rect.setAttribute("class", "tooltip-box");',
        '    rect.setAttribute("id", "tooltip-rect");',
        '    rect.setAttribute("x", "0");',
        '    rect.setAttribute("y", "0");',
        '    rect.setAttribute("width", "280");',
        '    rect.setAttribute("height", "120");',
        '    rect.setAttribute("rx", "5");',
        '    tooltip.appendChild(rect);',
        '    for (var i = 0; i < 8; i++) {',
        '      var text = document.createElementNS(svgNS, "text");',
        '      text.setAttribute("class", i === 0 ? "tooltip-text-bold" : "tooltip-text");',
        '      text.setAttribute("x", "10");',
        '      text.setAttribute("y", (20 + i * 14));',
        '      text.setAttribute("id", "tooltip-line" + i);',
        '      tooltip.appendChild(text);',
        '    }',
        '    document.documentElement.appendChild(tooltip);',
        '    vertexTooltip = document.createElementNS(svgNS, "g");',
        '    vertexTooltip.setAttribute("id", "vertex-tooltip");',
        '    var vrect = document.createElementNS(svgNS, "rect");',
        '    vrect.setAttribute("class", "tooltip-box");',
        '    vrect.setAttribute("x", "0");',
        '    vrect.setAttribute("y", "0");',
        '    vrect.setAttribute("width", "180");',
        '    vrect.setAttribute("height", "55");',
        '    vrect.setAttribute("rx", "5");',
        '    vertexTooltip.appendChild(vrect);',
        '    for (var i = 0; i < 3; i++) {',
        '      var vtext = document.createElementNS(svgNS, "text");',
        '      vtext.setAttribute("class", "tooltip-text-bold");',
        '      vtext.setAttribute("x", "10");',
        '      vtext.setAttribute("y", (18 + i * 14));',
        '      vtext.setAttribute("id", "vertex-line" + i);',
        '      vertexTooltip.appendChild(vtext);',
        '    }',
        '    document.documentElement.appendChild(vertexTooltip);',
        '  }',
        '  function showTooltip(evt, name, desc, material, mother, daughters, zCoords, rOuter, rInner) {',
        '    if (!tooltip) createTooltipElement();',
        '    var lines = [name, desc.substring(0, 45), "Material: " + material];',
        '    if (mother) lines.push("Mother: " + mother);',
        '    if (daughters && daughters.length > 0) {',
        '      var dStr = daughters.substring(0, 50);',
        '      if (daughters.length > 50) dStr += "...";',
        '      lines.push("Daughters: " + dStr);',
        '    }',
        '    var numLines = lines.length;',
        '    for (var i = 0; i < 8; i++) {',
        '      var elem = document.getElementById("tooltip-line" + i);',
        '      if (i < numLines) {',
        '        elem.textContent = lines[i];',
        '        elem.style.display = "block";',
        '      } else {',
        '        elem.style.display = "none";',
        '      }',
        '    }',
        '    var height = 20 + numLines * 14 + 10;',
        '    document.getElementById("tooltip-rect").setAttribute("height", height);',
        '    var x = evt.clientX + 10;',
        '    var y = evt.clientY + 10;',
        '    var svgRect = evt.target.ownerSVGElement.getBoundingClientRect();',
        '    x = x - svgRect.left;',
        '    y = y - svgRect.top;',
        '    if (x + 290 > ' + str(width) + ') x = x - 300;',
        '    if (y + height + 10 > ' + str(height) + ') y = y - height - 20;',
        '    tooltip.setAttribute("transform", "translate(" + x + "," + y + ")");',
        '    tooltip.style.display = "block";',
        '    checkVertexProximity(evt, zCoords, rOuter, rInner);',
        '  }',
        '  function checkVertexProximity(evt, zCoords, rOuter, rInner) {',
        '    if (!vertexTooltip) return;',
        '    var z = JSON.parse(zCoords);',
        '    var ro = JSON.parse(rOuter);',
        '    var ri = JSON.parse(rInner);',
        '    var pt = evt.target.ownerSVGElement.createSVGPoint();',
        '    pt.x = evt.clientX;',
        '    pt.y = evt.clientY;',
        '    var svgP = pt.matrixTransform(evt.target.ownerSVGElement.getScreenCTM().inverse());',
        '    var threshold = 15;',
        '    var closest = null;',
        '    var minDist = threshold;',
        '    for (var i = 0; i < z.length; i++) {',
        '      var zScreen = ' + str(margin_left) + ' + (z[i] - ' + str(z_min) + ') * ' + str(scale_z) + ';',
        '      var rOutScreen = ' + str(margin_top + plot_height/2) + ' - ro[i] * ' + str(scale_r) + ';',
        '      var dist = Math.sqrt(Math.pow(svgP.x - zScreen, 2) + Math.pow(svgP.y - rOutScreen, 2));',
        '      if (dist < minDist) { minDist = dist; closest = {plane: i, z: z[i], r: ro[i], type: "outer"}; }',
        '      if (ri[i] > 0) {',
        '        var rInScreen = ' + str(margin_top + plot_height/2) + ' - ri[i] * ' + str(scale_r) + ';',
        '        dist = Math.sqrt(Math.pow(svgP.x - zScreen, 2) + Math.pow(svgP.y - rInScreen, 2));',
        '        if (dist < minDist) { minDist = dist; closest = {plane: i, z: z[i], r: ri[i], type: "inner"}; }',
        '      }',
        '    }',
        '    if (closest) {',
        '      document.getElementById("vertex-line0").textContent = "Plane " + closest.plane + " (" + closest.type + ")";',
        '      document.getElementById("vertex-line1").textContent = "Z: " + closest.z.toFixed(2) + " cm";',
        '      document.getElementById("vertex-line2").textContent = "R: " + closest.r.toFixed(2) + " cm";',
        '      var vx = evt.clientX + 15;',
        '      var vy = evt.clientY - 70;',
        '      var svgRect = evt.target.ownerSVGElement.getBoundingClientRect();',
        '      vx = vx - svgRect.left;',
        '      vy = vy - svgRect.top;',
        '      if (vx + 190 > ' + str(width) + ') vx = vx - 200;',
        '      if (vy < 0) vy = evt.clientY - svgRect.top + 20;',
        '      vertexTooltip.setAttribute("transform", "translate(" + vx + "," + vy + ")");',
        '      vertexTooltip.style.display = "block";',
        '    } else {',
        '      vertexTooltip.style.display = "none";',
        '    }',
        '  }',
        '  function hideTooltip() {',
        '    if (tooltip) tooltip.style.display = "none";',
        '    if (vertexTooltip) vertexTooltip.style.display = "none";',
        '  }',
        '  var activeVolume = null;',
        '  var infoPanel = null;',
        '  function selectVolume(evt, name, desc, material, mother, daughters, zCoords, rOuter, rInner) {',
        '    var z = JSON.parse(zCoords);',
        '    var ro = JSON.parse(rOuter);',
        '    var ri = JSON.parse(rInner);',
        '    if (activeVolume && activeVolume.name === name) {',
        '      deselectVolume();',
        '      return;',
        '    }',
        '    if (activeMaterial) {',
        '      deselectMaterial();',
        '    }',
        '    if (activeVolume) {',
        '      var oldElems = document.querySelectorAll("[data-volume-name=\'" + activeVolume.name + "\']");',
        '      for (var i = 0; i < oldElems.length; i++) {',
        '        oldElems[i].classList.remove("volume-active");',
        '      }',
        '    }',
        '    activeVolume = {name: name, desc: desc, material: material, mother: mother, '
        'daughters: daughters, z: z, ro: ro, ri: ri};',
        '    var elems = document.querySelectorAll("[data-volume-name=\'" + name + "\']");',
        '    for (var i = 0; i < elems.length; i++) {',
        '      elems[i].classList.add("volume-active");',
        '    }',
        '    showInfoPanel();',
        '  }',
        '  function deselectVolume() {',
        '    if (activeVolume) {',
        '      var elems = document.querySelectorAll("[data-volume-name=\'" + activeVolume.name + "\']");',
        '      for (var i = 0; i < elems.length; i++) {',
        '        elems[i].classList.remove("volume-active");',
        '      }',
        '      activeVolume = null;',
        '    }',
        '    if (infoPanel) {',
        '      infoPanel.style.display = "none";',
        '    }',
        '  }',
        '  function showInfoPanel() {',
        '    if (!infoPanel) {',
        '      var svgNS = "http://www.w3.org/2000/svg";',
        '      infoPanel = document.createElementNS(svgNS, "g");',
        '      infoPanel.setAttribute("id", "info-panel");',
        '      var rect = document.createElementNS(svgNS, "rect");',
        '      rect.setAttribute("class", "info-panel");',
        '      rect.setAttribute("x", "' + str(width - margin_right - 350) + '");',
        '      rect.setAttribute("y", "' + str(margin_top + 100) + '");',
        '      rect.setAttribute("width", "340");',
        '      rect.setAttribute("height", "550");',
        '      rect.setAttribute("rx", "5");',
        '      infoPanel.appendChild(rect);',
        '      var closeBtn = document.createElementNS(svgNS, "text");',
        '      closeBtn.setAttribute("class", "close-button");',
        '      closeBtn.setAttribute("x", "' + str(width - margin_right - 25) + '");',
        '      closeBtn.setAttribute("y", "' + str(margin_top + 120) + '");',
        '      closeBtn.setAttribute("font-size", "20");',
        '      closeBtn.setAttribute("fill", "#666");',
        '      closeBtn.textContent = "×";',
        '      closeBtn.onclick = deselectVolume;',
        '      infoPanel.appendChild(closeBtn);',
        '      document.documentElement.appendChild(infoPanel);',
        '    }',
        '    while (infoPanel.childNodes.length > 2) {',
        '      infoPanel.removeChild(infoPanel.lastChild);',
        '    }',
        '    var svgNS = "http://www.w3.org/2000/svg";',
        '    var x = ' + str(width - margin_right - 340) + ';',
        '    var y = ' + str(margin_top + 120) + ';',
        '    var title = document.createElementNS(svgNS, "text");',
        '    title.setAttribute("class", "info-panel-title");',
        '    title.setAttribute("x", x);',
        '    title.setAttribute("y", y);',
        '    title.textContent = activeVolume.name;',
        '    infoPanel.appendChild(title);',
        '    y += 18;',
        '    var info = [',
        '      "Description: " + activeVolume.desc.substring(0, 35),',
        '      "Material: " + activeVolume.material,',
        '      "Mother: " + (activeVolume.mother || "none"),',
        '      "Daughters: " + (activeVolume.daughters ? activeVolume.daughters.substring(0, 30) + "..." : "none"),',
        '      "Number of planes: " + activeVolume.z.length',
        '    ];',
        '    for (var i = 0; i < info.length; i++) {',
        '      var text = document.createElementNS(svgNS, "text");',
        '      text.setAttribute("class", "info-panel-text");',
        '      text.setAttribute("x", x);',
        '      text.setAttribute("y", y);',
        '      text.textContent = info[i];',
        '      infoPanel.appendChild(text);',
        '      y += 14;',
        '    }',
        '    y += 10;',
        '    var headers = ["Plane", "Z (cm)", "R_in (cm)", "R_out (cm)"];',
        '    for (var i = 0; i < headers.length; i++) {',
        '      var hdr = document.createElementNS(svgNS, "text");',
        '      hdr.setAttribute("class", "info-panel-header");',
        '      hdr.setAttribute("x", x + i * 80);',
        '      hdr.setAttribute("y", y);',
        '      hdr.textContent = headers[i];',
        '      infoPanel.appendChild(hdr);',
        '    }',
        '    y += 14;',
        '    var maxRows = 30;',
        '    for (var i = 0; i < Math.min(activeVolume.z.length, maxRows); i++) {',
        '      var row = [i, activeVolume.z[i].toFixed(2), activeVolume.ri[i].toFixed(2), activeVolume.ro[i].toFixed(2)];',
        '      for (var j = 0; j < row.length; j++) {',
        '        var cell = document.createElementNS(svgNS, "text");',
        '        cell.setAttribute("class", "info-panel-text");',
        '        cell.setAttribute("x", x + j * 80);',
        '        cell.setAttribute("y", y);',
        '        cell.textContent = row[j];',
        '        infoPanel.appendChild(cell);',
        '      }',
        '      y += 13;',
        '    }',
        '    if (activeVolume.z.length > maxRows) {',
        '      var more = document.createElementNS(svgNS, "text");',
        '      more.setAttribute("class", "info-panel-text");',
        '      more.setAttribute("x", x);',
        '      more.setAttribute("y", y);',
        '      more.textContent = "... (" + (activeVolume.z.length - maxRows) + " more planes)";',
        '      infoPanel.appendChild(more);',
        '    }',
        '    infoPanel.style.display = "block";',
        '  }',
        '  var activeMaterial = null;',
        '  function selectMaterial(material) {',
        '    if (activeMaterial === material) {',
        '      deselectMaterial();',
        '      return;',
        '    }',
        '    if (activeMaterial) {',
        '      var oldElems = document.querySelectorAll("[data-material=\'" + activeMaterial + "\']");',
        '      for (var i = 0; i < oldElems.length; i++) {',
        '        oldElems[i].classList.remove("volume-active");',
        '      }',
        '    }',
        '    if (activeVolume) {',
        '      deselectVolume();',
        '    }',
        '    activeMaterial = material;',
        '    var elems = document.querySelectorAll("[data-material=\'" + material + "\']");',
        '    for (var i = 0; i < elems.length; i++) {',
        '      elems[i].classList.add("volume-active");',
        '    }',
        '  }',
        '  function deselectMaterial() {',
        '    if (activeMaterial) {',
        '      var elems = document.querySelectorAll("[data-material=\'" + activeMaterial + "\']");',
        '      for (var i = 0; i < elems.length; i++) {',
        '        elems[i].classList.remove("volume-active");',
        '      }',
        '      activeMaterial = null;',
        '    }',
        '  }',
        ']]></script>',
        '</defs>',
        '',
        '<!-- Background -->',
        f'<rect x="0" y="0" width="{width}" height="{height}" fill="white"/>',
        '',
        '<!-- Title -->',
        f'<text x="{width/2}" y="30" class="title" text-anchor="middle">Belle II Cryostat - Radial Cross Section (R-Z view)</text>',
        '',
        '<!-- Grid lines -->',
    ]

    # Add vertical grid lines every 100 cm
    for z in range(int(z_min/100)*100, int(z_max/100)*100 + 100, 100):
        if z_min <= z <= z_max:
            x = z_to_x(z)
            svg_lines.append(f'<line x1="{x}" y1="{margin_top}" x2="{x}" y2="{height-margin_bottom}" class="grid"/>')

    # Add horizontal grid lines
    for r in range(0, int(r_max) + 10, 10):
        y_pos = r_to_y(r)
        y_neg = r_to_y(-r)
        svg_lines.append(f'<line x1="{margin_left}" y1="{y_pos}" x2="{width-margin_right}" y2="{y_pos}" class="grid"/>')
        svg_lines.append(f'<line x1="{margin_left}" y1="{y_neg}" x2="{width-margin_right}" y2="{y_neg}" class="grid"/>')

    svg_lines.append('')
    svg_lines.append('<!-- Axes -->')

    # Z axis (horizontal)
    y_axis = r_to_y(0)
    svg_lines.append(f'<line x1="{margin_left}" y1="{y_axis}" x2="{width-margin_right}" y2="{y_axis}" class="axis"/>')
    svg_lines.append(f'<text x="{width/2}" y="{height-20}" class="axis-label" text-anchor="middle">Z (cm)</text>')

    # R axis (vertical)
    x_axis = z_to_x(0)
    svg_lines.append(f'<line x1="{x_axis}" y1="{margin_top}" x2="{x_axis}" y2="{height-margin_bottom}" class="axis"/>')
    svg_lines.append(
        f'<text x="20" y="{height/2}" class="axis-label" text-anchor="middle" transform="rotate(-90 20 {height/2})">R (cm)</text>')

    # Add axis labels
    for z in range(int(z_min/100)*100, int(z_max/100)*100 + 100, 100):
        if z_min <= z <= z_max:
            x = z_to_x(z)
            svg_lines.append(f'<text x="{x}" y="{height-margin_bottom+20}" class="axis-label" text-anchor="middle">{z}</text>')

    for r in [0, 20, 40, 60]:
        if r <= r_max:
            y_pos = r_to_y(r)
            svg_lines.append(f'<text x="{margin_left-10}" y="{y_pos+4}" class="axis-label" text-anchor="end">{r}</text>')
            if r > 0:
                y_neg = r_to_y(-r)
                svg_lines.append(f'<text x="{margin_left-10}" y="{y_neg+4}" class="axis-label" text-anchor="end">{-r}</text>')

    # Helper function to apply rotation to Z,R coordinates
    def rotate_coordinates(z, r, angle):
        """Apply rotation around the origin in Z-R plane."""
        import math
        cos_a = math.cos(angle)
        sin_a = math.sin(angle)
        z_rot = z * cos_a - r * sin_a
        r_rot = z * sin_a + r * cos_a
        return z_rot, r_rot

    svg_lines.append('')
    svg_lines.append('<!-- Volumes -->')

    # Draw C volumes (central, no rotation)
    svg_lines.append('<!-- C volumes (Central, no rotation) -->')
    for vol in volumes_c:
        if not vol['z'] or not vol['r_outer']:
            continue

        color = vol.get('override_color') or get_color_for_material(vol['material'])

        # Create polygon for upper half (positive R)
        points_upper = []
        for i in range(len(vol['z'])):
            z = vol['z'][i]
            r_out = vol['r_outer'][i] if i < len(vol['r_outer']) else vol['r_outer'][-1]
            points_upper.append(f"{z_to_x(z)},{r_to_y(r_out)}")

        # Add inner radius points in reverse
        for i in range(len(vol['z'])-1, -1, -1):
            z = vol['z'][i]
            r_in = vol['r_inner'][i] if i < len(vol['r_inner']) else 0
            points_upper.append(f"{z_to_x(z)},{r_to_y(r_in)}")

        # Escape strings for JavaScript
        name_escaped = vol["name"].replace("'", "\\'").replace('"', '&quot;')
        desc_escaped = vol["desc"].replace("'", "\\'").replace('"', '&quot;')
        mat_escaped = vol["material"].replace("'", "\\'").replace('"', '&quot;')
        mother_escaped = vol.get("mother", "").replace("'", "\\'").replace('"', '&quot;')
        daughters_str = ", ".join(vol.get("daughters", []))
        daughters_escaped = daughters_str.replace("'", "\\'").replace('"', '&quot;')

        # Serialize coordinate arrays as JSON for JavaScript
        z_json = json_module.dumps(vol['z'])
        r_outer_json = json_module.dumps(vol['r_outer'])
        r_inner_json = json_module.dumps(vol['r_inner'])

        svg_lines.append(
            f'<polygon points="{" ".join(points_upper)}" class="volume" '
            f'fill="{color}" opacity="0.7" ')
        svg_lines.append(f'  data-volume-name="{name_escaped}" ')
        svg_lines.append(f'  data-material="{mat_escaped}" ')
        onmousemove_str = (
            f"  onmousemove=\"showTooltip(evt, '{name_escaped}', '{desc_escaped}', "
            f"'{mat_escaped}', '{mother_escaped}', '{daughters_escaped}', "
            f"'{z_json}', '{r_outer_json}', '{r_inner_json}')\" ")
        svg_lines.append(onmousemove_str)
        svg_lines.append('  onmouseout="hideTooltip()" ')
        onclick_str = (
            f"  onclick=\"selectVolume(evt, '{name_escaped}', '{desc_escaped}', "
            f"'{mat_escaped}', '{mother_escaped}', '{daughters_escaped}', "
            f"'{z_json}', '{r_outer_json}', '{r_inner_json}')\">")
        svg_lines.append(onclick_str)
        svg_lines.append(f'  <title>{vol["name"]}: {vol["desc"]} ({vol["material"]})</title>')
        svg_lines.append('</polygon>')

        # Create polygon for lower half (negative R)
        points_lower = []
        for i in range(len(vol['z'])):
            z = vol['z'][i]
            r_out = vol['r_outer'][i] if i < len(vol['r_outer']) else vol['r_outer'][-1]
            points_lower.append(f"{z_to_x(z)},{r_to_y(-r_out)}")

        # Add inner radius points in reverse
        for i in range(len(vol['z'])-1, -1, -1):
            z = vol['z'][i]
            r_in = vol['r_inner'][i] if i < len(vol['r_inner']) else 0
            points_lower.append(f"{z_to_x(z)},{r_to_y(-r_in)}")

        svg_lines.append(
            f'<polygon points="{" ".join(points_lower)}" class="volume" '
            f'fill="{color}" opacity="0.7" ')
        svg_lines.append(f'  data-volume-name="{name_escaped}" ')
        svg_lines.append(f'  data-material="{mat_escaped}" ')
        onmousemove_str = (
            f"  onmousemove=\"showTooltip(evt, '{name_escaped}', '{desc_escaped}', "
            f"'{mat_escaped}', '{mother_escaped}', '{daughters_escaped}', "
            f"'{z_json}', '{r_outer_json}', '{r_inner_json}')\" ")
        svg_lines.append(onmousemove_str)
        svg_lines.append('  onmouseout="hideTooltip()" ')
        onclick_str = (
            f"  onclick=\"selectVolume(evt, '{name_escaped}', '{desc_escaped}', "
            f"'{mat_escaped}', '{mother_escaped}', '{daughters_escaped}', "
            f"'{z_json}', '{r_outer_json}', '{r_inner_json}')\">")
        svg_lines.append(onclick_str)
        svg_lines.append(f'  <title>{vol["name"]}: {vol["desc"]} ({vol["material"]})</title>')
        svg_lines.append('</polygon>')

    # Draw A volumes with rotation (HER, positive angle)
    svg_lines.append('')
    svg_lines.append(f'<!-- A volumes (HER, rotated +{crossing_angle["HER"]:.4f} rad) -->')
    her_angle = crossing_angle['HER']
    for vol in volumes_a:
        if not vol['z'] or not vol['r_outer']:
            continue

        color = vol.get('override_color') or get_color_for_material(vol['material'])

        # Create polygon for upper half with rotation
        points_upper = []
        for i in range(len(vol['z'])):
            z = vol['z'][i]
            r_out = vol['r_outer'][i] if i < len(vol['r_outer']) else vol['r_outer'][-1]
            z_rot, r_rot = rotate_coordinates(z, r_out, her_angle)
            points_upper.append(f"{z_to_x(z_rot)},{r_to_y(r_rot)}")

        # Add inner radius points in reverse
        for i in range(len(vol['z'])-1, -1, -1):
            z = vol['z'][i]
            r_in = vol['r_inner'][i] if i < len(vol['r_inner']) else 0
            z_rot, r_rot = rotate_coordinates(z, r_in, her_angle)
            points_upper.append(f"{z_to_x(z_rot)},{r_to_y(r_rot)}")

        # Escape and serialize data (same as C volumes)
        name_escaped = vol["name"].replace("'", "\\'").replace('"', '&quot;')
        desc_escaped = vol["desc"].replace("'", "\\'").replace('"', '&quot;')
        mat_escaped = vol["material"].replace("'", "\\'").replace('"', '&quot;')
        mother_escaped = vol.get("mother", "").replace("'", "\\'").replace('"', '&quot;')
        daughters_str = ", ".join(vol.get("daughters", []))
        daughters_escaped = daughters_str.replace("'", "\\'").replace('"', '&quot;')

        z_json = json_module.dumps(vol['z'])
        r_outer_json = json_module.dumps(vol['r_outer'])
        r_inner_json = json_module.dumps(vol['r_inner'])

        svg_lines.append(
            f'<polygon points="{" ".join(points_upper)}" class="volume" '
            f'fill="{color}" opacity="0.7" ')
        svg_lines.append(f'  data-volume-name="{name_escaped}" ')
        svg_lines.append(f'  data-material="{mat_escaped}" ')
        onmousemove_str = (
            f"  onmousemove=\"showTooltip(evt, '{name_escaped}', '{desc_escaped}', "
            f"'{mat_escaped}', '{mother_escaped}', '{daughters_escaped}', "
            f"'{z_json}', '{r_outer_json}', '{r_inner_json}')\" ")
        svg_lines.append(onmousemove_str)
        svg_lines.append('  onmouseout="hideTooltip()" ')
        onclick_str = (
            f"  onclick=\"selectVolume(evt, '{name_escaped}', '{desc_escaped}', "
            f"'{mat_escaped}', '{mother_escaped}', '{daughters_escaped}', "
            f"'{z_json}', '{r_outer_json}', '{r_inner_json}')\">")
        svg_lines.append(onclick_str)
        svg_lines.append(f'  <title>{vol["name"]}: {vol["desc"]} ({vol["material"]})</title>')
        svg_lines.append('</polygon>')

        # Lower half with rotation
        points_lower = []
        for i in range(len(vol['z'])):
            z = vol['z'][i]
            r_out = vol['r_outer'][i] if i < len(vol['r_outer']) else vol['r_outer'][-1]
            z_rot, r_rot = rotate_coordinates(z, -r_out, her_angle)
            points_lower.append(f"{z_to_x(z_rot)},{r_to_y(r_rot)}")

        for i in range(len(vol['z'])-1, -1, -1):
            z = vol['z'][i]
            r_in = vol['r_inner'][i] if i < len(vol['r_inner']) else 0
            z_rot, r_rot = rotate_coordinates(z, -r_in, her_angle)
            points_lower.append(f"{z_to_x(z_rot)},{r_to_y(r_rot)}")

        svg_lines.append(
            f'<polygon points="{" ".join(points_lower)}" class="volume" '
            f'fill="{color}" opacity="0.7" ')
        svg_lines.append(f'  data-volume-name="{name_escaped}" ')
        svg_lines.append(f'  data-material="{mat_escaped}" ')
        svg_lines.append(onmousemove_str)
        svg_lines.append('  onmouseout="hideTooltip()" ')
        svg_lines.append(onclick_str)
        svg_lines.append(f'  <title>{vol["name"]}: {vol["desc"]} ({vol["material"]})</title>')
        svg_lines.append('</polygon>')

    # Draw B volumes with rotation (LER, negative angle)
    svg_lines.append('')
    svg_lines.append(f'<!-- B volumes (LER, rotated {crossing_angle["LER"]:.4f} rad) -->')
    ler_angle = crossing_angle['LER']
    for vol in volumes_b:
        if not vol['z'] or not vol['r_outer']:
            continue

        color = vol.get('override_color') or get_color_for_material(vol['material'])

        # Create polygon for upper half with rotation
        points_upper = []
        for i in range(len(vol['z'])):
            z = vol['z'][i]
            r_out = vol['r_outer'][i] if i < len(vol['r_outer']) else vol['r_outer'][-1]
            z_rot, r_rot = rotate_coordinates(z, r_out, ler_angle)
            points_upper.append(f"{z_to_x(z_rot)},{r_to_y(r_rot)}")

        for i in range(len(vol['z'])-1, -1, -1):
            z = vol['z'][i]
            r_in = vol['r_inner'][i] if i < len(vol['r_inner']) else 0
            z_rot, r_rot = rotate_coordinates(z, r_in, ler_angle)
            points_upper.append(f"{z_to_x(z_rot)},{r_to_y(r_rot)}")

        # Escape and serialize data
        name_escaped = vol["name"].replace("'", "\\'").replace('"', '&quot;')
        desc_escaped = vol["desc"].replace("'", "\\'").replace('"', '&quot;')
        mat_escaped = vol["material"].replace("'", "\\'").replace('"', '&quot;')
        mother_escaped = vol.get("mother", "").replace("'", "\\'").replace('"', '&quot;')
        daughters_str = ", ".join(vol.get("daughters", []))
        daughters_escaped = daughters_str.replace("'", "\\'").replace('"', '&quot;')

        z_json = json_module.dumps(vol['z'])
        r_outer_json = json_module.dumps(vol['r_outer'])
        r_inner_json = json_module.dumps(vol['r_inner'])

        svg_lines.append(
            f'<polygon points="{" ".join(points_upper)}" class="volume" '
            f'fill="{color}" opacity="0.7" ')
        svg_lines.append(f'  data-volume-name="{name_escaped}" ')
        svg_lines.append(f'  data-material="{mat_escaped}" ')
        onmousemove_str = (
            f"  onmousemove=\"showTooltip(evt, '{name_escaped}', '{desc_escaped}', "
            f"'{mat_escaped}', '{mother_escaped}', '{daughters_escaped}', "
            f"'{z_json}', '{r_outer_json}', '{r_inner_json}')\" ")
        svg_lines.append(onmousemove_str)
        svg_lines.append('  onmouseout="hideTooltip()" ')
        onclick_str = (
            f"  onclick=\"selectVolume(evt, '{name_escaped}', '{desc_escaped}', "
            f"'{mat_escaped}', '{mother_escaped}', '{daughters_escaped}', "
            f"'{z_json}', '{r_outer_json}', '{r_inner_json}')\">")
        svg_lines.append(onclick_str)
        svg_lines.append(f'  <title>{vol["name"]}: {vol["desc"]} ({vol["material"]})</title>')
        svg_lines.append('</polygon>')

    # Draw outer border overlays for selected detector components.
    svg_lines.append('')
    svg_lines.append('<!-- Component outer border overlays -->')
    for outline in extra_outlines:
        z_vals = outline.get('z', [])
        r_vals = outline.get('r_outer', [])
        if not z_vals or not r_vals or len(z_vals) != len(r_vals):
            continue

        color = outline.get('color', '#000000')
        name = outline.get('name', 'component')

        points_upper = ' '.join(f"{z_to_x(z)},{r_to_y(r)}" for z, r in zip(z_vals, r_vals))
        points_lower = ' '.join(f"{z_to_x(z)},{r_to_y(-r)}" for z, r in zip(z_vals, r_vals))

        svg_lines.append(
            f'<polyline points="{points_upper}" class="component-outline" '
            f'stroke="{color}"><title>{name} outer border</title></polyline>'
        )
        svg_lines.append(
            f'<polyline points="{points_lower}" class="component-outline" '
            f'stroke="{color}"><title>{name} outer border (mirrored)</title></polyline>'
        )

        z_inner = outline.get('z_inner', [])
        r_inner = outline.get('r_inner', [])
        if z_inner and r_inner and len(z_inner) == len(r_inner):
            points_inner_up = ' '.join(f"{z_to_x(z)},{r_to_y(r)}" for z, r in zip(z_inner, r_inner))
            points_inner_dn = ' '.join(f"{z_to_x(z)},{r_to_y(-r)}" for z, r in zip(z_inner, r_inner))
            svg_lines.append(
                f'<polyline points="{points_inner_up}" class="component-outline-inner" '
                f'stroke="{color}"><title>{name} inner border</title></polyline>'
            )
            svg_lines.append(
                f'<polyline points="{points_inner_dn}" class="component-outline-inner" '
                f'stroke="{color}"><title>{name} inner border (mirrored)</title></polyline>'
            )

        # Lower half with rotation
        points_lower = []
        for i in range(len(vol['z'])):
            z = vol['z'][i]
            r_out = vol['r_outer'][i] if i < len(vol['r_outer']) else vol['r_outer'][-1]
            z_rot, r_rot = rotate_coordinates(z, -r_out, ler_angle)
            points_lower.append(f"{z_to_x(z_rot)},{r_to_y(r_rot)}")

        for i in range(len(vol['z'])-1, -1, -1):
            z = vol['z'][i]
            r_in = vol['r_inner'][i] if i < len(vol['r_inner']) else 0
            z_rot, r_rot = rotate_coordinates(z, -r_in, ler_angle)
            points_lower.append(f"{z_to_x(z_rot)},{r_to_y(r_rot)}")

        svg_lines.append(
            f'<polygon points="{" ".join(points_lower)}" class="volume" '
            f'fill="{color}" opacity="0.7" ')
        svg_lines.append(f'  data-volume-name="{name_escaped}" ')
        svg_lines.append(f'  data-material="{mat_escaped}" ')
        svg_lines.append(onmousemove_str)
        svg_lines.append('  onmouseout="hideTooltip()" ')
        svg_lines.append(onclick_str)
        svg_lines.append(f'  <title>{vol["name"]}: {vol["desc"]} ({vol["material"]})</title>')
        svg_lines.append('</polygon>')

    # Add legend
    legend_x = width - margin_right + 10
    legend_y = margin_top + 50
    svg_lines.append('')
    svg_lines.append('<!-- Legend -->')
    svg_lines.append(
        f'<text x="{legend_x}" y="{legend_y-20}" class="label" font-weight="bold">Materials (click to highlight):</text>')

    materials_used = set(v['material'] for v in all_volumes if v['material'])
    for i, mat in enumerate(sorted(materials_used)):
        if not mat:
            continue
        color = get_color_for_material(mat)
        mat_escaped = mat.replace("'", "\\'").replace('"', '&quot;')
        y = legend_y + i * 18
        if y < height - margin_bottom:
            # Create a group for each legend item to make it clickable
            svg_lines.append(f'<g onclick="selectMaterial(\'{mat_escaped}\')" style="cursor: pointer;">')
            svg_lines.append(
                f'  <rect x="{legend_x}" y="{y-10}" width="15" height="12" fill="{color}" stroke="#333" stroke-width="0.5"/>')
            svg_lines.append(f'  <text x="{legend_x+20}" y="{y}" class="label">{mat[:20]}</text>')
            svg_lines.append(f'  <title>Click to highlight all {mat} volumes</title>')
            svg_lines.append('</g>')

    # Border overlay legend
    overlay_legend_y = legend_y + len(materials_used) * 18 + 25
    svg_lines.append(
        f'<text x="{legend_x}" y="{overlay_legend_y}" class="label" font-weight="bold">Component borders:</text>'
    )
    y = overlay_legend_y + 16
    for outline in extra_outlines:
        svg_lines.append(
            f'<line x1="{legend_x}" y1="{y}" x2="{legend_x + 18}" y2="{y}" '
            f'stroke="{outline.get("color", "#000")}" stroke-width="2"/>'
        )
        svg_lines.append(
            f'<text x="{legend_x + 22}" y="{y + 3}" class="label">{outline.get("name", "component")}</text>'
        )
        y += 14

    # Add volume list
    svg_lines.append('')
    svg_lines.append('<!-- Volume List -->')
    volume_list_y = y + 20
    svg_lines.append(
        f'<text x="{legend_x}" y="{volume_list_y-10}" class="label" '
        'font-weight="bold">Volumes (click for details):</text>')

    # Sort volumes by name for better navigation
    sorted_volumes = sorted(all_volumes, key=lambda v: v['name'])

    # Use multiple columns to fit all volumes
    max_rows = 40  # Maximum rows per column
    column_width = 90  # Width of each column
    col = 0
    row = 0

    for i, vol in enumerate(sorted_volumes):
        vol_name = vol['name']
        vol_name_escaped = vol_name.replace("'", "\\'").replace('"', '&quot;')
        vol_desc_escaped = vol['desc'].replace("'", "\\'").replace('"', '&quot;')
        vol_mat_escaped = vol['material'].replace("'", "\\'").replace('"', '&quot;')
        mother_escaped = vol.get("mother", "").replace("'", "\\'").replace('"', '&quot;')
        daughters_str = ", ".join(vol.get("daughters", []))
        daughters_escaped = daughters_str.replace("'", "\\'").replace('"', '&quot;')

        # Serialize coordinate arrays as JSON for JavaScript
        z_json = json_module.dumps(vol['z'])
        r_outer_json = json_module.dumps(vol['r_outer'])
        r_inner_json = json_module.dumps(vol['r_inner'])

        # Calculate position with columns
        x = legend_x + col * column_width
        y = volume_list_y + row * 12

        # Create clickable text for each volume
        onclick_attr = (
            f"onclick=\"selectVolume(event, '{vol_name_escaped}', "
            f"'{vol_desc_escaped}', '{vol_mat_escaped}', '{mother_escaped}', "
            f"'{daughters_escaped}', '{z_json}', '{r_outer_json}', '{r_inner_json}')\"")
        svg_lines.append(
            f'<text x="{x}" y="{y}" class="label" '
            f'style="cursor: pointer; fill: #0066cc; font-size: 9px;" '
            f'{onclick_attr}>')
        svg_lines.append(f'  {vol_name[:12]}')
        svg_lines.append(f'  <title>{vol_name}: {vol["desc"]} ({vol["material"]})</title>')
        svg_lines.append('</text>')

        # Move to next row, or next column if max rows reached
        row += 1
        if row >= max_rows:
            row = 0
            col += 1

    svg_lines.append('</svg>')

    # Write to file
    with open(output_file, 'w') as f:
        f.write('\n'.join(svg_lines))

    print(f"SVG file generated: {output_file}")
    print(f"Z range: {z_min:.1f} to {z_max:.1f} cm")
    print(f"R range: {-r_max:.1f} to {r_max:.1f} cm")
    print(f"Number of volumes: {len(all_volumes)} (C: {len(volumes_c)}, "
          f"A: {len(volumes_a)}, B: {len(volumes_b)})")


def main():
    """
    Main function to generate interactive SVG visualization of Belle II Cryostat.

    Parses the Cryostat.xml geometry file and generates an interactive SVG
    visualization showing a radial cross-section (R-Z view) with the following features:
    - Hover tooltips showing component details and vertex coordinates
    - Click-to-activate volumes with detailed information panel
    - Material legend with click-to-highlight functionality
    - Volume hierarchy display (mother/daughter relationships)

    Command line arguments:
        argv[1]: Path to Cryostat.xml file (default: 'ir/data/Cryostat.xml')
        argv[2]: Output SVG file path (default: 'cryostat_cross_section.svg')

    Returns:
        int: 0 on success, 1 on error
    """
    xml_file = 'ir/data/Cryostat.xml'
    if len(sys.argv) > 1:
        xml_file = sys.argv[1]

    output_file = 'cryostat_cross_section.svg'
    if len(sys.argv) > 2:
        output_file = sys.argv[2]

    print(f"Parsing {xml_file}...")

    try:
        tree = ET.parse(xml_file)
        root = tree.getroot()
    except Exception as e:
        print(f"Error parsing XML: {e}")
        return 1

    # Find Content element
    content = root.find('.//Content')
    if content is None:
        print("No Content element found")
        return 1

    # Parse CrossingAngle
    crossing_angle = {'HER': 0.0, 'LER': 0.0}
    crossing_elem = content.find('CrossingAngle')
    if crossing_elem is not None:
        for sec in crossing_elem.findall('sec'):
            name = sec.get('name')
            unit = sec.get('unit', 'rad')
            value = float(sec.text)
            if unit == 'rad':
                crossing_angle[name] = value
            print(f"  CrossingAngle {name}: {value} {unit}")

    # Parse additional component outlines.
    repo_root = Path(__file__).resolve().parents[2]
    extra_outlines = []

    volumes_a = []  # HER (High Energy Ring) - positive angle
    volumes_b = []  # LER (Low Energy Ring) - negative angle
    volumes_c = []  # Central region - no rotation

    # Parse all volume elements
    for element in content:
        # Skip certain elements
        if element.tag in ['LimitStepLength', 'CrossingAngle']:
            continue

        # Check if it's a volume with polycone geometry
        n_elem = element.find('N')
        if n_elem is not None:
            vol = parse_polycone(element)
            if vol:
                # Categorize volumes by name prefix
                vol_name = vol['name']
                if vol_name.startswith('A'):
                    volumes_a.append(vol)
                elif vol_name.startswith('B'):
                    volumes_b.append(vol)
                else:
                    volumes_c.append(vol)
                print(f"  Parsed: {vol['name']} ({vol['n_planes']} planes)")

    # Parse VTX structures using visualize_vtx imported logic
    try:
        sys.path.append(str(Path(__file__).resolve().parent))
        import visualize_vtx
        vtx_comp = repo_root / 'vtx/data/VTX-Components-CMOS5.xml'
        sensors, ladder_types = visualize_vtx.parse_vtx_components(vtx_comp)

        import math
        vtx_shapes = []
        for layer, lad in ladder_types.items():
            radius = lad['radius'] / 10.0  # cm
            shift = lad['shift'] / 10.0    # cm

            for sinfo in lad['sensors']:
                stype = sinfo['type']
                z_center = sinfo['z'] / 10.0  # cm

                sdef = sensors.get(stype)
                if not sdef:
                    continue

                w = sdef['width'] / 10.0
                h = sdef['height'] / 10.0
                length_cm = sdef['length'] / 10.0

                lx = [shift - w/2, shift + w/2, shift + w/2, shift - w/2]
                ly = [radius + h/2, radius + h/2, radius - h/2, radius - h/2]
                rs = [math.sqrt(x**2 + y**2) for x, y in zip(lx, ly)]
                r_min = min(rs)
                r_max = max(rs)

                z_min_local = z_center - length_cm/2
                z_max_local = z_center + length_cm/2

                vtx_shapes.append({
                    'name': f"VTX Layer {layer} {stype}",
                    'desc': f"VTX Sensor (Dim: {w*10:.1f}Wx{h*10:.2f}Hx{length_cm*10:.1f}L mm)",
                    'material': sdef['material'],
                    'mother': '',
                    'daughters': [],
                    'z': [z_min_local, z_max_local],
                    'r_outer': [r_max, r_max],
                    'r_inner': [r_min, r_min],
                    'override_color': sdef['color']
                })
        print(f"  Parsed VTX geometries natively: {len(vtx_shapes)} shapes")
        volumes_c.extend(vtx_shapes)
    except Exception as e:
        print(f"  Warning: Detailed VTX geometries not loaded natively: {e}")

    # Combine all volumes for processing
    all_volumes = volumes_c + volumes_a + volumes_b

    if not all_volumes:
        print("No volumes found!")
        return 1

    # Build daughter volume relationships
    for vol in all_volumes:
        vol['daughters'] = []
        # Find all volumes that have this volume as mother
        for other_vol in all_volumes:
            if other_vol.get('mother') == vol['name']:
                vol['daughters'].append(other_vol['name'])

    print(f"\nGenerating SVG with {len(all_volumes)} volumes...")
    print(
        f"  C volumes: {len(volumes_c)}, A volumes (HER, +{crossing_angle['HER']:.4f} rad): "
        f"{len(volumes_a)}, B volumes (LER, {crossing_angle['LER']:.4f} rad): {len(volumes_b)}"
    )
    # Build extra outlines
    extra_outlines = []

    # Add beampipe
    beampipe_outline = parse_beampipe_outline("ir/data/BeamPipe.xml", "#ff0000")
    print("Beampipe:", beampipe_outline)
    if beampipe_outline:
        extra_outlines.append(beampipe_outline)

    # (optional) add other components if you want
    # cryostat_outline = parse_envelope_outline(...)
    # extra_outlines.append(cryostat_outline)

    generate_svg(
        volumes_c,
        volumes_a,
        volumes_b,
        crossing_angle,
        output_file=output_file,
        extra_outlines=extra_outlines
    )
    return 0


if __name__ == '__main__':
    sys.exit(main())
