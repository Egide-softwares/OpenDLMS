package com.pivotaccess.dlmscosem;

import com.pivotaccess.dlmscosem.apdu.AarqApdu;
import com.pivotaccess.dlmscosem.apdu.ApduException;
import com.pivotaccess.dlmscosem.apdu.DataObject;
import com.pivotaccess.dlmscosem.cosem.*;
import com.pivotaccess.dlmscosem.cosem.objects.ClockObject;
import com.pivotaccess.dlmscosem.cosem.objects.CosemDataObject;
import com.pivotaccess.dlmscosem.cosem.objects.ProfileGenericObject;
import com.pivotaccess.dlmscosem.transport.TransportConfig;
import com.pivotaccess.dlmscosem.util.MeterAddress;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.time.OffsetDateTime;
import java.time.format.DateTimeFormatter;

public class SerialMeter {
    private static final Logger log = LoggerFactory.getLogger(SerialMeter.class);

    public static void main(String[] args) {
        log.info("\nStarting DLMS/COSEM (SERIAL + HDLC)...");
        CosemClientConfig serialHdlc = CosemClientConfig.builder()
                .transport(TransportConfig.rs485("/tmp/ttyS11", 9600))
                .serverAddress(MeterAddress.hdlcServer(1))
                .clientAddress(MeterAddress.hdlcClient(1))
                .authentication(AarqApdu.AuthenticationLevel.NONE, null)
                .enableStrictSequenceControl(false)
                .maxReceivePduSize(1024)
                .timeoutMs(5_000)
                .build();
        try (CosemClient client = new CosemClient(serialHdlc)) {
            readOrSetValues(client);
        } catch (CosemClientException ex) {
            log.error("CosemClientException (SERIAL + HDLC): {}", ex.getMessage());
        } catch (ApduException e) {
            log.error("ApduException (SERIAL + HDLC): {}", e.getMessage());
        } finally {
            log.info("\nDLMS/COSEM client finished.");
        }
    }

    private static void readOrSetValues(CosemClient client) throws CosemClientException, ApduException {
        client.open();

        try {
            //log.info("Setting new time...");
            //client.setClock(ClockObject.encodeDateTime(OffsetDateTime.now()));

            //ClockObject clock = client.getClock();
            //log.info("Current time: {}", clock.getTime().format(DateTimeFormatter.ISO_DATE_TIME));

            String serialNumber = client.readSerialNumber();
            log.info("VAL 0: {}", serialNumber);

            log.info("Setting new serial number...");
            client.set(ObisCode.SERIAL_NUMBER, CosemClassId.DATA, 2, DataObject.ofVisibleString("1234567890"));

            String serialNumber2 = client.readSerialNumber();
            log.info("VAL 0 [NEW]: {}", serialNumber2);

            CosemDataObject manufactureId = client.getData(ObisCode.of("0.0.96.1.1.255"));
            log.info("VAL 1: {}", manufactureId.getStringValue());

            CosemDataObject val2 = client.getData(ObisCode.of("0.0.96.1.2.255"));
            log.info("VAL 2: {}", val2.getStringValue());

            CosemDataObject val3 = client.getData(ObisCode.of("0.0.96.1.3.255"));
            log.info("VAL 3: {}", val3.getStringValue());

            CosemDataObject val4 = client.getData(ObisCode.of("0.0.96.1.4.255"));
            log.info("VAL 4: {}", val4.getStringValue());

            CosemDataObject val5 = client.getData(ObisCode.of("0.0.96.1.5.255"));
            log.info("VAL 5: {}", val5.getStringValue());

            //log.info("Public/Management Association...");
            //DataObject data = client.get(ObisCode.of("0.0.40.0.1.255"), CosemClassId.ASSOCIATION_LN, 2);
            //log.info("DATA: {}", data);
        } catch (CosemClientException ignored) {
            log.error("READ FAILED");
        }

        client.close();
    }
}
