package org.belle2.daq.graphics;

public class FillProperty {

    private HtmlColor color;
    private double transparency;

    public FillProperty() {
        this(null, 1);
    }

    public FillProperty(HtmlColor c) {
        this(c, 1);
    }

    public FillProperty(HtmlColor c, int t) {
        color = c;
        transparency = t;
    }

    public void setColor(HtmlColor c) {
        color = c;
    }

    public void setTransparency(double t) {
        transparency = t;
    }

    public HtmlColor getColor() {
        return color;
    }

    public double getTransparency() {
        return transparency;
    }

    public String getStyle() {
        return "";
    }

    public void setStyle(String str) {
    }

    public void copy(FillProperty pro) {
        if (pro == null) {
            return;
        }
        transparency = pro.transparency;
        color = pro.color;
    }

}
