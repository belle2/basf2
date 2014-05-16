package b2logger.java.ui;

import b2logger.core.LogMessage;
import b2logger.core.SystemLogLevel;

public class LogPopupLauncher {
	public LogPopup create(LogMessage message) {
		int level = message.getLogLevel().getLevel();
		if (level == SystemLogLevel.DEBUG.getLevel() ) {
			return new LogInfoPopup(message);
		} else if (level == SystemLogLevel.INFO.getLevel() ) {
			return new LogInfoPopup(message);
		} else if (level == SystemLogLevel.NOTICE.getLevel() ) {
			return new LogNoticePopup(message);
		} else if (level == SystemLogLevel.WARNING.getLevel() ) {
			return new LogWarningPopup(message);
		} else if (level == SystemLogLevel.ERROR.getLevel() ) {
			return new LogErrorPopup(message);
		} else if (level == SystemLogLevel.FATAL.getLevel() ) {
			return new LogFatalPopup(message);
		} else {
			return new LogInfoPopup(message);
		}
	}
}
