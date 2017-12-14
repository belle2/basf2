importPackage(Packages.org.csstudio.opibuilder.scriptUtil);
var pv0 = PVUtil.getString(pvs[0]);
if(pv0=="UNLOCKED") {
  widget.setPropertyValue("foreground_color",ColorFontUtil.getColorFromRGB(0,0,0));
  widget.setPropertyValue("background_color",ColorFontUtil.getColorFromRGB(0,255,255));
} else if(pv0=="INTERLOCK") {
  widget.setPropertyValue("foreground_color",ColorFontUtil.getColorFromRGB(0,0,0));
  widget.setPropertyValue("background_color",ColorFontUtil.getColorFromRGB(255,0,255));
} else {
  widget.setPropertyValue("foreground_color",ColorFontUtil.getColorFromRGB(0,0,0));
  widget.setPropertyValue("background_color",ColorFontUtil.getColorFromRGB(255,255,255));
}