#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import Module
from ROOT import Belle2


class PXDRawROIUnpackerModule(Module):
    """Unpack Raw Rois"""

    def initialize(self):
        """reimplementation of Module::initialize()."""
        self.mHLT = Belle2.PyStoreArray(Belle2.ROIid.Class(), "PXDROIsHLT")
        self.mDC = Belle2.PyStoreArray(Belle2.ROIid.Class(), "PXDROIsDC")
        self.mHLT.registerInDataStore()
        self.mDC.registerInDataStore()

    def event(self):
        """reimplementation of Module::event()."""

        rois = Belle2.PyStoreArray('PXDRawROIss')
        if not rois:
            return
        for q in rois:
            m = q.getNrROIs()
            for i in range(0, m):
                DHEID = q.getDHHID(i)
                sensor = (DHEID & 0x1) + 1
                ladder = (DHEID & 0x1E) >> 1  # no +1
                layer = ((DHEID & 0x20) >> 5) + 1
                v = Belle2.VxdID(layer, ladder, sensor)
                if q.getType(i) == 0:
                    ROI = self.mHLT.appendNew()
                    ROI.setSensorID(v)
                    ROI.setMinUid(q.getMinUid(i))
                    ROI.setMaxUid(q.getMaxUid(i))
                    ROI.setMinVid(q.getMinVid(i))
                    ROI.setMaxVid(q.getMaxVid(i))
                else:
                    ROI = self.mDC.appendNew()
                    ROI.setSensorID(v)
                    ROI.setMinUid(q.getMinUid(i))
                    ROI.setMaxUid(q.getMaxUid(i))
                    ROI.setMinVid(q.getMinVid(i))
                    ROI.setMaxVid(q.getMaxVid(i))


class PXDPayloadROIUnpackerModule(Module):
    """Unpack HLT Payload Rois"""

    def initialize(self):
        """reimplementation of Module::initialize()."""
        self.mPHLT = Belle2.PyStoreArray(Belle2.ROIid.Class(), "PXDROIsPayHLT")
        self.mPDC = Belle2.PyStoreArray(Belle2.ROIid.Class(), "PXDROIsPayDC")
        self.mPHLT.registerInDataStore()
        self.mPDC.registerInDataStore()

    def event(self):
        """reimplementation of Module::event()."""

        q = Belle2.PyStoreObj('ROIpayload')
        if not q:
            B2ERROR("ROIpayload is missing")
            return
        q = q.obj()
        n = q.getNrROIs()
        for i in range(0, n):
            DHEID = q.getDHHID(i)
            sensor = (DHEID & 0x1) + 1
            ladder = (DHEID & 0x1E) >> 1  # no +1
            layer = ((DHEID & 0x20) >> 5) + 1
            v = Belle2.VxdID(layer, ladder, sensor)
            if q.getType(i) == 0:
                ROI = self.mPHLT.appendNew()
                ROI.setSensorID(v)
                ROI.setMinUid(q.getMinUid(i))
                ROI.setMaxUid(q.getMaxUid(i))
                ROI.setMinVid(q.getMinVid(i))
                ROI.setMaxVid(q.getMaxVid(i))
            else:
                ROI = self.mPDC.appendNew()
                ROI.setSensorID(v)
                ROI.setMinUid(q.getMinUid(i))
                ROI.setMaxUid(q.getMaxUid(i))
                ROI.setMinVid(q.getMinVid(i))
                ROI.setMaxVid(q.getMaxVid(i))
