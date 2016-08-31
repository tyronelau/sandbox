#pragma once

#include <cstdint>

#include "internal/fdkaac/aacenc_lib.h"

namespace agora {
namespace recording {

int aac_encode_create(HANDLE_AACENCODER *encoder);

int aac_encode_init(HANDLE_AACENCODER encoder, int sample_rate, int bit_rate,
    AACENC_InfoStruct* info);

int aac_encode(HANDLE_AACENCODER encoder, int16_t* buffer, uint32_t samples,
    uint8_t* encoded);

int aac_encode_free(HANDLE_AACENCODER encoder);

}
}
