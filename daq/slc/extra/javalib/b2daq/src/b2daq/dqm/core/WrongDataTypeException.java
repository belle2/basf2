package b2daq.dqm.core;

import java.io.IOException;

public class WrongDataTypeException extends IOException {
	
	public WrongDataTypeException(String type) {
		super(type);
	}

	private static final long serialVersionUID = 1L;

}
