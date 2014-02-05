package b2dqm.graphics;

import b2daq.core.RCNode;
import b2daq.core.RCState;
import b2daq.graphics.GCanvas;
import b2daq.graphics.HtmlColor;

public class GRCStateLabel extends GEnumLabel {

	private RCNode _node = new RCNode();
	private boolean _enabled = true;

	public GRCStateLabel() {
		_text.setFontSize(1.75);
	}

	public RCNode getNode() {
		return _node;
	}

	public void setEnabled(boolean enabled) {
		_enabled = enabled;
		update();
	}
	
	public void setNode(RCNode node) {
		_node = node;
	}
	public void setState(int state) {
		_node.getState().copy(state);
		update();
	}

	public void update() {
		if ( !_enabled ) {
			_rect.setLineColor(HtmlColor.GRAY);
			_rect.setFillColor(HtmlColor.LIGHT_GRAY);
			_text.setFontColor(HtmlColor.GRAY);
			_text.setText("");
		} else {
			if (!_node.isUsed()) {
				_text.setFontSize(1.1);
				_rect.setLineColor(HtmlColor.GRAY);
				_rect.setFillColor(HtmlColor.LIGHT_GRAY);
				_text.setFontColor(HtmlColor.GRAY);
				_text.setText("NOTUSED");
			} else {
				if ( _node.getState().equals(RCState.RUNNING_S)) {
					_rect.setLineColor(HtmlColor.GREEN);
					_rect.setFillColor(HtmlColor.DARK_GREEN);
					_text.setFontColor(HtmlColor.WHITE);
				} else if (_node.getState().getLabel().endsWith("_S")) {
					_rect.setLineColor(HtmlColor.CYAN);
					_rect.setFillColor(HtmlColor.BLUE);
					_text.setFontColor(HtmlColor.WHITE);
				} else if (_node.getState().getLabel().endsWith("_TS")) {
					_rect.setLineColor(HtmlColor.YELLOW);
					_rect.setFillColor(HtmlColor.GOLD);
					_text.setFontColor(HtmlColor.WHITE);
				} else if (_node.getState().getLabel().endsWith("_ES")) {
					_rect.setLineColor(new HtmlColor("#BB0000"));
					_rect.setFillColor(HtmlColor.RED);
					_text.setFontColor(HtmlColor.WHITE);
				} else if (_node.getState().getLabel().endsWith("_RS")) {
					_rect.setLineColor(new HtmlColor("#DDA0DD"));
					_rect.setFillColor(new HtmlColor("#9932CC"));
					_text.setFontColor(HtmlColor.WHITE);
					_text.setText("ABORTING");
				} else {
					_rect.setLineColor(HtmlColor.RED);
					_rect.setFillColor(HtmlColor.WHITE);
					_text.setFontColor(HtmlColor.RED);
				}
				_text.setText(_node.getState().getAlias());
			}
		}
	}

	@Override
	public void draw(GCanvas c) {
		update();
		_rect.draw(c);
		_text.draw(c);
	}

}
