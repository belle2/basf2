#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import xml.etree.ElementTree as ET
import math
import json as json_module


def get_hms_geometries(repo_root):
    hms_comp = repo_root / 'vxd/data/HeavyMetalShield.xml'
    if not hms_comp.exists():
        return []

    tree = ET.parse(hms_comp)
    root = tree.getroot()
    content = root.find('Content')

    shapes = []
    for shield in content.findall('Shield'):
        name = shield.get('name', 'HMS_Shield')
        desc = shield.get('description', '')
        mat = shield.find('Material').text if shield.find('Material') is not None else 'W'

        planes = shield.findall('Plane')
        z_vals = []
        r_out = []
        r_in = []
        for p in planes:
            # posZ
            z_el = p.find('posZ')
            z_val = float(z_el.text) * (0.1 if z_el.get('unit', 'mm') == 'mm' else 1.0)
            z_vals.append(z_val)
            # outerRadius
            ro_el = p.find('outerRadius')
            ro_val = float(ro_el.text) * (0.1 if ro_el.get('unit', 'mm') == 'mm' else 1.0)
            r_out.append(ro_val)
            # innerRadius
            ri_el = p.find('innerRadius')
            ri_val = float(ri_el.text) * (0.1 if ri_el.get('unit', 'mm') == 'mm' else 1.0)
            r_in.append(ri_val)

        if not z_vals:
            continue

        shapes.append({
            'name': f"HMS_{name}",
            'desc': desc,
            'material': mat,
            'z': z_vals,
            'r_outer': r_out,
            'r_inner': r_in,
            'override_color': '#808080'  # W material color
        })
    return shapes


