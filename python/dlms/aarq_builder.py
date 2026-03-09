def build_wrapper() -> bytes[int]:
    """
    Builds the wrapper for an AARQ message. This is a fixed header that is used for all AARQ messages.
    The wrapper is based on the standard xDLMS AARQ message, which is defined in the Blue Book (Section 10.1.2).
    """
    return bytes([
        0x60, 0x1D, 0xA1, 0x09, 0x06, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x01, 0x01,
        0xBE, 0x10, 0x04, 0x0E, 0x01, 0x00, 0x00, 0x00, 0x06,
        0x5F, 0x1F, 0x04, 0x00, 0x40, 0x1E, 0x5D, 0xFF, 0xFF
    ])

def build_hdlc() -> bytes[int]:
    """
    Builds the HDLC wrapper for an AARQ message. This is a fixed header that is used for all AARQ messages when using HDLC framing.
    The HDLC wrapper is defined in the Blue Book (Section 10.1.3) and includes the standard AARQ wrapper as the payload.
    """
    return bytes([
        # AARQ Tag, Total Length 29 (Standard xDLMS AARQ)
        0x60, 0x1D,
        # Application Context Name (11 bytes)
        # Identifies the protocol version (Logical Name Referencing)
        0xA1, 0x09,
        0x06, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x01, 0x01,

        # User Information (Length 16)
        0xBE, 0x10,
        # Octet String (Length 14)
        0x04, 0x0E,

        # --- xDLMS Initiate Request (14 Bytes) ---
        0x01,                   # Tag: initiateRequest
        0x00,                   # Dedicated key: None
        0x00,                   # Response Allowed: False
        0x00,                   # QoS: None
        0x06,                   # DLMS Version: 6

        # Conformance Block (4 bytes)
        # FIXED: Request ONLY [GET] (Bit 3).
        # This matches the "Public Client" capabilities defined in the Blue Book.
        0x5F, 0x1F, 0x04,
        0x00, 0x00, 0x00, 0x10,

        # Max PDU Size: 832 (0x0340)
        # This is a safe default for IP-based DLMS (See 171-dlmscosem.pdf)
        0x03, 0x40
    ])