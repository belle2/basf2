#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import ROOT
import math
from ROOT import Belle2
from modularAnalysis import *
from array import array

from heapq import nlargest
from effCalculation import EffCalculation

PI = 3.1415926
Fac = 180.0 / PI


import sys  # get argv
argvs = sys.argv
argn = len(argvs)
if argn != 3:
    sys.exit("ztsim02.py> # of arg is strange. Exit.")


def Count_mcpart(parts):
    npart = [0, 0]
    dec_cdc = [17, 150]
    dec_ecl = [14.1, 155]
    for part in parts:
        pdglist = [11, 13, 2212, 321, 211]
        pdg = math.fabs(part.getPDG())
        charge = part.getCharge()
        theta = part.getMomentum().Theta() * Fac
        if theta > dec_cdc[0] and theta < dec_cdc[1]:
            if pdg in pdglist:
                npart[0] += 1

        if math.fabs(pdg) == 22:
            if theta > dec_ecl[0] and theta < dec_ecl[1]:
                npart[1] += 1
    return npart


def Count_part_inDetector(parts):
    npart = [0, 0]
    for part in parts:
        pdg = part.getPDG()
        charge = part.getCharge()
        indec = part.hasSeenInDetector(Belle2.Const.DetectorSet(Belle2.Const.ECL))
        if indec:
            if math.fabs(charge) > 0:
                npart[0] += 1
            elif math.fabs(pdg) == 22:
                npart[1] += 1
    return npart


def Theta_MCTruth(part):
    theta = part.getMomentum().Theta() * Fac
    return theta


def NeutralCluster(clusters):
    neutral_cluster = []
    for cluster in clusters:
        if cluster.getRelatedFrom('TRGNNTracks'):
            continue
        else:
            neutral_cluster.append(cluster)
    return neutral_cluster


def Vec_Cluster(cluster, CMS=False):
    x = cluster.getPositionX()
    y = cluster.getPositionY()
    z = cluster.getPositionZ()
    e = cluster.getEnergyDep()
    vec = ROOT.TVector3(x, y, z)
    vec_unit = vec.Unit()
    vec_unit *= e
    v_mom = ROOT.TLorentzVector(vec_unit, e)
    if CMS:
        new_vec = trans.rotateLabToCms() * v_mom
    else:
        new_vec = v_mom
    new_theta = new_vec.Theta() * Fac
    new_phi = new_vec.Phi() * Fac
    if(new_phi < 0):
        new_phi += 2 * PI
    new_e = new_vec.E()
    NVC = [new_e, new_theta, new_phi]
    return NVC


def MatchTrk_Info(matchtrk, clusters):
    RV = [0., 0., 0., 0., 0., 0.]
    RV[0] = matchtrk.getPt()
    RV[1] = matchtrk.getPz()
    RV[2] = (PI - matchtrk.getTheta()) * Fac
    RV[3] = matchtrk.getPhi() * Fac
    clusterId = matchtrk.getECLClusterId()
    for cluster in clusters:
        clusterId2 = cluster.getClusterId()
        if clusterId == clusterId2:
            vec_ = Vec_Cluster(cluster, False)
            vec_cms = Vec_Cluster(cluster, False)
            RV[4] = vec_[0]
            RV[5] = vec_cms[0]
    return RV


def Max_Cluster(neuclus, CMS, n):
    E = []
    for neuclu in neuclus:
        NV = Vec_Cluster(neuclu, CMS)
        E.append(NV)
    if E and len(E) >= (n + 1):
        seqen = nlargest(n + 1, E, key=lambda item: item[0])
        return seqen[n]
    else:
        return [-1, -999, -999]


def Etot_Cluster(eclcluster):
    Elist = []
    for clu in eclcluster:
        e = clu.getEnergyDep()
        Elist.append(e)
    return sum(Elist)


def Max_DeltPhi_trk(trk_2d_list):
    Delt = []
    for i, trk in enumerate(trk_2d_list):
        phi1 = (trk.getPhi0()) * Fac
        for j, trk2 in enumerate(trk_2d_list):
            if j <= i:
                continue
            else:
                phi2 = (trk2.getPhi0()) * Fac
                Delt_phi = math.fabs(phi1 - phi2)
                Delt.append(Delt_phi)
    return max(Delt or [-100.0])


