package org.belle2.daq.graphics;

public interface GObject {

    public void draw(GraphicsDrawer c);

    public boolean hit(double x, double y);

}
