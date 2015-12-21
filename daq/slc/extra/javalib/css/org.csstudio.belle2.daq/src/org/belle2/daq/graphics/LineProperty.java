package org.belle2.daq.graphics;

public class LineProperty {

    private HtmlColor color = null;
    private float width = 1;
    private String style = "";

    public LineProperty() {
    }

    public LineProperty(HtmlColor c) {
        this(c, 1);
    }

    public LineProperty(HtmlColor c, int w) {
        this(c, w, "solid");
    }

    public LineProperty(HtmlColor c, int w, String style) {
        color = c;
        width = w;
        this.style = style;
    }

    public void setColor(HtmlColor c) {
        color = c;
    }

    public void setWidth(double width) {
        this.width = (float)width;
    }

    public void setStyle(String s) {
        style = s;
    }

    public HtmlColor getColor() {
        return color;
    }

    public float getWidth() {
        return width;
    }

    public String getStyle() {
        return style;
    }

    public void copy(LineProperty pro) {
        if (pro == null) {
            return;
        }
        width = pro.width;
        color = pro.color;
        style = pro.style;
    }

}
