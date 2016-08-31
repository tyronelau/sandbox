#include "xcoder/aac_encoder.h"

namespace agora {
namespace recording {

int aac_encode_create(HANDLE_AACENCODER *encoder) {
  if ( aacEncOpen(encoder, 1, 2) != AACENC_OK )
    return -1;

  return 0;
}

int aac_encode_init(HANDLE_AACENCODER encoder, int sample_rate, int bit_rate,
    AACENC_InfoStruct* info) {
  AACENC_ERROR ErrorStatus;

  ErrorStatus = aacEncoder_SetParam(encoder, AACENC_AOT, 2);
  ErrorStatus = aacEncoder_SetParam(encoder, AACENC_CHANNELMODE, 1);
  ErrorStatus = aacEncoder_SetParam(encoder, AACENC_BITRATEMODE, 0);
  ErrorStatus = aacEncoder_SetParam(encoder, AACENC_GRANULE_LENGTH, 1024);
  ErrorStatus = aacEncoder_SetParam(encoder, AACENC_SAMPLERATE, sample_rate);
  ErrorStatus = aacEncoder_SetParam(encoder, AACENC_BITRATE, bit_rate);

  ErrorStatus = aacEncEncode(encoder, NULL, NULL, NULL, NULL);

  if (ErrorStatus != AACENC_OK)
    return -1;

  ErrorStatus = aacEncInfo(encoder, info);
  return 0;
}

int aac_encode(HANDLE_AACENCODER encoder, int16_t* buffer, uint32_t samples,
    uint8_t* encoded) {
  AACENC_ERROR ErrorStatus;
  uint8_t ancillaryBuffer[50];
  int16_t inputBuffer[2048];
  uint8_t outputBuffer[2048];

  AACENC_MetaData metaDataSetup;
  void* inBuffer[] = { inputBuffer, ancillaryBuffer, &metaDataSetup };
  int inBufferIds[] = { IN_AUDIO_DATA, IN_ANCILLRY_DATA, IN_METADATA_SETUP };
  int inBufferSize[] = { sizeof(inputBuffer), sizeof(ancillaryBuffer), sizeof(metaDataSetup) };
  int inBufferElSize[] = { sizeof(int16_t), sizeof(uint8_t), sizeof(AACENC_MetaData) };
  static void* outBuffer[] = { outputBuffer };
  int outBufferIds[] = { OUT_BITSTREAM_DATA };
  int outBufferSize[] = { sizeof(outputBuffer) };
  int outBufferElSize[] = { sizeof(uint8_t) };

  AACENC_BufDesc inBufDesc = { 0 };
  AACENC_BufDesc outBufDesc = { 0 };
  inBufDesc.numBufs = sizeof(inBuffer) / sizeof(void*);
  inBufDesc.bufs = (void**)&inBuffer;
  inBufDesc.bufferIdentifiers = inBufferIds;
  inBufDesc.bufSizes = inBufferSize;
  inBufDesc.bufElSizes = inBufferElSize;
  outBufDesc.numBufs = sizeof(outBuffer) / sizeof(void*);
  outBufDesc.bufs = (void**)&outBuffer;
  outBufDesc.bufferIdentifiers = outBufferIds;
  outBufDesc.bufSizes = outBufferSize;
  outBufDesc.bufElSizes = outBufferElSize;

  AACENC_InArgs inargs = { 0 };
  AACENC_OutArgs outargs = { 0 };
  inargs.numInSamples = samples;
  FDKmemcpy(inputBuffer, buffer, sizeof(int16_t)*samples);
  ErrorStatus = aacEncEncode(encoder, &inBufDesc, &outBufDesc, &inargs, &outargs);

  int outlen = 0;
  if ( (outlen = outargs.numOutBytes) > 0 )
    FDKmemcpy(encoded, outputBuffer, outargs.numOutBytes);

  if (outargs.numInSamples > 0) {
    inargs.numInSamples -= outargs.numInSamples;
    if (inargs.numInSamples > 0) {
      FDKmemmove(inputBuffer, &inputBuffer[outargs.numInSamples], sizeof(int16_t)*inargs.numInSamples);
      ErrorStatus = aacEncEncode(encoder, &inBufDesc, &outBufDesc, &inargs, &outargs);
    }
  }
  return outlen;
}

int aac_encode_free(HANDLE_AACENCODER encoder) {
  aacEncClose(&encoder);
  return 0;
}

}
}
