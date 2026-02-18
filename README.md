# OpenDLMS

**OpenDLMS** is a high-performance, multi-language implementation of the **DLMS/COSEM** protocol suite. This repository provides a unified toolset for communicating with smart meters via C++, Java, and Python.

## Key Features

* **Dual-Mode Transport:** Native support for both **HDLC (IEC 62056-46)** and **TCP Wrapper (IEC 62056-47)**.
* **Asynchronous I/O:** Built on `asio` for non-blocking communication with robust deadline timers and timeout handling.
* **Smart A-XDR Decoding:** Automatic translation of DLMS tags into C++ `std::variant` types, including specialized parsers for `DateTime` (OctetStrings).
* **State Management:** Automated HDLC sequence numbering N(S) and N(R) and association lifecycle management.
* **Permission Handling:** Built-in permissions handling.

---

## Project Structure

```text
OpenDLMS/
├── cpp/                # Core C++17 Library
│   ├── include/        # Header files (.hpp)
│   ├── src/            # Implementation files (.cpp)
│   ├── examples/       # Test clients
│   └── CMakeLists.txt  # Build configuration
├── java/               # Java references
└── python/             # Python references
    └── reference_client.py
```
## Protocol Specifications

- IEC 62056-46 (HDLC)
- IEC 62056-47 (TCP Wrapper)

## DLMS Simulator

You can get the simulator from gurux.fi
