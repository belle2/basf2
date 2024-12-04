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
    "pxd": {},
    "svd": {
        "svd_loadedOnFADC": [
            "SVDNoiseCalibrations",
            "SVDPedestalCalibrations",
            "SVDPulseShapeCalibrations",
            "SVDFADCMaskedStrips",
            "SVDGlobalConfigParameters",
            "SVDGlobalXMLFile.xml",
            "SVDLocalConfigParameters",
        ]
    },
    "cdc": {},
    "ecl": {},
    "top": {},
    "arich": {},
    "klm": {},
    "trigger": {},
    "tracking": {},
    "alignment": {},
    "beam": {},
}
