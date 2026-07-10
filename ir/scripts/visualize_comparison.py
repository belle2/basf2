#!/usr/bin/env python3
"""
Generate comparison plots.

Outputs:
    vtx_overlay_zoom.svg  (Both envelopes)
    vtx_before_zoom.svg   (Original only)
    vtx_after_zoom.svg    (Updated only)
"""

import sys
from pathlib import Path
import xml.etree.ElementTree as ET

# Make helpers importable
_script_dir = Path(__file__).resolve().parent
sys.path.insert(0, str(_script_dir))


def main():
    from visualize_run2 import parse_polycone, generate_svg
    import visualize_run2_helpers

    repo_root = Path(__file__).resolve().parents[2]
    xml_file = repo_root / 'ir/data/Cryostat.xml'

    # ---------- Parse Cryostat XML ----------
    print(f"Parsing {xml_file}...")
    tree = ET.parse(xml_file)
    root = tree.getroot()
    content = root.find('.//Content')

    crossing_angle = {'HER': 0.0415, 'LER': -0.0415}

    volumes_base = []
    for element in content:
        if element.tag in ['LimitStepLength', 'CrossingAngle']:
            continue
        if element.find('N') is not None:
            vol = parse_polycone(element)
            if vol:
                if vol['name'].startswith('TubeR'):
                    continue
                vol['is_filled'] = True
                vol['is_outline'] = False
                volumes_base.append(vol)

    # ---------- Parse VTX geometries ----------
    try:
        vtx_shapes = visualize_run2_helpers.get_tracker_geometries(
            repo_root, 'VTX', comp_xml_name='VTX-Components-5layer-2025-baseline.xml')
        for s in vtx_shapes:
            s['override_color'] = '#c0c'  # Magenta
            s['is_filled'] = True
            s['is_outline'] = False
        volumes_base.extend(vtx_shapes)
    except Exception as e:
        print(f"  Warning: VTX geometries not loaded: {e}")

    # ---------- Load BOTH Envelopes ----------
    env_before = visualize_run2_helpers.get_envelope_geometries(repo_root, 'VTX', filename='VTX-Envelope-before.xml')
    env_after = visualize_run2_helpers.get_envelope_geometries(repo_root, 'VTX', filename='VTX-Envelope.xml')

    if not env_before:
        print("ERROR: VTX-Envelope-before.xml not found or empty!")
    else:
        print(f"Successfully loaded 'before' envelope: {len(env_before)} shapes")

    for s in env_before:
        # We set both color keys to be safe, depending on how generate_svg is written
        s['line_color'] = '#ff0000'
        s['line_dash'] = ''
        s['line_width'] = 5
        s['opacity'] = 1.0
        s['is_outline'] = True
        s['is_filled'] = False
        s['name'] = 'VTX Envelope (ORIGINAL)'
        if 'override_color' in s:
            del s['override_color']
        if 'color' in s:
            del s['color']

    for s in env_after:
        s['line_color'] = '#ff0000'
        s['line_dash'] = ''
        s['line_width'] = 5
        s['opacity'] = 1.0
        s['is_outline'] = True
        s['is_filled'] = False
        s['name'] = 'VTX Envelope (UPDATED)'
        if 'override_color' in s:
            del s['override_color']
        if 'color' in s:
            del s['color']
    # ---------- Generation Logic ----------

    def generate_plot(vols, title, outfile, envelope_type='both', z_bounds=None, r_max=None):
        # We pass all volumes to generate_svg and use custom bounds for zooming
        generate_svg(vols, [], [], crossing_angle, output_file=outfile,
                     custom_z_bounds=z_bounds, custom_r_max=r_max)

        with open(outfile, 'r') as f:
            lines = f.readlines()

        # Determine legend height
        legend_height = 100 if envelope_type != 'both' else 130

        # Insert legend
        legend_lines = [
            f'  <text x="950" y="40" class="title" text-anchor="middle" font-size="28">{title}</text>',
            '  <g transform="translate(100, 60)">',
            f'    <rect x="0" y="0" width="320" height="{legend_height}" '
            'fill="white" stroke="black" stroke-width="1" fill-opacity="0.9"/>',
            '    <text x="10" y="25" font-family="Arial" font-size="16" font-weight="bold">Legend:</text>',
            '    <rect x="10" y="40" width="35" height="12" fill="#ddd" stroke="#333" stroke-width="0.5"/>',
            '    <text x="55" y="50" font-family="Arial" font-size="14">Cryostat Wall</text>',
            '    <rect x="10" y="60" width="35" height="12" fill="#c0c"/>',
            '    <text x="55" y="72" font-family="Arial" font-size="14">VTX Internal Components</text>']

        if envelope_type in ['before', 'both']:
            y_pos = 85
            legend_lines.append(f'    <line x1="10" y1="{y_pos}" x2="45" y2="{y_pos}" stroke="#ff0000" stroke-width="5"/>')
            legend_lines.append(
                f'    <text x="55" y="{y_pos+5}" font-family="Arial" font-size="14">Original Envelope (Overlap)</text>')
        if envelope_type in ['after', 'both']:
            y_pos = 105 if envelope_type == 'both' else 85
            legend_lines.append(f'    <line x1="10" y1="{y_pos}" x2="45" y2="{y_pos}" stroke="#ff0000" stroke-width="5"/>')
            legend_lines.append(
                f'    <text x="55" y="{y_pos+5}" font-family="Arial" font-size="14">Updated Envelope (Clearance)</text>')

        legend_lines.append('  </g>')

        for i in range(len(lines)-1, -1, -1):
            if '</svg>' in lines[i]:
                lines[i:i] = [line + '\n' for line in legend_lines]
                break

        with open(outfile, 'w') as f:
            f.writelines(lines)

    print("\\nGenerating Separate Zoomed Plots (Z: -50 to 100, R: 0 to 30)...")
    generate_plot(volumes_base + env_before,
                  "VTX Envelope - BEFORE FIX",
                  "vtx_before.svg",
                  envelope_type='before',
                  z_bounds=(-50.0, 100.0),
                  r_max=30.0)

    generate_plot(volumes_base + env_after,
                  "VTX Envelope - AFTER FIX",
                  "vtx_after.svg",
                  envelope_type='after',
                  z_bounds=(-50.0, 100.0),
                  r_max=30.0)

    return 0


if __name__ == '__main__':
    sys.exit(main())
