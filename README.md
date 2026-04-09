# dlms-cosem

A Java library for communicating with DLMS/COSEM smart meters over RS-485 serial and TCP/IP. Built from scratch with full IP ownership — no GPL dependencies.

---

## Features

- **Multi-meter polling** — concurrently read and write multiple meters on one or more RS-485 buses
- **HDLC framing** — full IEC 62056-46 HDLC implementation (SNRM, UA, DISC, I-frames, segmentation)
- **DLMS/COSEM application layer** — GET, SET, ACTION services (IEC 62056-53)
- **COSEM object model** — Register, Profile Generic (load profiles), Clock, Data objects with OBIS addressing
- **Transport abstraction** — RS-485 serial (via jSerialComm) and TCP/IP (WRAPPER mode)
- **Security** — Low authentication (password), High authentication, AES-128-GCM encryption (suite 0)
- **Non-blocking API** — `CompletableFuture`-based, meter pool with per-bus serialized access
- **Data callbacks** — plug in your own exporter to forward data to any central system

---

## Requirements

- Java 11+
- Maven 3.6+
- RS-485 serial adapter (e.g. USB-to-RS485) or TCP-accessible meter

---

## Quick Start

### Read a single meter

```java
MCosemClientConfig config = CosemClientConfig.builder()
    .transport(TransportConfig.rs485("/dev/ttyUSB0", 9600))
    .serverAddress(MeterAddress.hdlcServer(1))
    .clientAddress(MeterAddress.hdlcClient(16))
    .authentication(AarqApdu.AuthenticationLevel.NONE, null)
    .maxReceivePduSize(1024)
    .timeoutMs(2_000)
    .build();

try (CosemClient client = CosemClient.connect(config)) {
    RegisterObject energy = client.getObject(
        ObisCode.of("1.0.1.8.0.255"),
        RegisterObject.class
    );
    System.out.println("Energy import: " + energy.getValue() + " " + energy.getUnit());
}
```

### Read multiple meters concurrently

```java
private record MeterConfig(
    String meterId,
    String transportId,
    long interval,
    TimeUnit intervalUnit,
    CosemClientConfig clientConfig
) {}

// Initialize the polling engine
MeterPollingEngine engine = new MeterPollingEngine();

// Define the callback for when a reading arrives
MeterScheduler scheduler = new MeterScheduler(engine, reading -> {
    if (reading.isSuccessful()) {
        log.info("[OK] {}", reading);
        // Ready to be serialized and sent to api or database, etc
    } else {
        log.error("[FAIL] {}", reading);
    }
});

// A list of read meter tasks with intervals and transport details
List<MeterConfig> configurations = List.of(
    new MeterConfig(
        "METER_01", "TCP_HDLC_127.0.0.1_4060", 20, TimeUnit.SECONDS,
        CosemClientConfig.builder()
                .transport(TransportConfig.tcp("localhost", 4060, TransportConfig.FramingMode.HDLC))
                .serverAddress(MeterAddress.hdlcServer(1))
                .clientAddress(MeterAddress.hdlcClient(16))
                .authentication(AarqApdu.AuthenticationLevel.NONE, null)
                .maxReceivePduSize(1024)
                .timeoutMs(5_000)
                .maxRetries(2)
                .build()
    ),
    new MeterConfig(
        "METER_02", "TCP_HDLC_127.0.0.1_4061", 35, TimeUnit.SECONDS,
        CosemClientConfig.builder()
                .transport(TransportConfig.tcp("localhost", 4061, TransportConfig.FramingMode.HDLC))
                .serverAddress(MeterAddress.hdlcServer(1))
                .clientAddress(MeterAddress.hdlcClient(16))
                .authentication(AarqApdu.AuthenticationLevel.NONE, null)
                .maxReceivePduSize(1024)
                .timeoutMs(5_000)
                .maxRetries(2)
                .build()
    ),
    new MeterConfig(
        "METER_03", "TCP_WRAPPER_127.0.0.1_4070", 25, TimeUnit.SECONDS,
        CosemClientConfig.builder()
                .transport(TransportConfig.tcp("localhost", 4070, TransportConfig.FramingMode.WRAPPER))
                .serverAddress(MeterAddress.wrapperServer(1))
                .clientAddress(MeterAddress.wrapperClient(16))
                .authentication(AarqApdu.AuthenticationLevel.NONE, null)
                .maxReceivePduSize(1024)
                .timeoutMs(2_000)
                .build()
    ),
    new MeterConfig(
        "METER_04", "TCP_WRAPPER_127.0.0.1_4071", 30, TimeUnit.SECONDS,
        CosemClientConfig.builder()
                .transport(TransportConfig.tcp("localhost", 4071, TransportConfig.FramingMode.WRAPPER))
                .serverAddress(MeterAddress.wrapperServer(1))
                .clientAddress(MeterAddress.wrapperClient(16))
                .authentication(AarqApdu.AuthenticationLevel.NONE, null)
                .maxReceivePduSize(1024)
                .timeoutMs(2_000)
                .build()
    )
);
        
// Schedule all meters for polling
for  (MeterConfig configuration : configurations) {
    MeterReadTask task = new MeterReadTask(
            configuration.meterId,
            configuration.transportId,
            configuration.clientConfig
    );
    scheduler.schedule(task, configuration.interval, configuration.intervalUnit);
}
```

