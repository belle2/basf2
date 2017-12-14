package b2daq.dqm.graphics;

import b2daq.dqm.core.MonObject;
import b2daq.graphics.GraphicsDrawer;

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
