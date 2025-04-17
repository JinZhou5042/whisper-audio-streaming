#include "whisper.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// WAV header structure
struct WavHeader {
  char riff_header[4];      // "RIFF"
  uint32_t wav_size;        // File size - 8
  char wave_header[4];      // "WAVE"
  char fmt_header[4];       // "fmt "
  uint32_t fmt_chunk_size;  // 16 for PCM
  uint16_t audio_format;    // 1 for PCM, 3 for IEEE float
  uint16_t num_channels;    // 1 for mono
  uint32_t sample_rate;     // e.g., 16000
  uint32_t byte_rate;       // SampleRate * NumChannels * BitsPerSample/8
  uint16_t block_align;     // NumChannels * BitsPerSample/8
  uint16_t bits_per_sample; // 16 for PCM, 32 for float
  char data_header[4];      // "data"
  uint32_t data_chunk_size; // NumSamples * NumChannels * BitsPerSample/8
};

// Function to read a WAV file and return the audio data as float samples
bool read_wav_file(const std::string &filename, std::vector<float> &audio_data,
                   int &sample_rate) {
  std::ifstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Error: Could not open file " << filename << std::endl;
    return false;
  }

  // Read WAV header
  WavHeader header;
  file.read(reinterpret_cast<char *>(&header), sizeof(header));

  // Verify it's a WAV file
  if (strncmp(header.riff_header, "RIFF", 4) != 0 ||
      strncmp(header.wave_header, "WAVE", 4) != 0 ||
      strncmp(header.data_header, "data", 4) != 0) {
    std::cerr << "Error: Not a valid WAV file" << std::endl;
    return false;
  }

  // Get sample rate from header
  sample_rate = header.sample_rate;
  std::cout << "Sample rate: " << sample_rate << " Hz" << std::endl;
  std::cout << "Channels: " << header.num_channels << std::endl;
  std::cout << "Bits per sample: " << header.bits_per_sample << std::endl;
  std::cout << "Audio format: " << header.audio_format << std::endl;

  // Calculate number of samples
  size_t num_samples = header.data_chunk_size;
  if (header.audio_format == 3) { // IEEE float
    num_samples /= sizeof(float);
  } else if (header.bits_per_sample == 16) { // PCM 16-bit
    num_samples /= sizeof(int16_t);
  } else {
    std::cerr << "Error: Unsupported audio format" << std::endl;
    return false;
  }

  // Resize the audio data vector
  audio_data.resize(num_samples);

  // Read the audio data
  if (header.audio_format == 3) { // IEEE float (32-bit)
    // Direct read for float data
    file.read(reinterpret_cast<char *>(audio_data.data()),
              num_samples * sizeof(float));
  } else if (header.bits_per_sample == 16) { // PCM 16-bit
    // Read as int16_t and convert to float
    std::vector<int16_t> pcm_data(num_samples);
    file.read(reinterpret_cast<char *>(pcm_data.data()),
              num_samples * sizeof(int16_t));

    // Convert to float in range [-1.0, 1.0]
    for (size_t i = 0; i < num_samples; i++) {
      audio_data[i] = pcm_data[i] / 32768.0f;
    }
  }

  std::cout << "Read " << audio_data.size() << " audio samples" << std::endl;
  return true;
}

int main(int argc, char **argv) {
  // Check command line arguments
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << " <whisper_model_path> <wav_file_path>"
              << std::endl;
    return 1;
  }

  const char *model_path = argv[1];
  const char *wav_path = argv[2];

  // Initialize whisper context
  struct whisper_context_params cparams = whisper_context_default_params();
  struct whisper_context *ctx =
      whisper_init_from_file_with_params(model_path, cparams);

  if (ctx == nullptr) {
    std::cerr << "Error: Failed to initialize whisper context" << std::endl;
    return 1;
  }

  // Read WAV file
  std::vector<float> audio_data;
  int sample_rate;
  if (!read_wav_file(wav_path, audio_data, sample_rate)) {
    whisper_free(ctx);
    return 1;
  }

  // Set up whisper parameters
  whisper_full_params wparams =
      whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
  wparams.print_realtime = false;
  wparams.print_progress = true;
  wparams.print_timestamps = true;
  wparams.print_special = false;
  wparams.translate = false;
  wparams.language = "en";
  wparams.n_threads = 4;
  wparams.offset_ms = 0;

  // Run inference
  std::cout << "Running Whisper inference..." << std::endl;
  if (whisper_full(ctx, wparams, audio_data.data(), audio_data.size()) != 0) {
    std::cerr << "Error: Failed to process audio" << std::endl;
    whisper_free(ctx);
    return 1;
  }

  // Print results
  const int n_segments = whisper_full_n_segments(ctx);
  std::cout << "\nTranscription Results:\n"
            << std::string(50, '-') << std::endl;

  for (int i = 0; i < n_segments; ++i) {
    const char *text = whisper_full_get_segment_text(ctx, i);
    const int64_t t0 = whisper_full_get_segment_t0(ctx, i);
    const int64_t t1 = whisper_full_get_segment_t1(ctx, i);

    std::cout << "[" << (t0 / 100) / 10.0 << "s -> " << (t1 / 100) / 10.0
              << "s] " << text << std::endl;
  }

  // Clean up
  whisper_free(ctx);
  return 0;
}
