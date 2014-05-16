package b2daq.logger.ui;

import java.awt.Point;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.io.BufferedInputStream;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Iterator;

import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.Clip;
import javax.sound.sampled.DataLine;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;
import javax.swing.JWindow;
import javax.swing.SwingUtilities;
import javax.swing.event.MouseInputAdapter;

import b2daq.logger.core.LogMessage;
import b2daq.logger.core.SystemLogLevel;

public class LogWarningPopup extends LogPopup {

	protected static final int MARGIN = 10;
	protected static final int X_ZERO = WIDTH - MARGIN;
	protected static final int Y_ZERO = MARGIN;
	protected static final ArrayList<LogWarningPopup> POPUP_LIST = new ArrayList<LogWarningPopup>();
	protected int _width = 0;
	protected int _height = 0;
	
	static public void clearAllPopups() {
		Iterator<LogWarningPopup> ite = POPUP_LIST.iterator();
		while ( ite.hasNext() ) {
			LogWarningPopup popup = (LogWarningPopup)ite.next();
			popup.hide();
			ite.remove();
		}
	} 
	
	public LogWarningPopup(LogMessage message) {
		this(message, 15);
	}

	public LogWarningPopup() {
		this(new LogMessage("localhost", "LogViewer", 
				SystemLogLevel.WARNING,
				"This is <span color='red'>Konno</span>"));
	}

	public LogWarningPopup(LogMessage message, int font_size) {
		super(message, font_size);
		DragWindowListener mouse = new DragWindowListener();
		_window.addMouseMotionListener(mouse);
		_window.addMouseListener(mouse);
	}

	@Override
	public void run() {
		_window.add(_tip);
		_window.pack();
		_height = _window.getHeight();
		_width = _window.getWidth();
		setPosition();		
		_window.setSize(_window.getWidth(), 0);
		_window.setVisible(true);
		POPUP_LIST.add(this);
		try {
			InputStream istream = getClass().getResourceAsStream("yosshi.wav");
			InputStream bistream = new BufferedInputStream(istream);
			AudioInputStream stream = AudioSystem.getAudioInputStream(bistream);
			AudioFormat format = stream.getFormat();
			DataLine.Info di = new DataLine.Info(Clip.class, format);
			Clip clip = (Clip) AudioSystem.getLine(di);
			clip.open(stream);
			clip.start();
		} catch (Exception e) { /*JOptionPane.showMessageDialog(_window, e.toString());*/ }
		for( int n = 0; n < _height; n += 5) {
			try {
				sleep(20);
			} catch (InterruptedException e) {}
			_window.setSize(_window.getWidth(), n);
		}
        _window.setAlwaysOnTop(false);
	}

	public void dispose() {
		POPUP_LIST.remove(this);
		_window.dispose();
	}

	public void hide() {
		_window.dispose();
	}

	private class DragWindowListener extends MouseInputAdapter {
		
		private int _x, _y;
		
		public void mousePressed(MouseEvent arg0) {
			if ( SwingUtilities.isRightMouseButton(arg0) ) {
				JPopupMenu menu = new JPopupMenu();
				JMenuItem item = new JMenuItem("Delete");
				item.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent arg0) {
						dispose();
					}
				});
				menu.add(item);
				menu.show(arg0.getComponent(), arg0.getX(), arg0.getY());
			} else {
				_x = arg0.getX();
				_y = arg0.getY();
			}
		}

		public void mouseDragged(MouseEvent arg0) {
			JWindow win = (JWindow) arg0.getComponent();
			win.setLocation(arg0.getXOnScreen() - _x, arg0.getYOnScreen() - _y);
		}
	}
	
	public boolean isOverlaped(int x, int y, int width, int height) {
		if ( ( _window.getX() <= x && _window.getX() + _width >= x && 
				_window.getY() <= y && _window.getY() + _height >= y ) ||
			 ( _window.getX() <= x + width && _window.getX() + _width >= x + width && 
				_window.getY() <= y && _window.getY() + _height >= y ) ||
			 ( _window.getX() <= x && _window.getX() + _width >= x && 
				_window.getY() <= y + height && _window.getY() + _height >= y + height ) ||
			 ( _window.getX() <= x + width && _window.getX() + _width >= x + width && 
				_window.getY() <= y + height && _window.getY() + _height >= y + height )) {
			return true;
		} else {
			return false;
		}
	}
	
	public void setPosition () {
		int x = X_ZERO - _width;
		int y = Y_ZERO;
		int width_max = 0;
		Iterator<LogWarningPopup> it = POPUP_LIST.iterator();
		while ( it.hasNext() ) {
			Iterator<LogWarningPopup> it_r = POPUP_LIST.iterator();
			boolean overlapped = true;
			while ( it_r.hasNext() ) {
				overlapped = it_r.next().isOverlaped(x, y, _width, _height);
				if ( overlapped ) break;
			}
			if ( !overlapped ) {
				_window.setLocation(new Point(x, y));
				return;
			}
			LogWarningPopup popup = it.next();
			y += popup._height + MARGIN;
			if ( width_max < popup._width ) width_max = popup._width;
			if ( y + _height > HEIGHT + MARGIN ) {
				x -= width_max + MARGIN;
				y = MARGIN;
			}
		}
		_window.setLocation(new Point(x, y));
	}
	
}
