"""
This script contains the mapping between the subsystems and the
calibration responsibles
"""

from collections import namedtuple

CalibResp = namedtuple("CalibrationResponsible", ["name", "gitlab_username"])

calibration_responsibles = {
    "pxd": CalibResp(name="Maiko Takahashi", gitlab_username="maiko.takahashi"),
    "svd": CalibResp(name="Giulio Dujany", gitlab_username="giulio.dujany"),
    "cdc": CalibResp(name="Van Thanh Dong", gitlab_username="dongthanh.ht"),
    "ecl": CalibResp(name="Christopher Hearty", gitlab_username="hearty"),
    "top": CalibResp(name="Shahab Kohani", gitlab_username="kohani"),
    "arich": CalibResp(name="Kristof Spenko", gitlab_username="kristof.spenko"),
    "klm": CalibResp(name="Chunhui Chen", gitlab_username="cchen23"),
    "trigger": CalibResp(name="Hideyuki Nakazawa", gitlab_username="nkzw"),
    "tracking": CalibResp(name="Giulia Casarosa", gitlab_username="giulia.casarosa"),
    "alignment": CalibResp(name="Tadeas Bilka", gitlab_username="tadeas.bilka"),
    "beam": CalibResp(name="Radek Zlebcik", gitlab_username="zlebcik"),
}
