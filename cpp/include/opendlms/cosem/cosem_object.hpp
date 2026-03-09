#pragma once
#include "opendlms/common/types.hpp"

namespace opendlms {

    class CosemObject {
        public:
            CosemObject(uint16_t classId, ObisCode obis)
                : classId_(classId), obis_(obis) {}

            virtual ~CosemObject() = default;

            uint16_t classId() const { return classId_; }
            ObisCode obis() const { return obis_; }

        protected:
            uint16_t classId_;
            ObisCode obis_;
    };

    // Specifically ICID 3
    class CosemRegister : public CosemObject {
        public:
            CosemRegister(ObisCode obis) : CosemObject(3, obis) {}
            DlmsValue value;
            DlmsValue scaler_unit; // Usually a structure containing scaler and unit
    };

} // namespace opendlms