def BhabhaVeto1(matchtrks):
    Delt = []
    if len(matchtrks) <= 1:
        return Delt
    for i, matchtrk1 in enumerate(matchtrks):
        if i == len(matchtrks) - 1:
            continue
        cdctrk1 = matchtrk1.getRelatedTo('TRGNNTracks')
        cluster1 = matchtrk1.getRelatedTo('TRGECLClusters')
        if cdctrk1 and cluster1:
            tanLam1 = cdctrk1.getTanLambda()
            theta1 = math.acos(tanLam1 / math.sqrt(1. + tanLam1 * tanLam1)) * Fac
            phi1 = (cdctrk1.getPhi0()) * Fac
            if phi1 < 0.:
                phi1 += 2 * PI
            e1 = cluster1.getEnergyDep()
            for j, matchtrk2 in enumerate(matchtrks):
                if j <= i:
                    continue
                else:
                    cdctrk2 = matchtrk2.getRelatedTo('TRGNNTracks')
                    cluster2 = matchtrk2.getRelatedTo('TRGECLClusters')
                    if cdctrk2 and cluster2:
                        tanLam2 = cdctrk2.getTanLambda()
                        theta2 = math.acos(tanLam2 / math.sqrt(1. + tanLam2 * tanLam2)) * Fac
                        phi2 = (cdctrk2.getPhi0()) * Fac
                        if phi2 < 0.:
                            phi2 += 2 * PI
                        e2 = cluster2.getEnergyDep()
                        Delt_theta = theta1 + theta2 - 180
                        Delt_phi = math.fabs(phi1 - phi2) - 180
                        DeltArray = [Delt_theta, Delt_phi, e1, e2, e1 + e2]
                        Delt.append(DeltArray)
    if len(Delt) >= 1:
        return Delt
    else:
        return [-999., -999., -999., -999., -999]


def SBhabhaVeto(matchtrks):
    Elist = []
    if len(matchtrks) <= 1:
        return Elist
    for i, matchtrk1 in enumerate(matchtrks):
        cdctrk1 = matchtrk1.getRelatedTo('TRGNNTracks')
        cluster1 = matchtrk1.getRelatedTo('TRGECLClusters')
        if cdctrk1 and cluster1:
            e1 = cluster1.getEnergyDep()
            Elist.append([e1])
    if len(Elist) >= 1:
        return Elist
    else:
        return [-999.]


def eclBhabhaVeto(eclclusters):
    neclBhabha = []
    for i, cluster1 in enumerate(eclclusters):
        if i == len(eclclusters) - 1:
            continue
        e1 = cluster1.getEnergyDep()
        x1 = cluster1.getPositionX()
        y1 = cluster1.getPositionY()
        z1 = cluster1.getPositionZ()
        vec1 = ROOT.TVector3(x1, y1, z1)
        theta1 = vec1.Theta()
        phi1 = vec1.Phi()
        if(phi1 < 0):
            phi1 += 2 * PI
        for j, cluster2 in enumerate(eclclusters):
            e2 = cluster2.getEnergyDep()
            if j <= i:
                continue
            x2 = cluster2.getPositionX()
            y2 = cluster2.getPositionY()
            z2 = cluster2.getPositionZ()
            vec2 = ROOT.TVector3(x2, y2, z2)
            theta2 = vec2.Theta()
            phi2 = vec2.Phi()
            if phi2 < 0.0:
                phi2 += 2 * PI
            delt_theta = math.fabs((theta1 + theta2) * Fac - 180.0)
            delt_phi = math.fabs(phi1 - phi2) * Fac - 180.0
            if theta1 < theta2:
                efr = e1
                ebr = e2
            else:
                efr = e2
                ebr = e1
            bhacl = [delt_theta, delt_phi, efr, ebr, efr + ebr]
            neclBhabha.append(bhacl)
    return neclBhabha


