package cz.esw.serialization.handler;

import cz.esw.serialization.ResultConsumer;
import cz.esw.serialization.json.DataType;
import cz.esw.serialization.proto.*;

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
public class ProtoDataHandler implements DataHandler {

	private final InputStream is;
	private final OutputStream os;
	protected ArrayList<PMeasurementInfo> measurementInfo;
	protected Map<Integer, PRecords.Builder> recordMap;

	public ProtoDataHandler(InputStream is, OutputStream os) {
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
		PMeasurementInfo info = PMeasurementInfo.newBuilder()
				.setId(datasetId)
				.setMeasurerName(measurerName)
				.setTimestamp(timestamp)
				.build();

		PRecords.Builder record = PRecords.newBuilder();

		measurementInfo.add(info);
		recordMap.put(datasetId, record);
	}

	@Override
	public void handleValue(int datasetId, DataType type, double value) {
		switch (type) {
			case DOWNLOAD -> recordMap.get(datasetId).addDownload(value);
			case UPLOAD -> recordMap.get(datasetId).addUpload(value);
			case PING -> recordMap.get(datasetId).addPing(value);
		}
	}

	@Override
	public void getResults(ResultConsumer consumer) throws IOException {
		PMeasurementsRequest.Builder requestBuild = PMeasurementsRequest.newBuilder();

		for (PMeasurementInfo info : measurementInfo) {
			requestBuild.addRequestTuple(PRequestTuple.newBuilder()
					.setMeasurementInfo(info)
					.setRecords(recordMap.get(info.getId()).build())
					.build());
		}


		PMeasurementsRequest request = requestBuild.build();
		int sizeInt = request.getSerializedSize();
		byte[] sizeByte = ByteBuffer.allocate(4).putInt(sizeInt).array();

		os.write(sizeByte);
		request.writeTo(os);


		PMeasurementsResponse response = PMeasurementsResponse.parseFrom(is);

		for (PResponseTuple responseTuple : response.getResponseTupleList()) {
			PMeasurementInfo info = responseTuple.getMeasurementInfo();
			consumer.acceptMeasurementInfo(info.getId(), info.getTimestamp(), info.getMeasurerName());

			consumer.acceptResult(DataType.DOWNLOAD, responseTuple.getAverage().getDownload());
			consumer.acceptResult(DataType.UPLOAD, responseTuple.getAverage().getUpload());
			consumer.acceptResult(DataType.PING, responseTuple.getAverage().getPing());
		}
	}
}
