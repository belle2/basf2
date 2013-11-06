package b2logger.java.ui;

import b2logger.core.LogMessage;
import b2logger.core.SystemLogLevel;

public class LogNoticePopup extends LogInfoPopup {
	
	public LogNoticePopup(LogMessage message) {
		super(message, 14, 5);
	}

	public LogNoticePopup() {
		this(new LogMessage("localhost", "LogViewer", SystemLogLevel.NOTICE,
				"This is <span color='red'>Konno</span>"));
	}

}
