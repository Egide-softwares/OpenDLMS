from python.common.obis import ObisCode

class ApduFactory:
    
    @staticmethod
    def create_get_request(invoke_id: int, class_id: int, obis_str: str, attribute_id: int) -> bytes:
        pdu = bytearray()
        pdu.append(0xC0)  # Tag: Get-Request
        pdu.append(0x01)  # Choice: Get-Request-Normal
        pdu.append(invoke_id)

        # 1. Interface Class ID (2 bytes)
        pdu.append((class_id >> 8) & 0xFF)
        pdu.append(class_id & 0xFF)

        # 2. OBIS Code (6 bytes)
        obis_obj = ObisCode(obis_str)
        pdu.extend(obis_obj.bytes)

        # 3. Attribute ID (1 byte)
        pdu.append(attribute_id)

        # 4. Data Access Selection (Optional: 00 for none)
        pdu.append(0x00)

        return bytes(pdu)