### Scheduled polling with callback

```java
MeterScheduler scheduler = new MeterScheduler(engine, reading -> {
    if (reading.isSuccessful()) {
        log.info("[OK] {}", reading);
        // Ready to be serialized and sent to api or database, etc
    } else {
        log.error("[FAIL] {}", reading);
    }
});
```

### Read load profile (Profile Generic)

```java
ProfileGenericObject profile = client.getObject(
    ObisCode.of("1.0.99.1.0.255"),
    ProfileGenericObject.class
);

List<List<DataObject>> buffer = profile.getBuffer(
    Instant.now().minus(Duration.ofDays(1)),
    Instant.now()
);

buffer.forEach(row -> System.out.println(row));
```

### Set meter clock

```java
ClockObject clock = client.getObject(
    ObisCode.of("0.0.1.0.0.255"),
    ClockObject.class
);
clock.setTime(ZonedDateTime.now());
```

---

## Configuration Reference

### `MeterConfig`

| Field | Type | Description |
|---|---|---|
| `meterId` | `String` | Unique identifier for this meter |
| `transport` | `TransportType` | `RS485` or `TCP` |
| `portName` | `String` | Serial port (e.g. `/dev/ttyUSB0`) or IP address |
| `baudRate` | `int` | Serial baud rate (default: `9600`) |
| `hdlcAddress` | `int` | Meter HDLC server address (1–126) |
| `clientAddress` | `int` | HDLC client address (default: `16`) |
| `authentication` | `AuthLevel` | `NONE`, `LOW`, `HIGH` |
| `password` | `String` | Low-level authentication password |
| `encryptionKey` | `byte[]` | AES-128 encryption key (16 bytes) |
| `timeoutMs` | `int` | Response timeout in ms (default: `10000`) |
| `retries` | `int` | Number of retries on timeout (default: `3`) |

---

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    Your Application                      │
└─────────────────────┬───────────────────────────────────┘
                      │
┌─────────────────────▼───────────────────────────────────┐
│                    dlms-cosem                            │
│                                                          │
│  ┌─────────────┐  ┌──────────────┐  ┌────────────────┐  │
│  │  MeterPool  │  │  CosemClient │  │  COSEM Objects │  │
│  │ (concurrent)│  │  (sessions)  │  │  (OBIS model)  │  │
│  └─────────────┘  └──────────────┘  └────────────────┘  │
│                                                          │
│  ┌─────────────┐  ┌──────────────┐  ┌────────────────┐  │
│  │    HDLC     │  │  BER Codec   │  │    Security    │  │
│  │  (framing)  │  │  (ASN.1)     │  │  (AES-128-GCM) │  │
│  └─────────────┘  └──────────────┘  └────────────────┘  │
│                                                          │
│  ┌─────────────┐  ┌──────────────┐                       │
│  │  RS-485     │  │     TCP      │                       │
│  │  Transport  │  │  Transport   │                       │
│  └─────────────┘  └──────────────┘                       │
└──────────────────────────────────────────────────────────┘
         ↕ RS-485                    ↕ TCP/IP
   [M1] [M2] [M3]...           [M4] [M5]...
```

---

## Standards

This library implements the following IEC standards:

| Standard | Description |
|---|---|
| IEC 62056-46 | HDLC data link layer |
| IEC 62056-53 | DLMS/COSEM application layer |
| IEC 62056-62 | COSEM interface classes and objects |
| IEC 62056-21 | Mode E serial handshake |
| DLMS Blue Book | COSEM object model (dlms.com) |
| DLMS Green Book | Architecture and security (dlms.com) |

---

## Dependencies

| Library | Version | License | Purpose |
|---|---|---|---|
| [jSerialComm](https://github.com/Fazecast/jSerialComm) | 2.11.0 | Apache 2.0 | RS-485 serial communication |
| [SLF4J](https://www.slf4j.org/) | 2.0.9 | MIT | Logging facade |

> All runtime dependencies are permissively licensed (Apache 2.0 / MIT). There are no GPL dependencies — dlms-cosem can be used in proprietary commercial applications.

---

## Contributing

Contributions are welcome. Please follow these steps:

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/my-feature`
3. Commit your changes: `git commit -m "feat: add my feature"`
4. Push to your fork: `git push origin feature/my-feature`
5. Open a pull request

### Commit message convention

This project follows [Conventional Commits](https://www.conventionalcommits.org/):

- `feat:` — new feature
- `fix:` — bug fix
- `chore:` — build, tooling, dependencies
- `docs:` — documentation only
- `test:` — tests only
- `refactor:` — code change with no feature or fix

### Running tests

```bash
mvn test
```

---

## Roadmap

- [x] Project scaffolding
- [x] RS-485 transport (jSerialComm)
- [x] HDLC framing layer
- [x] BER ASN.1 encoder/decoder
- [x] DLMS GET / SET / ACTION
- [x] COSEM object model (Register, Clock, Data)
- [x] MeterPool concurrent polling
- [x] Profile Generic (load profiles)
- [x] Security — Low auth, AES-128-GCM
- [x] TCP/WRAPPER transport
- [ ] Data export callbacks
- [ ] Central system integration example

---

## License

[MIT](LICENSE)