def get_tracker_geometries(repo_root, system, comp_xml_name=None):
    sys_lower = system.lower()
    if comp_xml_name:
        comp_xml = repo_root / f'{sys_lower}/data/{comp_xml_name}'
    else:
        comp_xml = repo_root / f'{sys_lower}/data/{system}-Components.xml'

    # For VTX the default files use a variant suffix; try common alternatives
    if not comp_xml.exists():
        for suffix in ['CMOS5', 'CMOS6', '5layer-2025-baseline']:
            alt = repo_root / f'{sys_lower}/data/{system}-Components-{suffix}.xml'
            if alt.exists():
                comp_xml = alt
                break

    if not comp_xml.exists():
        return []

    tree = ET.parse(comp_xml)
    root = tree.getroot()

    # Parse SensorBases for defaults (SVD specific)
    sensor_bases = {}
    for sb in root.findall('SensorBase'):
        sb_type = sb.get('type')
        if sb_type:
            sensor_bases[sb_type] = sb

    # Globally defined Components
    global_comps = {}
    for c in root.findall('Component'):
        c_name = c.get('name')
        if c_name:
            global_comps[c_name] = c

    sensors = {}
    sensor_subcomps = {}
    for sdef in root.findall('Sensor'):
        stype = sdef.get('type')
        width_el = sdef.find('width')
        length_el = sdef.find('length')
        height_el = sdef.find('height')
        color_el = sdef.find('Color')
        mat_el = sdef.find('Material')

        # Try to resolve missing info from SensorBase
        sbase_type = None
        for xi in sdef.findall('.//{http://www.w3.org/2001/XInclude}include'):
            if 'SensorBase' in xi.get('xpointer', ''):
                if 'Slanted' in xi.get('xpointer', ''):
                    sbase_type = 'Slanted'
                if 'Barrel' in xi.get('xpointer', ''):
                    sbase_type = 'Barrel'

        if sbase_type and sbase_type in sensor_bases:
            sb = sensor_bases[sbase_type]
            if width_el is None:
                width_el = sb.find('width')
            if length_el is None:
                length_el = sb.find('length')
            if height_el is None:
                height_el = sb.find('height')
            if color_el is None:
                color_el = sb.find('Color')
            if mat_el is None:
                mat_el = sb.find('Material')

        if width_el is None or length_el is None or height_el is None:
            continue

        height_scale = 1.0 if height_el.get('unit') == 'mm' else 0.001 if height_el.get('unit') == 'um' else 0.1

        sensors[stype] = {
            'width': float(width_el.text) * (1.0 if width_el.get('unit') == 'mm' else 0.1),
            'length': float(length_el.text) * (1.0 if length_el.get('unit') == 'mm' else 0.1),
            'height': float(height_el.text) * height_scale,
            'color': color_el.text if color_el is not None else '#006699',
            'material': mat_el.text if mat_el is not None else 'Si',
            'slanted': sdef.get('slanted', 'false').lower() == 'true'
        }

        # Parse subcomponents placed on this sensor
        subcomps = []
        for cplace in sdef.findall('Component'):
            cname = cplace.get('name')
            ctype = cplace.get('type')
            target_c = cplace
            # Check global comps if it uses type=... instead of inline name=...
            if ctype and ctype in global_comps:
                # Merge placement and global attributes (target_c has priority for placement)
                target_c = global_comps[ctype]

            v_els = cplace.findall('v')
            w_els = cplace.findall('w')
            woffset_el = cplace.find('woffset')

            c_width_el = target_c.find('width')
            c_length_el = target_c.find('length')
            c_height_el = target_c.find('height')
            c_color_el = target_c.find('Color')
            c_mat_el = target_c.find('Material')

            # Look in cplace if not found in target_c
            if c_width_el is None:
                c_width_el = cplace.find('width')
            if c_length_el is None:
                c_length_el = cplace.find('length')
            if c_height_el is None:
                c_height_el = cplace.find('height')
            if c_color_el is None:
                c_color_el = cplace.find('Color')
            if c_mat_el is None:
                c_mat_el = cplace.find('Material')

            # Sometimes dimensions are inherited from xi:include in Component
            if c_width_el is None or c_length_el is None:
                continue

            c_w = float(c_width_el.text) * (1.0 if c_width_el.get('unit') == 'mm' else 0.1)
            c_l = float(c_length_el.text) * (1.0 if c_length_el.get('unit') == 'mm' else 0.1)
            if c_height_el is not None:
                c_height_scale = 1.0 if c_height_el.get('unit') == 'mm' else 0.001 if c_height_el.get('unit') == 'um' else 0.1
                c_h = float(c_height_el.text) * c_height_scale
            else:
                c_h = 0.2

            w_pos = w_els[0].text.strip() if w_els else 'above'
            woffset = float(woffset_el.text) * (1.0 if woffset_el.get('unit') == 'mm' else 0.1) if woffset_el is not None else 0.0

            c_color = c_color_el.text if c_color_el is not None else '#aaaaaa'
            c_mat = c_mat_el.text if c_mat_el is not None else 'Unknown'

            for v_el in v_els:
                v_val = float(v_el.text) * (1.0 if v_el.get('unit') == 'mm' else 0.1)
                subcomps.append({
                    'name': cname or ctype or 'SubComp',
                    'material': c_mat,
                    'color': c_color,
                    'w_pos': w_pos,
                    'woffset': woffset,
                    'v': v_val,
                    'width': c_w,
                    'length': c_l,
                    'height': c_h
                })

        sensor_subcomps[stype] = subcomps

    ladder_types = {}
    for ldef in root.findall('Ladder'):
        layer = ldef.get('layer')
        if not layer:
            continue

        shift_el = ldef.find('shift')
        rad_el = ldef.find('radius')
        slant_ang_el = ldef.find('slantedAngle')
        slant_rad_el = ldef.find('slantedRadius')

        ladder_types[layer] = {
            'shift': float(shift_el.text) if shift_el is not None else 0.0,
            'radius': float(rad_el.text) if rad_el is not None else 0.0,
            'slanted_angle': float(slant_ang_el.text) if slant_ang_el is not None else 0.0,
            'slanted_radius': float(slant_rad_el.text) if slant_rad_el is not None else 0.0,
            'sensors': []
        }

        for sens in ldef.findall('Sensor'):
            ladder_types[layer]['sensors'].append({
                'id': sens.get('id'),
                'type': sens.get('type'),
                'z': float(sens.text),
                'flipV': sens.get('flipV', 'false').lower() == 'true',
                'flipW': sens.get('flipW', 'false').lower() == 'true'
            })

    shapes = []

    def add_shape(
            sg_name,
            desc,
            mat,
            color,
            w,
            h,
            length_cm,
            local_x_centers,
            local_y_center,
            z_center,
            tilt_deg,
            shift,
            base_radius):
        '''Helper to compute corners and add to shapes array'''
        tilt_rad = math.radians(tilt_deg)
        cos_t = math.cos(tilt_rad)
        sin_t = math.sin(tilt_rad)

        # Local dimensions (Z is local z, R is local y perpendicular)
        local_z_corners = [-length_cm/2, length_cm/2, length_cm/2, -length_cm/2]
        local_r_corners = [h/2, h/2, -h/2, -h/2]

        z_vals = []
        r_vals = []
        for lz, lr in zip(local_z_corners, local_r_corners):
            rot_z = lz * cos_t - lr * sin_t
            rot_r = lz * sin_t + lr * cos_t
            final_z = z_center + rot_z

            y_coord = local_y_center + rot_r
            x_coord_min = shift - w/2
            x_coord_max = shift + w/2

            global_r_min = math.sqrt(min(abs(x_coord_min), abs(x_coord_max))**2 + y_coord**2)
            if x_coord_min < 0 and x_coord_max > 0:
                global_r_min = abs(y_coord)
            global_r_max = math.sqrt(max(abs(x_coord_min), abs(x_coord_max))**2 + y_coord**2)

            z_vals.append(final_z)
            r_vals.append((global_r_min, global_r_max))

        r_out = [r_vals[0][1], r_vals[1][1], r_vals[2][1], r_vals[3][1]]
        r_in = [r_vals[0][0], r_vals[1][0], r_vals[2][0], r_vals[3][0]]

        svg_z = [z_vals[0], z_vals[1]]
        svg_r_out = [r_out[0], r_out[1]]
        svg_r_in = [r_in[3], r_in[2]]

        shapes.append({
            'name': sg_name,
            'desc': desc,
            'material': mat,
            'mother': '',
            'daughters': [],
            'z': svg_z,
            'r_outer': svg_r_out,
            'r_inner': svg_r_in,
            'override_color': color
        })

    for layer, lad in ladder_types.items():
        base_radius = lad['radius'] / 10.0  # cm
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

            is_slanted = sdef['slanted']
            if is_slanted and lad['slanted_radius']:
                local_r = lad['slanted_radius'] / 10.0
                tilt_deg = -lad['slanted_angle']  # Tilt angle is inwards (negative)
            else:
                local_r = base_radius
                tilt_deg = 0.0

            # Add sensor active silicon
            add_shape(f"{system}_L{layer}_{stype}_{sinfo['id']}",
                      f"{system} Layer {layer} (Dim: {w*10:.1f}x{h*10:.2f}x{length_cm*10:.1f}mm)",
                      sdef['material'], sdef['color'],
                      w, h, length_cm, [0], local_r, z_center, tilt_deg, shift, base_radius)

            # Subcomponents
            subcomps = sensor_subcomps.get(stype, [])
            for i, sc in enumerate(subcomps):
                sc_w = sc['width'] / 10.0
                sc_h = sc['height'] / 10.0
                sc_l = sc['length'] / 10.0
                sc_v = sc['v'] / 10.0

                # Position logic
                # 'v' is relative to bottom edge and denotes the CENTER of the subcomponent
                local_z_offset = sc_v - length_cm/2
                if sinfo['flipV']:
                    local_z_offset = length_cm/2 - sc_v

                # R offset mapping
                w_pos = sc['w_pos']
                sc_woffset = sc['woffset'] / 10.0
                if w_pos == 'above' or w_pos == 'top':
                    local_y_offset = h/2 + sc_woffset + sc_h/2
                elif w_pos == 'below' or w_pos == 'bottom':
                    local_y_offset = -h/2 - sc_woffset - sc_h/2
                elif w_pos == 'center':
                    local_y_offset = 0.0
                else:
                    local_y_offset = 0.0

                if sinfo['flipW']:
                    local_y_offset = -local_y_offset

                # Apply tilt to offsets
                tilt_rad = math.radians(tilt_deg)
                cos_t = math.cos(tilt_rad)
                sin_t = math.sin(tilt_rad)
                global_z_center = z_center + local_z_offset * cos_t - local_y_offset * sin_t
                global_y_center = local_r + local_z_offset * sin_t + local_y_offset * cos_t

                # We reuse the tilt of the sensor for the subcomponent
                add_shape(f"{system}_L{layer}_{stype}_{sinfo['id']}_{sc['name']}_{i}",
                          f"{system} Layer {layer} Comp: {sc['name']}",
                          sc['material'], sc['color'],
                          sc_w, sc_h, sc_l, [0], global_y_center, global_z_center, tilt_deg, shift, base_radius)

    return shapes


