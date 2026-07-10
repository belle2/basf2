import sys
from pathlib import Path
# Make helpers importable
_script_dir = Path(__file__).resolve().parent
sys.path.insert(0, str(_script_dir))

import visualize_run2_helpers  # noqa: E402


def get_bounding_box(vol):
    """Return (z_min, z_max, r_min, r_max) for a volume shape."""
    z_min = min(vol['z'])
    z_max = max(vol['z'])
    # Handle inner/outer radius lists
    r_max = max(vol['r_outer'])
    r_min = min(vol['r_inner']) if 'r_inner' in vol and vol['r_inner'] else 0.0
    return (z_min, z_max, r_min, r_max)


def check_overlap(vol1, vol2):
    """Check if two volumes overlap in the R-Z plane using bounding boxes."""
    z1_min, z1_max, r1_min, r1_max = get_bounding_box(vol1)
    z2_min, z2_max, r2_min, r2_max = get_bounding_box(vol2)

    # Check Z overlap
    z_overlap = max(0, min(z1_max, z2_max) - max(z1_min, z2_min))
    # Check R overlap
    r_overlap = max(0, min(r1_max, r2_max) - max(r1_min, r2_min))

    if z_overlap > 1e-6 and r_overlap > 1e-6:
        return True, z_overlap, r_overlap
    return False, 0.0, 0.0


def main():
    repo_root = Path(__file__).resolve().parents[2]

    print("Loading Upgrade geometries for overlap check...")
    vtx_volumes = visualize_run2_helpers.get_tracker_geometries(repo_root, 'VTX', comp_xml_name='VTX-Components-CMOS5.xml')
    vtx_support = visualize_run2_helpers.get_support_geometries(repo_root, 'VTX')
    ftl_volumes = visualize_run2_helpers.get_ftl_geometries(repo_root)
    # CDCReducedNoSL0SL1
    cdc_volumes = visualize_run2_helpers.get_cdc_geometries(
        repo_root,
        filename='CDCReducedNoSL0SL1-Covers.xml',
        main_xml='CDCReducedNoSL0SL1.xml'
    )

    all_vtx = vtx_volumes + vtx_support
    all_ftl = ftl_volumes

    print(f"Loaded {len(all_vtx)} VTX volumes and {len(all_ftl)} FTL volumes.")

    overlaps = []

    # Check VTX vs FTL
    print("\nChecking VTX vs FTL overlaps...")
    for vvol in all_vtx:
        for fvol in all_ftl:
            is_ov, zo, ro = check_overlap(vvol, fvol)
            if is_ov:
                overlaps.append((vvol['name'], fvol['name'], zo, ro))

    # Check VTX vs CDC
    print("Checking VTX vs CDC overlaps...")
    for vvol in all_vtx:
        for cvol in cdc_volumes:
            is_ov, zo, ro = check_overlap(vvol, cvol)
            if is_ov:
                overlaps.append((vvol['name'], cvol['name'], zo, ro))

    # Check FTL vs CDC
    print("Checking FTL vs CDC overlaps...")
    for fvol in all_ftl:
        for cvol in cdc_volumes:
            is_ov, zo, ro = check_overlap(fvol, cvol)
            if is_ov:
                overlaps.append((fvol['name'], cvol['name'], zo, ro))

    if not overlaps:
        print("\nNo overlaps found in the R-Z plane bounding boxes.")
    else:
        print(f"\nFound {len(overlaps)} potential overlaps:")
        for name1, name2, zo, ro in overlaps:
            print(f"  Overlap: {name1} with {name2} (Z-overlap: {zo:.2f}cm, R-overlap: {ro:.2f}cm)")


if __name__ == '__main__':
    main()
