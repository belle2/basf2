#!/usr/bin/env python3
"""
Generate an interactive SVG R-Z cross-section showing the Belle II VTX (Upgrade)
geometry overlaid on cryostat/IR structures.

Usage:
    python3 ir/scripts/visualize_vtx_cryostat.py ir/data/Cryostat.xml vtx_cryostat_cross_section.svg
"""

import sys
from pathlib import Path
import xml.etree.ElementTree as ET

# Make helpers importable
_script_dir = Path(__file__).resolve().parent
sys.path.insert(0, str(_script_dir))


def convert_to_cm(value, unit):
    """Convert value to cm based on unit."""
    if unit == 'mm':
        return float(value) / 10.0
    elif unit == 'cm':
        return float(value)
    elif unit == 'm':
        return float(value) * 100.0
    return float(value)


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


def main():
    from visualize_run2 import parse_polycone, generate_svg
    import visualize_run2_helpers

    xml_file = sys.argv[1] if len(sys.argv) > 1 else 'ir/data/Cryostat.xml'
    output_file = sys.argv[2] if len(sys.argv) > 2 else 'upgrade_inner_detector.svg'

    repo_root = Path(__file__).resolve().parents[2]

    # ---------- Parse Cryostat XML ----------
    print(f"Parsing {xml_file}...")
    try:
        tree = ET.parse(xml_file)
        root = tree.getroot()
    except Exception as e:
        print(f"Error parsing Cryostat XML: {e}")
        return 1

    content = root.find('.//Content')
    if content is None:
        print("No Content element found")
        return 1

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

    volumes_a, volumes_b, volumes_c = [], [], []
    for element in content:
        if element.tag in ['LimitStepLength', 'CrossingAngle']:
            continue
        if element.find('N') is not None:
            vol = parse_polycone(element)
            if vol:
                vol_name = vol['name']
                # Filter out HMS if it leaked into Cryostat.xml
                if 'HMS' in vol_name:
                    continue

                # Filter Tungsten Shields: only keep the 20mm version (baseline)
                if 'QCSTungstenShield' in vol_name and vol_name != 'QCSTungstenShield20mm':
                    print(f"  Skipping inactive shield: {vol_name}")
                    continue

                if vol_name.startswith('A'):
                    volumes_a.append(vol)
                elif vol_name.startswith('B'):
                    volumes_b.append(vol)
                else:
                    volumes_c.append(vol)
                print(f"  Parsed: {vol['name']} ({vol['n_planes']} planes)")

    # ---------- Parse VTX geometries ----------
    try:
        vtx_shapes = []
        vtx_shapes.extend(visualize_run2_helpers.get_tracker_geometries(
            repo_root, 'VTX', comp_xml_name='VTX-Components-5layer-2025-baseline.xml'))
        vtx_shapes.extend(visualize_run2_helpers.get_support_geometries(repo_root, 'VTX'))
        vtx_shapes.extend(visualize_run2_helpers.get_radiation_sensor_geometries(repo_root, 'VTX'))
        # Style VTX components
        for s in vtx_shapes:
            if 'RadSensor' not in s['name']:
                s['override_color'] = '#c0c'  # Magenta
            s['is_filled'] = True
        print(f"  Parsed VTX-CMOS-5layer geometries: {len(vtx_shapes)} shapes")
        volumes_c.extend(vtx_shapes)
    except Exception as e:
        print(f"  Warning: VTX geometries not loaded: {e}")

    # ---------- Parse FTL geometries ----------
    try:
        ftl_shapes = []
        ftl_shapes.extend(visualize_run2_helpers.get_ftl_geometries(repo_root))
        ftl_shapes.extend(visualize_run2_helpers.get_support_geometries(repo_root, 'FTL'))
        # Style FTL components distinctly
        for s in ftl_shapes:
            s['override_color'] = '#0cc'  # Cyan
            s['is_filled'] = True
        print(f"  Parsed FTL geometries: {len(ftl_shapes)} shapes")
        volumes_c.extend(ftl_shapes)
    except Exception as e:
        print(f"  Warning: FTL geometries not loaded: {e}")

    # ---------- Parse CDC geometries (Reduced) ----------
    try:
        cdc_shapes = []
        # Exclude BeamPipe and HMS as requested: excludedComponents=['PXD','BeamPipe', 'HeavyMetalShield', 'SVD', 'CDC']
        # Load CDCReducedNoSL0SL1 instead of standard CDC
        cdc_shapes.extend(visualize_run2_helpers.get_cdc_geometries(
            repo_root,
            filename='CDCReducedNoSL0SL1-Covers.xml',
            main_xml='CDCReducedNoSL0SL1.xml'
        ))
        # Just plot the wire region boundary (bottom boundary) clipped to [-60, 60]
        cdc_shapes = [s for s in cdc_shapes if s['name'] == 'CDC_WireRegion']
        for s in cdc_shapes:
            s['desc'] = "CDC Inner Boundary (Clipped)"
            s['line_color'] = '#444'  # Dark Gray
            # Hard-clip the Z range to [-60, 60] as requested
            s['z'] = [-60.0, 60.0]
            s['r_outer'] = [s['r_inner'][0], s['r_inner'][1]]  # Just draw the inner line
            s['r_inner'] = [s['r_inner'][0], s['r_inner'][1]]
            s['line_dash'] = '2,2'

        print(f"  Parsed Reduced CDC (clipped to [-60, 60]): {len(cdc_shapes)} shapes")
        volumes_c.extend(cdc_shapes)
    except Exception as e:
        print(f"  Warning: CDC geometries not loaded: {e}")

    # ---------- Envelope outlines ----------

    # VTX Envelope
    vtx_env_geoms = visualize_run2_helpers.get_envelope_geometries(repo_root, 'VTX')
    if vtx_env_geoms:
        for s in vtx_env_geoms:
            s['line_color'] = '#c0c'  # Match VTX Magenta
            s['line_dash'] = ''       # Solid line to avoid confusion
            s['name'] = 'VTX Envelope'
        volumes_c.extend(vtx_env_geoms)
        print("  Added VTX Envelope geometries.")

    # FTL Envelope (try to load if exists)
    ftl_env_geoms = visualize_run2_helpers.get_envelope_geometries(repo_root, 'FTL')
    if ftl_env_geoms:
        for s in ftl_env_geoms:
            s['line_color'] = '#0cc'  # Match FTL Cyan
            s['line_dash'] = ''       # Solid line
            s['name'] = 'FTL Envelope'
        volumes_c.extend(ftl_env_geoms)
        print("  Added FTL Envelope geometries.")

    # ---------- Daughter relationships ----------
    all_volumes = volumes_c + volumes_a + volumes_b
    if not all_volumes:
        print("No volumes found!")
        return 1

    for vol in all_volumes:
        vol['daughters'] = [o['name'] for o in all_volumes if o.get('mother') == vol['name']]

    print(f"\nGenerating SVG with {len(all_volumes)} volumes...")
    print(
        f"  C volumes: {len(volumes_c)}, "
        f"A volumes (HER, +{crossing_angle['HER']:.4f} rad): {len(volumes_a)}, "
        f"B volumes (LER, {crossing_angle['LER']:.4f} rad): {len(volumes_b)}"
    )

    generate_svg(volumes_c, volumes_a, volumes_b, crossing_angle,
                 output_file=output_file)
    return 0


if __name__ == '__main__':
    sys.exit(main())
