importPackage(Packages.org.csstudio.opibuilder.scriptUtil); 
var pv0 = PVUtil.getDouble(pvs[0]);
if(pv0 >= 1) {
	widget.setPropertyValue("background_color",ColorFontUtil.getColorFromRGB(0,255,0));
	widget.setPropertyValue("line_style", 0);
} else {
	widget.setPropertyValue("background_color",ColorFontUtil.getColorFromRGB(255,0,0));
	widget.setPropertyValue("line_style", 1);
}