def get_beampipe_geometries(repo_root):
    # Manually parsing the Beampipe specific structure as in GeoBeamPipeCreator.cc
    bp_comp = repo_root / 'ir/data/BeamPipe.xml'
    if not bp_comp.exists():
        return []

    tree = ET.parse(bp_comp)
    root = tree.getroot()
    content = root.find('Content')
    if content is None:
        return []

    shapes = []

    def get_sec(element, name):
        el = element.find(f"sec[@name='{name}']")
        if el is not None:
            return float(el.text) * (0.1 if el.get('unit', 'cm') == 'mm' else 1.0)
        return 0.0

    def get_color(mat):
        colors = {
            'Vacuum': '#e8f4f8', 'Ti': '#b8b8b8', 'Be': '#e0e0d0',
            'Paraffin': '#ffddaa', 'Au': '#ffd700', 'Ta': '#a0a0a0',
            'W': '#808080', 'Cu': '#d4a574'
        }
        return colors.get(mat, '#cccccc')

    # Lv1SUS
    lv1sus = content.find('Lv1SUS')
    if lv1sus is not None:
        L = [get_sec(lv1sus, f'L{i}') for i in range(1, 17)]
        R = [get_sec(lv1sus, f'R{i}') for i in range(1, 12)]
        mat = lv1sus.find('Material').text if lv1sus.find('Material') is not None else 'Ti'

        Zs = [0.0]
        for i in range(8):
            Zs[0] -= L[i]
        Zs.append(Zs[0] + L[0])
        Zs.append(Zs[1])
        Zs.append(Zs[2] + L[1])
        Zs.append(Zs[3] + L[2])
        Zs.append(Zs[4])
        Zs.append(Zs[5] + L[3])
        Zs.append(Zs[6] + L[4])
        Zs.append(Zs[7] + L[5])
        Zs.append(Zs[8] + L[6])
        Zs.append(Zs[9] + L[7])
        Zs.append(Zs[10] + L[8])
        Zs.append(Zs[11] + L[9])
        Zs.append(Zs[12] + L[10])
        Zs.append(Zs[13] + L[11])
        Zs.append(Zs[14] + L[12])
        Zs.append(Zs[15])
        Zs.append(Zs[16] + L[13])
        Zs.append(Zs[17] + L[14])
        Zs.append(Zs[18])
        Zs.append(Zs[19] + L[15])

        R_out = [R[0], R[0], R[1], R[1], R[2], R[3], R[3], R[4], R[4], R[5],
                 R[5], R[5], R[6], R[6], R[7], R[7], R[8], R[9], R[9], R[10], R[10]]

        shapes.append({
            'name': 'BeamPipe_Lv1SUS',
            'desc': 'Central IP Pipe',
            'material': mat, 'mother': '', 'daughters': [],
            'z': Zs[:len(R_out)],
            'r_outer': R_out,
            'r_inner': [0.0]*len(R_out),
            'override_color': get_color(mat)
        })

    # Lv2OutTi
    lv2outti = content.find('Lv2OutTi')
    if lv2outti is not None:
        mat = lv2outti.find('Material').text if lv2outti.find('Material') is not None else 'Ti'
        z = [-get_sec(lv2outti, 'L1'), get_sec(lv2outti, 'L2')]
        ro = [get_sec(lv2outti, 'R2'), get_sec(lv2outti, 'R2')]
        ri = [get_sec(lv2outti, 'R1'), get_sec(lv2outti, 'R1')]
        shapes.append({
            'name': 'BeamPipe_Lv2OutTi', 'desc': '', 'material': mat, 'mother': '', 'daughters': [],
            'z': z, 'r_outer': ro, 'r_inner': ri, 'override_color': '#333300'
        })

    # Lv2OutBe
    lv2outbe = content.find('Lv2OutBe')
    if lv2outbe is not None:
        mat = lv2outbe.find('Material').text if lv2outbe.find('Material') is not None else 'Be'
        z = [-get_sec(lv2outbe, 'L1'), get_sec(lv2outbe, 'L2')]
        ro = [get_sec(lv2outbe, 'R2'), get_sec(lv2outbe, 'R2')]
        ri = [get_sec(lv2outbe, 'R1'), get_sec(lv2outbe, 'R1')]
        shapes.append({
            'name': 'BeamPipe_Lv2OutBe', 'desc': '', 'material': mat, 'mother': '', 'daughters': [],
            'z': z, 'r_outer': ro, 'r_inner': ri, 'override_color': '#333300'
        })

    # Lv2InBe
    lv2inbe = content.find('Lv2InBe')
    if lv2inbe is not None:
        mat = lv2inbe.find('Material').text if lv2inbe.find('Material') is not None else 'Be'
        z = [-get_sec(lv2inbe, 'L1'), get_sec(lv2inbe, 'L2')]
        ro = [get_sec(lv2inbe, 'R2'), get_sec(lv2inbe, 'R2')]
        ri = [get_sec(lv2inbe, 'R1'), get_sec(lv2inbe, 'R1')]
        shapes.append({
            'name': 'BeamPipe_Lv2InBe', 'desc': '', 'material': mat, 'mother': '', 'daughters': [],
            'z': z, 'r_outer': ro, 'r_inner': ri, 'override_color': '#333300'
        })

    def parse_fwd_bwd(node_name, dir_sign=1.0):
        node = content.find(node_name)
        if node is not None:
            mat = node.find('Material').text if node.find('Material') is not None else 'Ta'
            L1 = get_sec(node, 'L1')
            aR1 = get_sec(node, 'aR1')
            aR2 = get_sec(node, 'aR2')
            aL1 = get_sec(node, 'aL1')
            aL2 = get_sec(node, 'aL2')
            aL3 = get_sec(node, 'aL3')
            D1 = get_sec(node, 'D1')

            aR = [aR1, aR1, aR2, aR2]

            if dir_sign > 0:
                aL = [-L1/2, -L1/2 + aL1, -L1/2 + aL1 + aL2, -L1/2 + aL1 + aL2 + aL3]
                z_shifted = [D1 + L1/2 + al for al in aL]
            else:
                # Fwd is dir_sign > 0. Bwd is < 0 but let's check code
                # +Lv1TaBwd_L1 / 2.0 - (Lv1TaBwd_aL1 + Lv1TaBwd_aL2 + Lv1TaBwd_aL3) in Bwd
                aL = [L1/2 - (aL1+aL2+aL3), L1/2 - (aL1+aL2), L1/2 - aL1, L1/2]
                aR = [aR2, aR2, aR1, aR1]
                z_shifted = [-D1 - L1/2 + al for al in aL]

            shapes.append({
                'name': f'BeamPipe_{node_name}', 'desc': '', 'material': mat, 'mother': '', 'daughters': [],
                'z': z_shifted, 'r_outer': aR, 'r_inner': [0.0, 0.0, 0.0, 0.0], 'override_color': get_color(mat)
            })

    parse_fwd_bwd('Lv1TaFwd', 1.0)
    parse_fwd_bwd('Lv1TaBwd', -1.0)

    return shapes


