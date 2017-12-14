importPackage(Packages.org.csstudio.opibuilder.scriptUtil); 
var pv0 = PVUtil.getDouble(pvs[0]);
var pv1 = PVUtil.getDouble(pvs[1]);
if(pv0 > 0)
	widget.setPropertyValue("background_color",ColorFontUtil.getColorFromRGB(0,255,170));
else if(pv0 < 0 && pv1 > 0)
	widget.setPropertyValue("background_color",ColorFontUtil.getColorFromRGB(255,0,0));
else
	widget.setPropertyValue("background_color",ColorFontUtil.getColorFromRGB(255,255,255));
