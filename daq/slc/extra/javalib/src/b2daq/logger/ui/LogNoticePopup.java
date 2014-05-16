package b2daq.logger.ui;

import b2daq.logger.core.LogMessage;
import b2daq.logger.core.SystemLogLevel;

public class LogNoticePopup extends LogInfoPopup {
	
	public LogNoticePopup(LogMessage message) {
		super(message, 15, 5);
	}

	public LogNoticePopup() {
		this(new LogMessage("localhost", "LogViewer", SystemLogLevel.NOTICE,
				"This is <span color='red'>Konno</span>"));
	}

}
