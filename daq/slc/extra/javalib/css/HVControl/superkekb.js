importPackage(Packages.org.csstudio.opibuilder.scriptUtil);
var pv0 = PVUtil.getString(pvs[0]);
if(pv0=="CONTINJ") {
  widget.setPropertyValue("foreground_color",ColorFontUtil.getColorFromRGB(0,0,0));
  widget.setPropertyValue("background_color",ColorFontUtil.getColorFromRGB(0,255,255));
} else if(pv0=="NORINJ") {
  widget.setPropertyValue("foreground_color",ColorFontUtil.getColorFromRGB(0,0,0));
  widget.setPropertyValue("background_color",ColorFontUtil.getColorFromRGB(255,128,0));
} else if(pv0=="NOINJECTION") {
  widget.setPropertyValue("foreground_color",ColorFontUtil.getColorFromRGB(0,0,0));
  widget.setPropertyValue("background_color",ColorFontUtil.getColorFromRGB(192,192,192));
} else {
  widget.setPropertyValue("foreground_color",ColorFontUtil.getColorFromRGB(0,0,0));
  widget.setPropertyValue("background_color",ColorFontUtil.getColorFromRGB(255,255,255));
}