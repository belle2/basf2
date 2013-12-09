package b2dqm.graphics;

import b2daq.graphics.GShape;
import b2dqm.core.MonObject;

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
