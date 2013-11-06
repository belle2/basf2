package b2logger.java.ui;

import java.awt.Point;
import java.io.BufferedInputStream;
import java.io.InputStream;

import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.Clip;
import javax.sound.sampled.DataLine;

import b2logger.core.LogMessage;
import b2logger.core.SystemLogLevel;

public class LogInfoPopup extends LogPopup {

    private int _sleep_time = 2;
    
	public LogInfoPopup(LogMessage message) {
		this(message, 12);
	}
	
	public LogInfoPopup(LogMessage message, int font_size) {
		this(message, font_size, 2);
	}
	
	public LogInfoPopup(LogMessage message, int font_size, final int sleep_time) {
		super(message, font_size);
        _sleep_time = sleep_time; 
	}

	public LogInfoPopup() {
		this(new LogMessage("localhost", "LogViewer", SystemLogLevel.INFO, 
				"This is <span color='red'>Konno</span>"));
	}

	@Override
	public void run() {
		_window.add(_tip);
		_window.pack();
		_window.setLocation(new Point(WIDTH - _window.getWidth() - 10, HEIGHT + 10));
		_window.setVisible(true);
		int height = _window.getHeight() + 10;
		new Thread() {
			public void run() {
				try {
					String file_name = "super_kinoko.wav";
					if ( _message.getMessage().indexOf("[from GUI]") >= 0 ) {
						file_name = "kuribo.wav";
					}
					if ( _message.getMessage().indexOf("The run reached to the end.") >= 0 ) {
						file_name = "1up.wav";
					}
					InputStream istream = getClass().getResourceAsStream(file_name);
					InputStream bistream = new BufferedInputStream(istream);
					AudioInputStream stream = AudioSystem.getAudioInputStream(bistream);
					AudioFormat format = stream.getFormat();
					DataLine.Info di = new DataLine.Info(Clip.class, format);
					Clip clip = (Clip) AudioSystem.getLine(di);
					clip.open(stream);
					clip.start();
				} catch (Exception e) {/*JOptionPane.showMessageDialog(_window, e.toString());*/}
			}
		}.start();
		for( int n = 0; n < height; n += 5) {
			try {
				sleep(50);
			} catch (InterruptedException e) {}
			int x = _window.getX();
			int y = _window.getY();
			_window.setLocation(x, y - 5);
		}
		try {
			sleep(_sleep_time*1000);
		} catch (InterruptedException e) {}
		for( int n = 0; n < height; n += 5) {
			try {
				sleep(50);
			} catch (InterruptedException e) {}
			int x = _window.getX();
			int y = _window.getY();
			_window.setLocation(x, y + 5);
		}
		_window.dispose();
	}
}

