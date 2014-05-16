package b2daq.io;

import java.io.IOException;
import java.io.PipedOutputStream;

public class PipeDataWriter extends DataWriter {

	public PipeDataWriter(PipeDataReader reader) throws IOException {
		super(new PipedOutputStream(reader.getInputPipe()));
	}
}
