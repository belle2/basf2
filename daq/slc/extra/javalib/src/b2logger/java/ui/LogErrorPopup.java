package b2logger.java.ui;

import java.awt.Color;
import java.io.BufferedInputStream;
import java.io.InputStream;

import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.Clip;
import javax.sound.sampled.DataLine;

import b2logger.core.LogMessage;
import b2logger.core.SystemLogLevel;

public class LogErrorPopup extends LogWarningPopup {

    public LogErrorPopup(LogMessage message) {
	super(message, 16);
    }

    public LogErrorPopup() {
	this(new LogMessage("localhost", "LogViewer", SystemLogLevel.ERROR,
			    "This is <span color='red'>Konno</span>"));
    }

    @Override
	public void run() {
	_tip.setBackground(Color.WHITE);
	_window.add(_tip);
	_window.pack();
	new Thread() {
	    public void run() {
		try {
		    InputStream istream = getClass().getResourceAsStream("timlim.wav");
		    InputStream bistream = new BufferedInputStream(istream);
		    AudioInputStream stream = AudioSystem.getAudioInputStream(bistream);
		    AudioFormat format = stream.getFormat();
		    DataLine.Info di = new DataLine.Info(Clip.class, format);
		    Clip clip = (Clip) AudioSystem.getLine(di);
		    clip.open(stream);
		    clip.start();
		} catch (Exception e) {}
	    }
	}.start();
	try {
	    sleep(900);
	} catch (InterruptedException e) {}
	_window.setVisible(true);
	_width = _window.getWidth();
	_height = _window.getHeight();
	_tip.setVisible(false);
	for( int n = 0; n < 180; n += 5) {
	    try {
		sleep(12);
	    } catch (InterruptedException e) {}
	    _window.setBounds(WIDTH/2 - n , HEIGHT/2 - n, n*2, n*2);
	}
	_tip.setVisible(true);
	try {
	    sleep(3000);
	} catch (InterruptedException e) {}
	_window.setSize(_width, _height);
	setPosition();
	POPUP_LIST.add(this);
        _window.setAlwaysOnTop(false);
    }

}
