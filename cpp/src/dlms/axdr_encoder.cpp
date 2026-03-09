#include "opendlms/dlms/axdr_encoder.hpp"

namespace opendlms {

    void AxdrEncoder::writeTag(DlmsTag tag, std::vector<uint8_t>& buffer) {
        buffer.push_back(static_cast<uint8_t>(tag));
    }

    void AxdrEncoder::writeUint16(uint16_t val, std::vector<uint8_t>& buffer) {
        buffer.push_back(static_cast<uint8_t>(val >> 8));
        buffer.push_back(static_cast<uint8_t>(val & 0xFF));
    }

    void AxdrEncoder::encode(const DlmsValue& value, std::vector<uint8_t>& buffer) {
        std::visit([&buffer](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            
            if constexpr (std::is_same_v<T, uint16_t>) {
                buffer.push_back(static_cast<uint8_t>(DlmsTag::LongUnsigned));
                buffer.push_back(static_cast<uint8_t>(arg >> 8));
                buffer.push_back(static_cast<uint8_t>(arg & 0xFF));
            }
            else if constexpr (std::is_same_v<T, uint32_t>) {
                buffer.push_back(static_cast<uint8_t>(DlmsTag::DoubleLongUnsigned));
                buffer.push_back(static_cast<uint8_t>(arg >> 24));
                buffer.push_back(static_cast<uint8_t>(arg >> 16));
                buffer.push_back(static_cast<uint8_t>(arg >> 8));
                buffer.push_back(static_cast<uint8_t>(arg & 0xFF));
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                buffer.push_back(static_cast<uint8_t>(DlmsTag::VisibleString));
                buffer.push_back(static_cast<uint8_t>(arg.length())); // Simplification: length < 128
                for (char c : arg) buffer.push_back(static_cast<uint8_t>(c));
            }
            // ... more types to follow in Phase 2
        }, value);
    }

} // namespace opendlms