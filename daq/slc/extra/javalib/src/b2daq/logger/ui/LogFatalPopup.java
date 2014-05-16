package b2daq.logger.ui;

import java.awt.Toolkit;
import java.io.BufferedInputStream;
import java.io.InputStream;

import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.Clip;
import javax.sound.sampled.DataLine;
import javax.swing.JFrame;

import b2daq.core.Log;
import b2daq.logger.core.LogMessage;
import b2daq.logger.core.SystemLogLevel;
import b2daq.ui.LogPanel;

public class LogFatalPopup extends LogPopup {

	private JFrame _frame;
	private ExceptionLogPanel _view = new ExceptionLogPanel();
	public LogFatalPopup(LogMessage message) {
		super(message, 16);
		_frame = new JFrame("Logger " + message.getPriority().getLabel());
		_view.add(new Log("<p width='100%' style='padding:0;text-align:center;font-size:110%;font-weight:bold;'>Logger FATAL</p>"+
				"Node : " + message.getNodeName() + "<br/>" +
				"Date : " + message.getDate().toString() + "<br/" +
				"Meesage : <br/>" + message.getMessage(), SystemLogLevel.FATAL));
		_frame.setIconImage(Toolkit.getDefaultToolkit().createImage(getClass().getResource("gaibu.png")));
		_frame.add(_view);
	}

	public LogFatalPopup() {
		this(new LogMessage("LogViewer", "LOCAL", SystemLogLevel.FATAL,
				"This is <span color='red'>Konno</span>"));
	}

	@Override
	public void run() {
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
		_frame.setBounds(0, 0, 400, 420);
		_frame.setLocationRelativeTo(null);
		_frame.setAlwaysOnTop(true);
		_frame.setVisible(true);
	}

	private class ExceptionLogPanel extends LogPanel {
		private static final long serialVersionUID = 1L;
		public ExceptionLogPanel() {
			super("Logger FATAL");
		}
		public String createHTML(Log log) {
			return "<div width='100%' style='color:"+
			log.getLevel().getFontColor() + ";background:" +
			log.getLevel().getFillColor() + ";font-family:Arial;font-size:110%;'>" + 
			log.getText() + "</div>";
		}
	}
	
}