def get_support_geometries(repo_root, system):
    """Generic parser for RotationSolid shapes in *-Support.xml files (SVD, VTX, etc.)"""
    sys_lower = system.lower()
    support_xml = repo_root / f'{sys_lower}/data/{system}-Support.xml'
    if not support_xml.exists():
        return []
    try:
        tree = ET.parse(support_xml)
        root = tree.getroot()
    except Exception:
        return []

    shapes = []
    for rs in root.findall('.//RotationSolid'):
        name = rs.find('Name').text if rs.find('Name') is not None else 'Support'
        color = rs.find('Color').text if rs.find('Color') is not None else '#666'
        mat = rs.find('Material').text if rs.find('Material') is not None else 'Unknown'

        outer_pts = rs.find('OuterPoints')
        inner_pts = rs.find('InnerPoints')
        if outer_pts is None or inner_pts is None:
            continue

        z_out, r_out = [], []
        for pt in outer_pts.findall('point'):
            z_el = pt.find('z')
            x_el = pt.find('x')
            z_out.append(float(z_el.text) * (0.1 if z_el.get('unit', 'mm') == 'mm' else 1.0))
            r_out.append(float(x_el.text) * (0.1 if x_el.get('unit', 'mm') == 'mm' else 1.0))

        z_in, r_in = [], []
        for pt in inner_pts.findall('point'):
            z_el = pt.find('z')
            x_el = pt.find('x')
            z_in.append(float(z_el.text) * (0.1 if z_el.get('unit', 'mm') == 'mm' else 1.0))
            r_in.append(float(x_el.text) * (0.1 if x_el.get('unit', 'mm') == 'mm' else 1.0))

        if not z_out or not z_in:
            continue

        shapes.append({
            'name': f"{system}_Support_{name}",
            'desc': f"{system} support structure: {name}",
            'material': mat,
            'mother': '',
            'daughters': [],
            'z': z_out,
            'r_outer': r_out,
            'z_inner': z_in,
            'r_inner': r_in,
            'override_color': color
        })

    return shapes


