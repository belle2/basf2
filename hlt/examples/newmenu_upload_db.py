from ROOT import Belle2
from softwaretrigger import db_access
import basf2

HLTL0 = "[[nTracksLE>=3] and [Bhabha2Trk==0]]"
db_access.upload_cut_to_db(HLTL0, "hlt", "accept_hadron")

# HLTL1 = "[[nTracksLE>=2] and [Bhabha==0]]"
# db_access.upload_cut_to_db(HLTL1, "hlt", "accept_2_tracks")

# HLTL2 = "[[nTracksLE==1] and [Bhabha==0] and [PhiGTLE<2.793]]"
# db_access.upload_cut_to_db(HLTL2, "hlt", "accept_1_track1_cluster")

# HLTL3 = "[[nECLClustersLE>=3] and [AngleGG<3.054]]"
# db_access.upload_cut_to_db(HLTL3, "hlt", "accept_3_clusters")

HLTL4 = "[[G1CMSBhabhaLE>2.0] and [Bhabha2Trk==0]]"
db_access.upload_cut_to_db(HLTL4, "hlt", "accept_single_photon_2GeV_barrel")

HLTL5 = "[[G1CMSBhabhaLE>2.0] and [Bhabha2Trk==0] and [GG==0]]"
db_access.upload_cut_to_db(HLTL5, "hlt", "accept_single_photon_2GeV_endcap")

HLTL6 = "[[G1CMSBhabhaLE>1.0] and [Bhabha2Trk==0] and [GG==0]]"
db_access.upload_cut_to_db(HLTL6, "hlt", "accept_single_photon_1GeV")

HLTL7 = "[[nB2BCCPhiHighLE>=1] and [G1CMSBhabhaLE<2.0]]"
db_access.upload_cut_to_db(HLTL7, "hlt", "accept_b2bclusterhigh_phi")

HLTL8 = "[[nB2BCCPhiLowLE>=1] and [G1CMSBhabhaLE<2.0]]"
db_access.upload_cut_to_db(HLTL8, "hlt", "accept_b2bclusterlow_phi")

HLTL9 = "[[nB2BCC3DLE>=1] and [G1CMSBhabhaLE<2.0]]"
db_access.upload_cut_to_db(HLTL9, "hlt", "accept_b2bcluster_3D")

HLTL10 = "[Bhabha2Trk==1]"
# prescale_factor = [200,200,20,10,2,2,2,2,2]
db_access.upload_cut_to_db(HLTL10, "hlt", "accept_bhabha", 10)

trigmenu = [
    "accept_hadron",
    "accept_2_tracks",
    "accept_1_track1_cluster",
    "accept_mumu_2trk",
    "accept_mumu_1trk",
    "accept_tau_tau",
    "accept_single_photon_2GeV_barrel",
    "accept_single_photon_2GeV_endcap",
    "accept_single_photon_1GeV",
    "accept_b2bclusterhigh_phi",
    "accept_b2bclusterlow_phi",
    "accept_b2bcluster_3D",
    "accept_gamma_gamma",
    "accept_bhabha"]

db_access.upload_trigger_menu_to_db("hlt", trigmenu)