def Max_DeltPhi_cluster(eclclusters):
    eclphi_col = []
    for i, cluster1 in enumerate(eclclusters):
        e1 = cluster1.getEnergyDep()
        if e1 < 0.1:
            continue
        x1 = cluster1.getPositionX()
        y1 = cluster1.getPositionY()
        z1 = cluster1.getPositionZ()
        vec1 = ROOT.TVector3(x1, y1, z1)
        phi1 = vec1.Phi()
        if(phi1 < 0):
            phi1 += 2 * PI
        for j, cluster2 in enumerate(eclclusters):
            e2 = cluster2.getEnergyDep()
            if j <= i:
                continue
            if e2 < 0.1:
                continue
            x2 = cluster2.getPositionX()
            y2 = cluster2.getPositionY()
            z2 = cluster2.getPositionZ()
            vec2 = ROOT.TVector3(x2, y2, z2)
            phi2 = vec2.Phi()
            if phi2 < 0.0:
                phi2 += 2 * PI
            delt_phi = math.fabs(phi1 - phi2) * 180.0 / 3.1415926
            eclphi_col.append(delt_phi)
    return max(eclphi_col or [-1.0])


def Time_Window(clusters, eventtime):
    new_clusters = []
    energy = []
    for ev in eventtime:
        event_time = ev.m_eventtiming
        event_tot = ev.m_etot
        energy.append([event_time, event_tot])
    if energy:
        tmp = max(energy, key=lambda item: item[1])
        event_time = tmp[0]
    else:
        event_time = -999999.
    for cluster in clusters:
        ctime_ori = cluster.getTimeAve()
        ctime = ctime_ori - event_time
        if math.fabs(ctime) < 100:
            new_clusters.append(cluster)
    return new_clusters


def Cluster_Threshold(clusters, threshold, CMS):
    new_clusters = []
    for cluster in clusters:
        eng = 0
        if CMS:
            newv = Vec_Cluster(cluster, CMS)
            eng = newv[0]
        else:
            eng = cluster.getEnergyDep()
        if eng > threshold:
            new_clusters.append(cluster)
    return new_clusters


def Back_to_Back(clusters1, clusters2):
    npai = 0
    for cluster1 in clusters1:
        cid1 = cluster1.getClusterId()
        vec1 = Vec_Cluster(cluster1, False)
        theta1 = vec1[1]
        phi1 = vec1[2]
        for cluster2 in clusters2:
            cid2 = cluster2.getClusterId()
            if cid1 == cid2:
                continue
            vec2 = Vec_Cluster(cluster2, False)
            theta2 = vec2[1]
            phi2 = vec2[2]
            delttheta = math.fabs(theta1 + theta2 - 180)
            deltphi = math.fabs(math.fabs(phi1 - phi2) - 180)
            if delttheta < 100 and deltphi < 100:
                npai += 1
    return npai


def Time_Cluster(clusters, eventtime):
    time_list = []
    energy = []
    event_time = 0.
    for ev in eventtime:
        event_time = ev.m_eventtiming
        event_tot = ev.m_etot
        energy.append([event_time, event_tot])
    if energy:
        tmp = max(energy, key=lambda item: item[1])
        event_time = tmp[0]
    for cluster in clusters:
        ctime_ori = cluster.getTimeAve()
        ctime = ctime_ori - event_time
        time_list.append(ctime)
    return time_list


def PrintBranchDef():
    print('\n')
    print('ntrk_2dfinder: # 2d finder track')
    print('ntrk_2dfitter: # 2d fitter track')
    print('ntrk_3dfitter: # 3d fitter track')
    print('ntrk_NN      : # Neuro network track')
    print('ntrk_2Dmatch : # 2d finder track w/ associated ecl cluster')
    print('ntrk_3Dmatch : # 3d finder track w/ associated ecl cluster')
    print('ntrk_klm     : # KLM track')
    print('nhit_klm     : # KLM hit', '\n')

    print('ncluster:                 # ecl cluster')
    print('ncluster_1000b:           # ecl cluster with threshold >1.0GeV, exclude TC ID 1,2, 17')
    print('ncluster_2000e:           # ecl cluster with threshold >2.0GeV in TC ID 1, 17')
    print('max_cluster[3]:           [energy, theta, phi] of the largest energetic ecl cluster')
    print('smax_cluster[3]:          [energy, theta, phi] of the secondary energetic ecl cluster')
    print('ncluster_neutral:         # ecl cluster w/o associated cdc track')
    print('max_cluster_neutral[3]:   [energy, theta, phi] of the largest energetic ecl neutral  cluster')
    print('smax_cluster_neutral[3]:  [energy, theta, phi] of the secondary energetic ecl neutral cluster')
    print('time_cluster:             the cluster timing obtained with cluster.timing-event.timing', '\n')

    print('nbbc:             # back to back cluster pairs')
    print('nbbtc:            # back to back track and cluster pairs')
    print('bhabha:           bhabha veto logic, 1: bhabha, 0: non bhabha')
    print('sbhabha:          bhabha with single track veto logic, 1: bhabha, 0: non bhabha')
    print('eclbhabha:        eclbhabha veto logic, 1: bhabha, 0: non bhabha')
    print('bhabha_var[5]:    variables used in bhabha logic, [Delt_theta, Delt_phi, E1, E2, E1+E2]')
    print('                  For two tracks: Delt_theta: theta1+theta2-180, Delt_phi: |phi1-phi2|-180')
    print('                  E1, E2 are the ecl clusters energy accociated with the two tracks')
    print('eclbhabha_var[5]: variables used in eclbhabha logic, [Delt_theta, Delt_phi, E1, E2, E1+E2]')
    print('                  For two eclclusers: Delt_theta: theta1+theta2-180, Delt_phi: |phi1-phi2|-180')
    print('                  E1, E2 are the ecl clusters energy')
    print('\n')


