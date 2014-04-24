package b2rc.java.ui;

import java.awt.Dimension;

import b2daq.graphics.GRect;
import b2daq.graphics.GText;
import b2daq.graphics.HtmlColor;
import b2daq.java.graphics.JavaGCanvasPanel;
import b2daq.core.RCConnection;
import b2daq.core.RCNode;
import b2daq.core.RCState;

public class RCNodeStateLabel extends JavaGCanvasPanel {

	private static final long serialVersionUID = 1L;

	private GRect _rect = new GRect(0.02, 0.02, 0.96, 0.96);
	private GText _text = new GText("", 0.5, 0.5, "center middle");
	private RCNode _node;
	
	public RCNodeStateLabel(RCNode node) {
		super();
		setPreferredSize(new Dimension(150, 30));
		_text.setFontSize(1.75);
		_rect.setLineWidth(2);
		add(_rect);
		add(_text);
		setOpaque(false);
		update(node);
	}

	public void setEnabled(boolean enabled) {
		super.setEnabled(enabled);
		update(_node);
	}
	
	public void update(RCNode node) {
		_node = node;
		if ( !isEnabled() ) {
			_rect.setLineColor(HtmlColor.GRAY);
			_rect.setFillColor(HtmlColor.LIGHT_GRAY);
			_text.setFontColor(HtmlColor.GRAY);
			_text.setText("");
		} else {
			_text.setFontSize(1.25);
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
				if ( _node.getConnection().equal(RCConnection.OFFLINE) ) {
					_rect.setLineColor(HtmlColor.RED);
					_rect.setFillColor(HtmlColor.WHITE);
					_text.setFontColor(HtmlColor.RED);
					_text.setText("DISCONNECTED");
				} else {
					_text.setText(_node.getState().getLabel());
				}
			}
		}
		this.repaint();
	}
}
