"""
This script contains the settings for the local calibrations

for each subsystem there should be a dictionary with
{global_tag_name: [list, of, payloads, names]}

During the local calibration step for each global tag all the payloads from the list of
payloads will be copied and their iov restricted to match the one being
calibrated using the command

```bash
b2conditionsdb iovs copy --replace -rf "(payload1|payload2)" --run-range X Y Z K --set-run-range source_gt destination_gt
```
"""

#: Settings of local calibrations:
#: for each subsystem there should be a dictionary with
#: `{global_tag_name: [list, of, payloads, names]}`
settings_local_calibrations = {
    "pxd": {"local_staging_pxd": []},
    "svd": {
        "local_staging_svd": [
            "SVDNoiseCalibrations",
            "SVDPedestalCalibrations",
            "SVDPulseShapeCalibrations",
            "SVDFADCMaskedStrips",
            "SVDGlobalConfigParameters",
            "SVDGlobalXMLFile.xml",
            "SVDLocalConfigParameters",
        ]
    },
    "cdc": {"local_staging_cdc": ["CDCBadWires"]},
    "ecl": {
        "local_staging_ecl": ["ECLCrystalElectronics", "ECLCrystalElectronicsTime"]
    },
    "top": {
        "local_staging_top": [
            "TOPCalTimebase",
            "TOPCalChannelT0",
            "TOPCalChannelPulseHeight",
            "TOPCalChannelThresholdEff",
        ]
    },
    "arich": {"local_staging_arich": []},
    "klm": {"local_staging_klm": []},
    "trigger": {
        "local_staging_trigger": [
            "TRGGDLDBInputbits",
            "TRGGDLDBFTDLBits",
            "TRGGDLDBAlgs",
            "TRGGDLDBPrescales",
        ]
    },
    "tracking": {"local_staging_tracking": []},
    "alignment": {"local_staging_alignment": ["VXDAlignment", "CDCAlignment"]},
    "beam": {"local_staging_beam": []},
}