def get_run2_geometries(repo_root):
    shapes = []
    shapes.extend(get_beampipe_geometries(repo_root))
    shapes.extend(get_hms_geometries(repo_root))
    shapes.extend(get_tracker_geometries(repo_root, 'PXD'))
    shapes.extend(get_tracker_geometries(repo_root, 'SVD'))
    shapes.extend(get_support_geometries(repo_root, 'SVD'))
    return shapes


def get_envelope_geometries(repo_root, system, filename=None):
    sys_lower = system.lower()
    if filename:
        env_xml = repo_root / sys_lower / 'data' / filename
    else:
        env_xml = repo_root / sys_lower / 'data' / f"{system}-Envelope.xml"

    if not env_xml.exists():
        return []

    tree = ET.parse(env_xml)
    root = tree.getroot()

    outer = root.find('OuterPoints')
    inner = root.find('InnerPoints')
    if outer is None:
        return []

    def parse_pts(elem):
        pts = []
        if elem is None:
            return pts
        for p in elem.findall('point'):
            z_el = p.find('z')
            x_el = p.find('x')
            unit = z_el.get('unit')
            scale = 0.1 if unit == 'mm' else 1.0
            z = float(z_el.text) * scale
            x = float(x_el.text) * scale
            pts.append((z, x))
        return pts

    outer_pts = parse_pts(outer)
    inner_pts = parse_pts(inner)

    if not outer_pts:
        return []

    all_z = sorted(list(set([p[0] for p in outer_pts] + [p[0] for p in inner_pts])))
    if not all_z:
        return []

    def interpolate(pts, z):
        if not pts:
            return 0.0
        if len(pts) == 1:
            return pts[0][1]
        for i in range(len(pts) - 1):
            z1, r1 = pts[i]
            z2, r2 = pts[i+1]
            if (z1 <= z <= z2) or (z2 <= z <= z1):
                if abs(z2 - z1) < 1e-6:
                    return max(r1, r2)
                return r1 + (r2 - r1) * (z - z1) / (z2 - z1)
        z_start = pts[0][0]
        z_end = pts[-1][0]
        if z_start < z_end:
            if z < z_start:
                return pts[0][1]
            return pts[-1][1]
        else:
            if z < z_end:
                return pts[-1][1]
            return pts[0][1]

    r_outer = [interpolate(outer_pts, z) for z in all_z]
    if inner_pts:
        r_inner = [interpolate(inner_pts, z) for z in all_z]
    else:
        r_inner = [0.0] * len(all_z)

    return [{
        'name': f"{system}_Envelope",
        'desc': f"{system} Envelope Boundary",
        'material': 'Vacuum',
        'z': all_z,
        'r_outer': r_outer,
        'r_inner': r_inner,
        'override_color': 'none',
        'is_outline': True,
        'line_color': '#00aa44',
        'line_dash': '5,5'
    }]


# --- Merged from visualize_cryostat.py ---

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
