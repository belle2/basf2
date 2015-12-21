package org.belle2.daq.dqm.graphics;

import org.belle2.daq.dqm.MonObject;
import org.belle2.daq.graphics.GraphicsDrawer;

public abstract class GMonObject {

    protected HistogramCanvas canvas = null;
    protected MonObject object = null;

    public GMonObject(HistogramCanvas canvas) {
        this.canvas = canvas;
    }

    public abstract void update();
    public abstract void draw(GraphicsDrawer canvas);
    
    public MonObject getObject() {
        return object;
    }

    public void setObject(MonObject object) {
        this.object = object;
    }

}
