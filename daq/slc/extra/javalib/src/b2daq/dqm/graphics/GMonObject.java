package b2daq.dqm.graphics;

import b2daq.dqm.core.MonObject;
import b2daq.graphics.GShape;

public abstract class GMonObject extends GShape {

	protected Canvas _canvas = null;
	protected MonObject _object = null;
	
	public GMonObject(Canvas canvas) {
		_canvas = canvas;
	}
	
	public abstract void update();
	
	public MonObject get() {
		return _object;
	}

	public void set(MonObject object) {
		_object = object;
	}
	
}
