#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Generate a plot showing VTX + beampipe shielding cross-section.

Correctly parses BeamPipe.xml volumes that use Z{i}/RI{i}/RO{i} convention
(no 'N' element), which the standard parse_polycone() cannot handle.
"""

import sys
from pathlib import Path
import xml.etree.ElementTree as ET

# Make helpers importable
_script_dir = Path(__file__).resolve().parent
sys.path.insert(0, str(_script_dir))


# ---------------------------------------------------------------------------
# Dedicated parser for BeamPipe.xml polycone volumes (Z{i}/RI{i}/RO{i} format)
# ---------------------------------------------------------------------------zzz

def parse_beampipe_volume(element):
    """Parse a BeamPipe.xml polycone that uses Z{i}, RI{i}, RO{i} (0-based or 1-based) format."""
    name = element.tag

    # Determine index starting point (0 or 1)
    if element.find("sec[@name='Z0']") is not None:
        start_idx = 0
    elif element.find("sec[@name='Z1']") is not None:
        start_idx = 1
    else:
        return None

    mat_elem = element.find('Material')
    material = mat_elem.text.strip() if mat_elem is not None and mat_elem.text else ''

    def get_sec(prefix, idx):
        el = element.find(f"sec[@name='{prefix}{idx}']")
        if el is None:
            return None
        unit = el.get('unit', 'cm')
        val = float(el.text)
        if unit == 'mm':
            val /= 10.0
        elif unit == 'm':
            val *= 100.0
        return val

    z_values = []
    r_inner = []
    r_outer = []
    i = start_idx
    while True:
        z = get_sec('Z', i)
        if z is None:
            break
        ri = get_sec('RI', i)
        ro = get_sec('RO', i)
        if ri is None:
            ri = 0.0
        if ro is None:
            break
        z_values.append(z)
        r_inner.append(ri)
        r_outer.append(ro)
        i += 1

    if not z_values:
        return None

    return {
        'name': name,
        'desc': '',
        'material': material,
        'z': z_values,
        'r_inner': r_inner,
        'r_outer': r_outer,
    }


def parse_lr_beampipe_volume(element):
    """Parse a BeamPipe.xml polycone that uses L{i}, R{i} (0-based or 1-based) format."""
    name = element.tag

    # Determine index starting point (0 or 1)
    if element.find("sec[@name='L0']") is not None:
        start_idx = 0
    elif element.find("sec[@name='L1']") is not None:
        start_idx = 1
    else:
        return None

    mat_elem = element.find('Material')
    material = mat_elem.text.strip() if mat_elem is not None and mat_elem.text else ''

    def get_sec(prefix, idx):
        el = element.find(f"sec[@name='{prefix}{idx}']")
        if el is None:
            return None
        unit = el.get('unit', 'cm')
        val = float(el.text)
        if unit == 'mm':
            val /= 10.0
        elif unit == 'm':
            val *= 100.0
        return val

    z_values = []
    r_outer = []
    i = start_idx
    while True:
        z = get_sec('L', i)
        if z is None:
            break
        ro = get_sec('R', i)
        if ro is None:
            break
        z_values.append(z)
        r_outer.append(ro)
        i += 1

    if not z_values:
        return None

    return {
        'name': name,
        'desc': '',
        'material': material,
        'z': z_values,
        'r_inner': [0.0] * len(z_values),
        'r_outer': r_outer,
    }


# ---------------------------------------------------------------------------
# CDC wire-region envelope builder
# ---------------------------------------------------------------------------

def build_cdc_envelope(cdc_xml, name, line_color, line_dash):
    """Build a realistic CDC wire-region envelope from superlayer data."""
    tree = ET.parse(cdc_xml)
    root = tree.getroot()
    content = root.find('Content')
    slayers = content.find('SLayers')
    if slayers is None:
        return []

    layers = []
    for sl in slayers.findall('SLayer'):
        r = float(sl.find('Radius').text) * 0.1   # mm -> cm
        bz = float(sl.find('BackwardZ').text) * 0.1
        fz = float(sl.find('ForwardZ').text) * 0.1
        layers.append((r, bz, fz))

    # Forward boundary (ascending R, at fz)
    fwd_z, fwd_r = [], []
    for i, (r, bz, fz) in enumerate(layers):
        if i > 0 and abs(fz - layers[i-1][2]) > 0.01:
            fwd_z.append(layers[i-1][2])
            fwd_r.append(r)
        fwd_z.append(fz)
        fwd_r.append(r)

    # Backward boundary (descending R, at bz)
    bwd_z, bwd_r = [], []
    for i in range(len(layers)-1, -1, -1):
        r, bz, fz = layers[i]
        if i < len(layers)-1 and abs(bz - layers[i+1][1]) > 0.01:
            bwd_z.append(layers[i+1][1])
            bwd_r.append(r)
        bwd_z.append(bz)
        bwd_r.append(r)

    z_combined = fwd_z + bwd_z + [fwd_z[0]]
    r_combined = fwd_r + bwd_r + [fwd_r[0]]

    return [{
        'name': name,
        'desc': 'CDC wire region envelope',
        'material': 'He/Ethane',
        'z': z_combined,
        'r_outer': r_combined,
        'r_inner': r_combined,
        'is_outline': True,
        'is_filled': False,
        'line_color': line_color,
        'line_dash': line_dash,
        'line_width': 2,
        'opacity': 1.0,
    }]


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    from visualize_run2_helpers import parse_polycone, generate_svg
    import visualize_run2_helpers

    repo_root = Path(__file__).resolve().parents[3]
    xml_file = repo_root / 'ir/data/Cryostat.xml'

    # ---------- Parse Cryostat XML ----------
    print(f"Parsing {xml_file}...")
    tree = ET.parse(xml_file)
    root = tree.getroot()
    content = root.find('.//Content')

    crossing_angle = {'HER': 0.0415, 'LER': -0.0415}

    volumes_c = []   # Central (on-axis) cryostat volumes
    volumes_a = []   # HER beam-pipe volumes (drawn at +crossing angle)
    volumes_b = []   # LER beam-pipe volumes (drawn at -crossing angle)
    shields = []

    for element in content:
        if element.tag in ['LimitStepLength', 'CrossingAngle']:
            continue
        if element.find('N') is not None:
            vol = parse_polycone(element)
            if vol:
                if vol['name'].startswith('TubeR') or vol['name'].startswith('TubeL'):
                    continue

                vol['is_filled'] = True
                vol['is_outline'] = False

                if 'QCSTungstenShield' in vol['name']:
                    if 'IP' in vol['name']:
                        print(f"  Skipping IP shield: {vol['name']}")
                        continue
                    else:
                        print(f"  Highlighting main shield: {vol['name']}")
                        vol['override_color'] = '#ff8c00'  # Orange
                        shields.append(vol)
                else:
                    # Route by name prefix so crossing angles are applied correctly
                    if vol['name'].startswith('A'):
                        volumes_a.append(vol)
                    elif vol['name'].startswith('B'):
                        volumes_b.append(vol)
                    else:
                        volumes_c.append(vol)
                # Cryostat
                if 'Cryostat' in vol['name']:
                    vol['is_filled'] = False
                    vol['line_color'] = 'gray'

                # Shields
                if 'QCSTungstenShield' in vol['name']:
                    vol['is_filled'] = True
                    vol['color'] = '#ff8c00'

    print(f"  Cryostat volumes: {len(volumes_c)} central, {len(volumes_a)} HER (A), {len(volumes_b)} LER (B)")

    # For the shielding plot we only need the on-axis C volumes (the cryostat outer
    # envelope) in volumes_base; A/B will be drawn separately with crossing angles.
    volumes_base = volumes_c

    # ---------- Parse BeamPipe XML ----------
    bp_xml_file = repo_root / 'ir/data/BeamPipe.xml'
    print(f"Parsing {bp_xml_file}...")
    bp_tree = ET.parse(bp_xml_file)
    bp_root = bp_tree.getroot()
    bp_content = bp_root.find('.//Content')

    bp_volumes = []
    bp_shield_volumes = []
    if bp_content is not None:
        for element in bp_content:
            tag = element.tag
            if tag in ['LimitStepLength', 'CrossingAngle', 'Safety']:
                continue

            vol = None

            # Use dedicated parser for Z{i}/RI{i}/RO{i} style (our new volumes)
            if element.find("sec[@name='Z0']") is not None or element.find("sec[@name='Z1']") is not None:
                vol = parse_beampipe_volume(element)
            # Use dedicated parser for L{i}/R{i} style
            elif element.find("sec[@name='L0']") is not None or element.find("sec[@name='L1']") is not None:
                vol = parse_lr_beampipe_volume(element)
            # Standard parse_polycone for L{i}/R{i}/r{i} style (legacy Lv* volumes)
            elif element.find('N') is not None:
                vol = parse_polycone(element)

            if vol is None:
                continue

            vol['is_filled'] = True
            vol['is_outline'] = False
            vol_material = str(vol.get('material', '')).strip().lower()

            if tag in ('AdditionalShield_FWD', 'AdditionalShield_BWD'):
                if vol_material == 'vacuum':
                    print(f"  Additional shield: {tag} is Vacuum -> skipped")
                    continue
                vol['override_color'] = '#4a4a8a'   # Dark slate-blue for Additional W shields
                bp_shield_volumes.append(vol)
                print(f"  Additional shield: {tag}, Z=[{vol['z'][0]:.1f},{vol['z'][-1]:.1f}] cm, "
                      f"RI=[{vol['r_inner'][0]:.1f},{vol['r_inner'][-1]:.1f}] cm, "
                      f"RO=[{vol['r_outer'][0]:.1f},{vol['r_outer'][-1]:.1f}] cm")
            elif tag in ('BellowsShield_FWD', 'BellowsShield_BWD'):
                if vol_material == 'vacuum':
                    print(f"  Bellows shield: {tag} is Vacuum -> skipped")
                    continue
                vol['override_color'] = '#008080'   # Teal for Bellows W shields
                bp_shield_volumes.append(vol)
                print(f"  Bellows shield: {tag}, Z=[{vol['z'][0]:.1f},{vol['z'][-1]:.1f}] cm, "
                      f"RI=[{vol['r_inner'][0]:.1f},{vol['r_inner'][-1]:.1f}] cm, "
                      f"RO=[{vol['r_outer'][0]:.1f},{vol['r_outer'][-1]:.1f}] cm")
            elif tag.startswith('IPBeamPipe') or tag.startswith('IPChamber'):
                vol['override_color'] = '#8B4513'   # SaddleBrown for Ta
                bp_volumes.append(vol)
                print(f"  IPBeamPipe/Chamber: {tag}, Z=[{vol['z'][0]:.1f},{vol['z'][-1]:.1f}] cm")
            elif tag.startswith('BellowsPipe'):
                vol['override_color'] = '#DAA520'   # GoldenRod for Cu
                bp_volumes.append(vol)
                print(f"  BellowsPipe: {tag}, Z=[{vol['z'][0]:.1f},{vol['z'][-1]:.1f}] cm")
            elif tag.startswith('Vac'):
                # Don't plot vacuum volumes – they clutter the view
                pass
            elif 'HER' in tag:
                # HER tilted beampipes (rotated +crossing_angle)
                volumes_a.append(vol)
                print(f"  HER Beampipe: {tag}, Z=[{vol['z'][0]:.1f},{vol['z'][-1]:.1f}] cm")
            elif 'LER' in tag:
                # LER tilted beampipes (rotated -crossing_angle)
                volumes_b.append(vol)
                print(f"  LER Beampipe: {tag}, Z=[{vol['z'][0]:.1f},{vol['z'][-1]:.1f}] cm")
            elif tag.startswith('Lv1') or tag.startswith('Lv2') or tag.startswith('Lv3'):
                # IP region legacy Lv-volumes: skip for clarity
                pass
            elif tag in ('Flange', 'PXDMountFwd', 'PXDMountBwd',
                         'PXDMountFixtureFwd', 'PXDMountFixtureBwd',
                         'AreaTubeFwd', 'AreaTubeBwd'):
                pass  # Support structures – skip for clarity

    print(f"  Parsed {len(volumes_a) + len(volumes_b) - 36} beampipe pipe volumes, "
          f"{len(bp_shield_volumes)} beampipe shield volumes")

    # ---------- Parse VTX geometries ----------
    print("Parsing VTX...")
    vtx_shapes = []
    try:
        vtx_shapes = visualize_run2_helpers.get_tracker_geometries(
            repo_root, 'VTX', comp_xml_name='VTX-Components-5layer-2025-baseline.xml')
        for s in vtx_shapes:
            s['override_color'] = '#c000c0'   # Magenta
            s['is_filled'] = True
            s['is_outline'] = False
    except Exception as e:
        print(f"  Warning: VTX geometries not loaded: {e}")

    # ---------- Load VTX Envelope ----------
    print("Parsing VTX Envelope...")
    env_vtx = []
    try:
        env_vtx = visualize_run2_helpers.get_envelope_geometries(
            repo_root, 'VTX', filename='VTX-Envelope.xml')
        for s in env_vtx:
            s['line_color'] = '#ff0000'
            s['line_dash'] = ''
            s['line_width'] = 3
            s['opacity'] = 1.0
            s['is_outline'] = True
            s['is_filled'] = False
            s['name'] = 'VTX Envelope'
    except Exception as e:
        print(f"  Warning: VTX Envelope not loaded: {e}")

    # ---------- Parse CDC geometries ----------
    print("Parsing CDC (full)...")
    cdc_shapes = []
    try:
        cdc_shapes = build_cdc_envelope(
            repo_root / 'cdc/data/CDC.xml',
            name='CDC_Full',
            line_color='#444',
            line_dash='4,4'
        )
        print(f"  Parsed full CDC envelope: {len(cdc_shapes)} shapes")
    except Exception as e:
        print(f"  Warning: Full CDC geometries not loaded: {e}")

    # ---------- Parse FTL geometries ----------
    # print("Parsing FTL...")
    ftl_shapes = []
    # try:
    #     ftl_shapes = visualize_run2_helpers.get_ftl_geometries(repo_root)
    #     for s in ftl_shapes:
    #         s['override_color'] = '#00cccc'   # Cyan
    #         s['is_filled'] = True
    #         s['is_outline'] = False
    #     print(f"  Parsed FTL geometries: {len(ftl_shapes)} shapes")
    # except Exception as e:
    #     print(f"  Warning: FTL geometries not loaded: {e}")

    # ---------- Generation ----------
    if len(sys.argv) > 1:
        outfile = sys.argv[1]
    else:
        outfile = str(Path(__file__).resolve().parent / "shielding_study_vtx_beampipe_shield_no_ip.svg")
    title = "VTX + BeamPipe Shielding Cross-Section (R-Z, no IP shield)"

    # # Define visual holes bored through the beampipes
    # front_hole_fwd = {
    #     'name': 'Front_Hole_FWD',
    #     'desc': 'FWD IP Beampipe entry chamber',
    #     'material': 'Vacuum',
    #     'z': [13.6, 13.95],
    #     'r_inner': [0.0, 0.0],
    #     'r_outer': [1.5, 1.5],
    #     'is_filled': True,
    #     'is_outline': False,
    #     'override_color': '#ffffff',
    #     'opacity': 1.0
    # }
    # front_hole_bwd = {
    #     'name': 'Front_Hole_BWD',
    #     'desc': 'BWD IP Beampipe entry chamber',
    #     'material': 'Vacuum',
    #     'z': [-9.18, -8.9],
    #     'r_inner': [0.0, 0.0],
    #     'r_outer': [1.03, 1.03],
    #     'is_filled': True,
    #     'is_outline': False,
    #     'override_color': '#ffffff',
    #     'opacity': 1.0
    # }
    # her_hole_fwd = {
    #     'name': 'HER_Hole_FWD',
    #     'desc': 'Outgoing HER beam channel',
    #     'material': 'Vacuum',
    #     'z': [13.95, 25.6],
    #     'r_inner': [0.0, 0.0],
    #     'r_outer': [1.6, 1.6],
    #     'is_filled': True,
    #     'is_outline': False,
    #     'override_color': '#ffffff',
    #     'opacity': 1.0
    # }
    # ler_hole_fwd = {
    #     'name': 'LER_Hole_FWD',
    #     'desc': 'Incoming LER beam channel',
    #     'material': 'Vacuum',
    #     'z': [13.95, 25.6],
    #     'r_inner': [0.0, 0.0],
    #     'r_outer': [1.0, 1.0],
    #     'is_filled': True,
    #     'is_outline': False,
    #     'override_color': '#ffffff',
    #     'opacity': 1.0
    # }
    # her_hole_bwd = {
    #     'name': 'HER_Hole_BWD',
    #     'desc': 'Incoming HER beam channel',
    #     'material': 'Vacuum',
    #     'z': [-25.6, -9.18],
    #     'r_inner': [0.0, 0.0],
    #     'r_outer': [1.6, 1.6],
    #     'is_filled': True,
    #     'is_outline': False,
    #     'override_color': '#ffffff',
    #     'opacity': 1.0
    # }
    # ler_hole_bwd = {
    #     'name': 'LER_Hole_BWD',
    #     'desc': 'Outgoing LER beam channel',
    #     'material': 'Vacuum',
    #     'z': [-25.6, -9.18],
    #     'r_inner': [0.0, 0.0],
    #     'r_outer': [1.0, 1.0],
    #     'is_filled': True,
    #     'is_outline': False,
    #     'override_color': '#ffffff',
    #     'opacity': 1.0
    # }

    # # Extend lists to overlay the holes on top
    # volumes_a_extended = list(volumes_a) + [her_hole_fwd, her_hole_bwd]
    # volumes_b_extended = list(volumes_b) + [ler_hole_fwd, ler_hole_bwd]

    # Order: cryostat walls first (central, HER, LER with crossing angles),
    # then beampipe walls, shields on top, VTX, FTL, CDC
    vols_c_all = (volumes_base + bp_volumes + bp_shield_volumes + shields
                  + vtx_shapes + env_vtx + ftl_shapes + cdc_shapes)

    generate_svg(vols_c_all, volumes_a, volumes_b, crossing_angle, output_file=outfile)

    # ---------- Post-process: add title + legend ----------
    with open(outfile, 'r') as f:
        lines = f.readlines()

    legend_items = [
        ('#4a4a8a', None,   'rect', 'Additional Tungsten Shield'),
        ('#008080', None,   'rect', 'Bellows Tungsten Shield'),
        ('#ff8c00', None,   'rect', 'QCS Tungsten Shield'),
        ('#8B4513', None,   'rect', 'IP BeamPipe (Ta)'),
        ('#DAA520', None,   'rect', 'Bellows Pipe (Cu)'),
        ('#c000c0', None,   'rect', 'VTX Components'),
        ('#ff0000', None,   'line', 'VTX Envelope'),
        ('#00cccc', None,   'rect', 'FTL Components'),
        ('#444',    None,   'dash', 'CDC Inner Boundary'),
        ('gray',    None,   'line', 'Cryostat Outline'),
    ]

    legend_h = 20 + len(legend_items) * 22 + 10
    legend_svg = [
        f'  <text x="950" y="40" class="title" text-anchor="middle" font-size="26">{title}</text>',
        '  <g transform="translate(50, 55)">',
        f'    <rect x="0" y="0" width="340" height="{legend_h}" fill="white" '
        f'stroke="#999" stroke-width="1" fill-opacity="0.92" rx="4"/>',
        '    <text x="10" y="18" font-family="Arial" font-size="14" font-weight="bold">Legend</text>',
    ]

    for idx, (fill, stroke, shape, label) in enumerate(legend_items):
        y = 28 + idx * 22
        if shape == 'rect':
            sw = f' stroke="{stroke}" stroke-width="0.5"' if stroke else ''
            legend_svg.append(
                f'    <rect x="10" y="{y}" width="32" height="12" fill="{fill}"{sw}/>')
        elif shape == 'line':
            legend_svg.append(
                f'    <line x1="10" y1="{y+6}" x2="42" y2="{y+6}" stroke="{fill}" stroke-width="3"/>')
        else:  # dash
            legend_svg.append(
                f'    <line x1="10" y1="{y+6}" x2="42" y2="{y+6}" stroke="#444" '
                f'stroke-width="2" stroke-dasharray="4,4"/>')
        legend_svg.append(
            f'    <text x="52" y="{y+10}" font-family="Arial" font-size="13">{label}</text>')

    legend_svg.append('  </g>')

    for i in range(len(lines) - 1, -1, -1):
        if '</svg>' in lines[i]:
            lines[i:i] = [legend_line + '\n' for legend_line in legend_svg]
            break

    with open(outfile, 'w') as f:
        f.writelines(lines)

    print(f"Successfully generated {outfile}")
    return 0


if __name__ == '__main__':
    sys.exit(main())