class CreateLogics(Module):
    """This module is to calculate some variables which are useful for the trigger development"""
    #: the output file
    file = ROOT.TFile(argvs[2], 'recreate')
    #: the tree in the output file
    tgrl = ROOT.TTree('tgrl', 'tree with GRL_Logic')
    #: #2d finder tracks
    ntrk_2dfinder_t = array('i', [-1])
    #: #2d fitter tracks
    ntrk_2dfitter_t = array('i', [-1])
    #: #3d fitter tracks
    ntrk_3dfitter_t = array('i', [-1])
    #: #NN tracks
    ntrk_NN_t = array('i', [-1])
    #: #2d matched tracks
    ntrk_2Dmatch_t = array('i', [-1])
    #: #3d matched tracks
    ntrk_3Dmatch_t = array('i', [-1])
    #: #max phi angle between two 2d finder tracks
    max_deltphi_2dfinder_t = array('f', [0.0])
    #: #cluster pairs with different energy threshold
    cpair_t = array('i', 8 * [0])
    #: #array components
    ncomp_clu = 3
    #: the total deposited cluster energy in ecl
    etot_t = array('f', [0.0])
    #: #ecl cluster with threshold >1.0GeV, exclude TC ID 1,2, 17
    ncluster_1000b_t = array('i', [-1])
    #: # ecl cluster with threshold >2.0GeV in TC ID 1, 17
    ncluster_2000e_t = array('i', [-1])
    #: # ecl clusters
    ncluster_t = array('i', [-1])
    #: # ecl cluster w/o associated cdc track
    ncluster_neutral_t = array('i', [-1])
    #: [energy, theta, phi] of the largest energetic ecl neutral  cluster
    max_cluster_neutral_t = array('f', ncomp_clu * [0.0])
    #: [energy, theta, phi] of the largest energetic ecl neutral  cluster in CMS
    max_cms_cluster_neutral_t = array('f', ncomp_clu * [0.0])
    #: [energy, theta, phi] of the largest energetic ecl cluster
    max_cluster_t = array('f', ncomp_clu * [0.0])
    #: [energy, theta, phi] of the largest energetic ecl cluster in CMS
    max_cms_cluster_t = array('f', ncomp_clu * [0.0])
    #: [energy, theta, phi] of the secondary energetic ecl neutral cluster
    smax_cluster_neutral_t = array('f', ncomp_clu * [0.0])
    #: [energy, theta, phi] of the secondary energetic ecl neutral cluster in CMS
    smax_cms_cluster_neutral_t = array('f', ncomp_clu * [0.0])
    #: [energy, theta, phi] of the secondary energetic ecl cluster
    smax_cluster_t = array('f', ncomp_clu * [0.0])
    #: [energy, theta, phi] of the secondary energetic ecl cluster in CMS
    smax_cms_cluster_t = array('f', ncomp_clu * [0.0])
    #: max delt phi angle between two clusters
    max_deltphi_cluster_t = array('f', [0.0])
    #: the timing of clusters
    time_cluster_t = array('f', 100 * [-99999.])
    #: #KLM track
    ntrk_klm_t = array('i', [-1])
    #: #KLM hits
    nhit_klm_t = array('i', [-1])
    #: # back to back track and cluster pairs
    npair_tc_t = array('i', [-1])
    #: # back to back cluster pairs
    npair_cc_t = array('i', [-1])
    #: #array components
    nbha_var = 5
    #: bhabha veto logic, 1: bhabha, 0: non bhabha
    bhabha_t = array('i', [0])
    #: bhabha with single track veto logic, 1: bhabha, 0: non bhabha
    sbhabha_t = array('i', [0])
    #: eclbhabha veto logic, 1: bhabha, 0: non bhabha
    eclbhabha_t = array('i', [0])
    #: variables used in bhabha logic, [Delt_theta, Delt_phi, E1, E2, E1+E2]
    bhabha_var_t = array('f', nbha_var * [0.0])
    #: variables used in eclbhabha logic, [Delt_theta, Delt_phi, E1, E2, E1+E2]
    eclbhabha_var_t = array('f', nbha_var * [0.0])

    # track
    tgrl.Branch('ntrk_2dfinder', ntrk_2dfinder_t, 'ntrk_2dfinder/i')
    tgrl.Branch('ntrk_2dfitter', ntrk_2dfitter_t, 'ntrk_2dfitter/i')
    tgrl.Branch('ntrk_3dfitter', ntrk_3dfitter_t, 'ntrk_3dfitter/i')
    tgrl.Branch('ntrk_NN', ntrk_NN_t, 'ntrk_NN/i')
    tgrl.Branch('ntrk_2Dmatch', ntrk_2Dmatch_t, 'ntrk_2Dmatch/i')
    tgrl.Branch('ntrk_3Dmatch', ntrk_3Dmatch_t, 'ntrk_3Dmatch/i')
    tgrl.Branch('npair_tc', npair_tc_t, 'npair_tc/i')
    tgrl.Branch('npair_cc', npair_cc_t, 'npair_cc/i')
    tgrl.Branch('max_deltphi_2dfinder', max_deltphi_2dfinder_t, 'max_deltphi_2dfinder/f')

    # cluster
    tgrl.Branch('etot', etot_t, 'etot/f')
    tgrl.Branch('ncluster', ncluster_t, 'ncluster/i')
    tgrl.Branch('ncluster_1000b', ncluster_1000b_t, 'ncluster_1000b/i')
    tgrl.Branch('ncluster_2000e', ncluster_2000e_t, 'ncluster_2000e/i')
    tgrl.Branch('ncluster_neutral', ncluster_neutral_t, 'ncluster_neutral/i')
    tgrl.Branch('max_cluster_neutral', max_cluster_neutral_t, 'max_cluster_neutral[3]/f')
    # tgrl.Branch('max_cms_cluster_neutral', max_cms_cluster_neutral_t, 'max_cms_cluster_neutral[3]/f')
    tgrl.Branch('max_cluster', max_cluster_t, 'max_cluster[3]/f')
    # tgrl.Branch('max_cms_cluster', max_cms_cluster_t, 'max_cms_cluster[3]/f')
    tgrl.Branch('smax_cluster_neutral', smax_cluster_neutral_t, 'smax_cluster_neutral[3]/f')
    # tgrl.Branch('smax_cms_cluster_neutral', smax_cms_cluster_neutral_t, 'smax_cms_cluster_neutral[3]/f')
    tgrl.Branch('smax_cluster', smax_cluster_t, 'smax_cluster[3]/f')
    # tgrl.Branch('smax_cms_cluster', smax_cms_cluster_t, 'smax_cms_cluster[3]/f')
    tgrl.Branch('max_deltphi_cluster', max_deltphi_cluster_t, 'max_deltphi_cluster/f')
    tgrl.Branch('time_cluster', time_cluster_t, 'time_cluster[100]/f')

    # klm
    tgrl.Branch('ntrk_klm', ntrk_klm_t, 'ntrk_klm/i')
    tgrl.Branch('nhit_klm', nhit_klm_t, 'nhit_klm/i')

    # bhabha
    tgrl.Branch('bhabha', bhabha_t, 'bhabha/i')
    tgrl.Branch('sbhabha', sbhabha_t, 'sbhabha/i')
    tgrl.Branch('eclbhabha', eclbhabha_t, 'eclbhabha/i')
    tgrl.Branch('bhabha_var', bhabha_var_t, 'bhabha_var[5]/f')
    tgrl.Branch('eclbhabha_var', eclbhabha_var_t, 'eclbhabha_var[5]/f')

    tgrl.Branch('cpair', cpair_t, 'cpair[8]/i')

    def event(self):
        """
        MCPart = Belle2.PyStoreArray('MCParticles')
        if len(MCPart)>=4:
            self.eplus_t[0]=Theta_MCTruth(MCPart[2])
            self.eminus_t[0]=Theta_MCTruth(MCPart[3])
            if self.eplus_t[0] > self.eminus_t[0]:
                 self.efrd_t[0]=self.eminus_t[0]
                 self.ebkd_t[0]=self.eplus_t[0]
            else:
                 self.efrd_t[0]=self.eplus_t[0]
                 self.ebkd_t[0]=self.eminus_t[0]
        count_part = Count_part_inDetector(MCPart)
        self.n_par_t[0]=count_part[0]
        self.n_par_t[1]=count_part[1]
        count_mcpart = Count_mcpart(MCPart)
        self.n_mcpar_t[0]=count_mcpart[0]
        self.n_mcpar_t[1]=count_mcpart[1]
        """
        trk_2d_finder = Belle2.PyStoreArray('TRG2DFinderTracks')
        self.ntrk_2dfinder_t[0] = len(trk_2d_finder)
        trk_2d_fitter = Belle2.PyStoreArray('TRG2DFitterTracks')
        self.ntrk_2dfitter_t[0] = len(trk_2d_fitter)
        trk_3d_fitter = Belle2.PyStoreArray('TRG3DFitterTracks')
        self.ntrk_3dfitter_t[0] = len(trk_3d_fitter)
        tracks = Belle2.PyStoreArray('TRGNNTracks')
        self.ntrk_NN_t[0] = len(tracks)

        # clusters
        clusters_original = Belle2.PyStoreArray('TRGECLClusters')
        eventtime = Belle2.PyStoreArray('TRGECLTrgs')
        # get clusters list in time window [-100,100]
        clusters = Time_Window(clusters_original, eventtime)
        self.ncluster_t[0] = len(clusters)
        # get the cluster timing before time window requirement
        time_clu_list = Time_Cluster(clusters_original, eventtime)
        for i in range(len(time_clu_list)):
            if i < 100:
                self.time_cluster_t[i] = time_clu_list[i]

        clusters_100 = Cluster_Threshold(clusters, 0.1, False)
        clusters_100_cms = Cluster_Threshold(clusters, 0.1, False)
        clusters_300_cms = Cluster_Threshold(clusters, 0.3, False)
        clusters_400_cms = Cluster_Threshold(clusters, 0.4, False)
        clusters_500_cms = Cluster_Threshold(clusters, 0.5, False)
        clusters_700_cms = Cluster_Threshold(clusters, 0.7, False)
        clusters_1000_cms = Cluster_Threshold(clusters, 1.0, False)
        clusters_2000_cms = Cluster_Threshold(clusters, 2.0, False)
        clusters_2500_cms = Cluster_Threshold(clusters, 2.5, False)
        self.cpair_t[0] = Back_to_Back(clusters, clusters)
        self.cpair_t[1] = Back_to_Back(clusters_300_cms, clusters)
        self.cpair_t[2] = Back_to_Back(clusters_400_cms, clusters)
        self.cpair_t[3] = Back_to_Back(clusters_500_cms, clusters)
        self.cpair_t[4] = Back_to_Back(clusters_700_cms, clusters)
        self.cpair_t[5] = Back_to_Back(clusters_1000_cms, clusters)
        self.cpair_t[6] = Back_to_Back(clusters_2000_cms, clusters)
        self.cpair_t[7] = Back_to_Back(clusters_2500_cms, clusters)

        klmtrkcol = Belle2.PyStoreArray('TRGKLMTracks')
        self.ntrk_klm_t[0] = len(klmtrkcol)
        klmhitcol = Belle2.PyStoreArray('TRGKLMHits')
        self.nhit_klm_t[0] = len(klmhitcol)

        matchlist = Belle2.PyStoreArray('TRG3DMatchTracks')
        self.ntrk_3Dmatch_t[0] = len(matchlist)

        trginfo = Belle2.PyStoreObj('TRGGRLObjects')
        self.npair_tc_t[0] = trginfo.getNbbTrkCluster()
        self.npair_cc_t[0] = trginfo.getNbbCluster()
        self.ncluster_1000b_t[0] = trginfo.getNhighcluster2()
        self.ncluster_2000e_t[0] = trginfo.getNhighcluster4()
        self.max_deltphi_2dfinder_t[0] = Max_DeltPhi_trk(trk_2d_finder)

        neutral_clusters = NeutralCluster(clusters)
        self.ncluster_neutral_t[0] = len(neutral_clusters)
        max_cluster_neu = Max_Cluster(neutral_clusters, False, 0)
        max_cluster = Max_Cluster(clusters, False, 0)
        # max_cms_cluster_neu = Max_Cluster(neutral_clusters, False, 0)
        # max_cms_cluster = Max_Cluster(clusters, False, 0)
        smax_cluster_neu = Max_Cluster(neutral_clusters, False, 1)
        smax_cluster = Max_Cluster(clusters, False, 1)
        # smax_cms_cluster_neu = Max_Cluster(neutral_clusters, False, 1)
        # smax_cms_cluster = Max_Cluster(clusters, False, 1)
        for i in range(self.ncomp_clu):
            self.max_cluster_neutral_t[i] = max_cluster_neu[i]
            # self.max_cms_cluster_neutral_t[i] = max_cms_cluster_neu[i]
            self.max_cluster_t[i] = max_cluster[i]
            # self.max_cms_cluster_t[i] = max_cms_cluster[i]
            self.smax_cluster_neutral_t[i] = smax_cluster_neu[i]
            # self.smax_cms_cluster_neutral_t[i] = smax_cms_cluster_neu[i]
            self.smax_cluster_t[i] = smax_cluster[i]
            # self.smax_cms_cluster_t[i] = smax_cms_cluster[i]

        self.etot_t[0] = Etot_Cluster(clusters)

        # bhabha
        bhabhaveto_1 = BhabhaVeto1(matchlist)
        bha_logic = 0
        for bha in bhabhaveto_1:
            if math.fabs(bha[0]) < 50 and math.fabs(bha[0]) > 10 and math.fabs(bha[1]) < 20:
                if bha[2] > 2.0 and bha[3] > 2.0 and bha[4] > 6.0:
                    if bha[2] > 3.0 or bha[3] > 3.0:
                        if len(trk_2d_finder) == 2:
                            bha_logic = 1
        self.bhabha_t[0] = bha_logic

        if len(bhabhaveto_1) >= 1:
            for i in range(self.nbha_var):
                self.bhabha_var_t[i] = bhabhaveto_1[0][i]

        # eclbhabha
        eclbhabhaveto = eclBhabhaVeto(clusters)
        eclbha_logic = 0
        for eclbha in eclbhabhaveto:
            if math.fabs(eclbha[0]) < 50 and math.fabs(eclbha[1]) < 50:
                if eclbha[2] > 2.0 and eclbha[3] > 2.0 and eclbha[4] > 6.0:
                    if eclbha[2] > 3.0 or eclbha[3] > 3.0:
                        eclbha_logic = 1
        self.eclbhabha_t[0] = eclbha_logic

        # sbhahba
        sbha_logic = 0
        if len(trk_2d_finder) == 1:
            if eclbha_logic == 1:
                ecol = SBhabhaVeto(matchlist)
                for i, etr in ecol:
                    if etr[i] > 1.0:
                        sbha_logic = 1
        self.sbhabha_t[0] = sbha_logic

        self.max_deltphi_cluster_t[0] = Max_DeltPhi_cluster(clusters)
        if len(eclbhabhaveto) >= 1:
            for i in range(self.nbha_var):
                self.eclbhabha_var_t[i] = eclbhabhaveto[0][i]
        self.tgrl.Fill()

    def terminate(self):
        """Write and close the file"""
        self.file.cd()
        self.file.Write()
        self.file.Close()
        PrintBranchDef()


if __name__ == "__main__":
    main = create_path()
    inputMdst('default', argvs[1], main)
    main.add_module(CreateLogics())
    EffCalculation(main)
    process(main)
