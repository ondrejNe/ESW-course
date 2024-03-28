package cz.esw.serialization.handler;

import cz.esw.serialization.ResultConsumer;
import cz.esw.serialization.avro.*;
import cz.esw.serialization.json.DataType;
import cz.esw.serialization.json.Dataset;
import org.apache.avro.io.*;
import org.apache.avro.specific.SpecificDatumReader;
import org.apache.avro.specific.SpecificDatumWriter;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

/**
 * @author Marek Cuchý (CVUT)
 */
public class AvroDataHandler implements DataHandler {
	private final InputStream is;
	private final OutputStream os;

	protected ArrayList<AMeasurementInfo> measurementInfo;
	protected Map<Integer, ARecords.Builder> recordMap;

	public AvroDataHandler(InputStream is, OutputStream os) {
		this.is = is;
		this.os = os;
	}

	@Override
	public void initialize() {
		measurementInfo = new ArrayList<>();
		recordMap = new HashMap<>();
	}

	@Override
	public void handleNewDataset(int datasetId, long timestamp, String measurerName) {
		AMeasurementInfo info = AMeasurementInfo.newBuilder()
				.setId(datasetId)
				.setMeasurerName(measurerName)
				.setTimestamp(timestamp)
				.build();

		ARecords.Builder record = ARecords.newBuilder();
		record.setDOWNLOAD(new ArrayList<>());
		record.setUPLOAD(new ArrayList<>());
		record.setPING(new ArrayList<>());

		measurementInfo.add(info);
		recordMap.put(datasetId, record);
	}

	@Override
	public void handleValue(int datasetId, DataType type, double value) {
		switch (type) {
			case DOWNLOAD -> recordMap.get(datasetId).getDOWNLOAD().add(value);
			case UPLOAD -> recordMap.get(datasetId).getUPLOAD().add(value);
			case PING -> recordMap.get(datasetId).getPING().add(value);
		}
	}

	@Override
	public void getResults(ResultConsumer consumer) throws IOException {
		AMeasurementsRequest.Builder requestBuild = AMeasurementsRequest.newBuilder();
		requestBuild.setRequestTuple(new ArrayList<>());

		for (AMeasurementInfo info : measurementInfo) {
			requestBuild.getRequestTuple().add(ARequestTuple.newBuilder()
					.setMeasurementInfo(info)
					.setRecords(recordMap.get(info.getId()).build())
					.build());
		}
		AMeasurementsRequest request = requestBuild.build();

		DatumWriter<AMeasurementsRequest> datumWriter = new SpecificDatumWriter<>(AMeasurementsRequest.class);
		ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
		BinaryEncoder encoder = EncoderFactory.get().binaryEncoder(byteArrayOutputStream , null);
		datumWriter.write(request, encoder);
		encoder.flush();

		int sizeInt = byteArrayOutputStream.size();
		byte[] sizeByte = ByteBuffer.allocate(4).putInt(sizeInt).array();

		os.write(sizeByte);

		os.write(byteArrayOutputStream.toByteArray());
		os.flush();

		AMeasurementsResponse response = new AMeasurementsResponse();
		DatumReader<AMeasurementsResponse> datumReader = new SpecificDatumReader<>(AMeasurementsResponse.class);
		DecoderFactory decoderFactory = new DecoderFactory();
		BinaryDecoder binaryDecoder = decoderFactory.binaryDecoder(is, null);
		datumReader.read(response, binaryDecoder);

		for (AResponseTuple responseTuple : response.getResponseTuple()) {
			AMeasurementInfo info = responseTuple.getMeasurementInfo();
			consumer.acceptMeasurementInfo(info.getId(), info.getTimestamp(), info.getMeasurerName().toString());

			consumer.acceptResult(DataType.DOWNLOAD, responseTuple.getAverage().getDOWNLOAD());
			consumer.acceptResult(DataType.UPLOAD, responseTuple.getAverage().getUPLOAD());
			consumer.acceptResult(DataType.PING, responseTuple.getAverage().getPING());
		}
	}
}
