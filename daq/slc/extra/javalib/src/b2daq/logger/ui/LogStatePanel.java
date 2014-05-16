package b2daq.logger.ui;

import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;

import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;

import b2daq.graphics.FontProperty;
import b2daq.graphics.GRect;
import b2daq.graphics.GText;
import b2daq.graphics.HtmlColor;
import b2daq.logger.core.LogMessage;
import b2daq.logger.core.SystemLogLevel;
import b2daq.ui.GCanvasPanel;

public class LogStatePanel extends GCanvasPanel {
	
	private static final long serialVersionUID = 1L;
	private GText _g_state_text = new GText("UNDEFINED", 0.75, 0.44, "center middle");
	private GRect _g_state_rect = new GRect(0.55, 0.32, 0.4, 0.24, HtmlColor.WHITE, HtmlColor.BLACK);
	private GRect _g_background = new GRect(0, 0, 1, 1, HtmlColor.WHITE, HtmlColor.BLACK);
	private GText _g_name = new GText("dcprocess", 0.5, 0.20, "center", 0., HtmlColor.BLACK);
	private GText _g_update_time = new GText("1900/01/01 00:00", 0.95, 0.8, "right", 0., HtmlColor.BLACK);
	private boolean _is_stable = true;
	
	public LogStatePanel() {
		_g_name.setFontSize(1.15);
		setPreferredSize(new Dimension(200, 60));
		add(_g_background);
		add(_g_name);
		add(new GText("state : ", 0.05, 0.44, "left", 0., HtmlColor.BLACK));
		add(_g_state_rect);
		add(_g_state_text);
		add(new GText("update at : ", 0.05, 0.8, "left", 0., HtmlColor.BLACK));
		_g_update_time.setFontWeight(FontProperty.WEIGHT_NORMAL);
		add(_g_update_time);
		addMouseListener(new MouseListener() {
			public void mouseClicked(MouseEvent arg0) {
				if ( !_is_stable ) {
					JPopupMenu popup_menu = new JPopupMenu();
					JMenuItem menu = new JMenuItem("Clear problem");
					menu.addActionListener(new ActionListener() {
						public void actionPerformed(ActionEvent arg0) {
							new Thread () {
								public void run() {
									/*
									Belle2LogViewer.get().sendMessage(new LogMessage(
											getProcessName(), getProcessName(), 
											SystemLogLevel.INFO, "[from GUI] Problem was cleared."));
									 */
								}
							}.start();
						}
					});
					popup_menu.add(menu);
					popup_menu.show(arg0.getComponent(), arg0.getX(), arg0.getY());
				}
			}
			public void mouseEntered(MouseEvent arg0) {}
			public void mouseExited(MouseEvent arg0) {}
			public void mousePressed(MouseEvent arg0) {}
			public void mouseReleased(MouseEvent arg0) {}
		});
	}
	
	public LogStatePanel(LogMessage message) {
		this();
		_g_name.setText(message.getGroupName());
		update(message);
	}
	
	public void update(LogMessage message) {
		SystemLogLevel level = message.getPriority();
		if ( level.hiegher(SystemLogLevel.ERROR) ) {
			_g_state_text.setText("PROBLEM");
			_g_state_text.setFontColor(HtmlColor.WHITE);
			_g_state_rect.setFillColor(HtmlColor.RED);
			_g_state_rect.setLineColor(HtmlColor.PINK);
			_is_stable = false;
		} else if ( !_g_state_text.getText().matches("PROBLEM") &&
				level.equals(SystemLogLevel.WARNING) ) {
			_g_state_text.setText("WARNING");
			_g_state_text.setFontColor(HtmlColor.BLACK);
			_g_state_rect.setFillColor(HtmlColor.YELLOW);
			_g_state_rect.setLineColor(HtmlColor.WHITE);
			_is_stable = false;
		} else if ( !level.hiegher(SystemLogLevel.WARNING) ) {
			_g_state_text.setText("FINE");
			_g_state_text.setFontColor(HtmlColor.WHITE);
			_g_state_rect.setFillColor(HtmlColor.DARK_GREEN);
			_g_state_rect.setLineColor(HtmlColor.GREEN);
			_is_stable = true;
		}
		_g_update_time.setText(message.getDate().toString());
		repaint();
	}
	
	public void setProcessName(String name) {
		_g_name.setText(name);
	}
	
	public String getProcessName() {
		return _g_name.getText();
	}
	
	public void setBackgroundColor(HtmlColor color) {
		_g_background.setFillColor(color);
	}
	
}
