// Real-time speech recognition using ESP32 WiFi Microphone
#include "audio_manager.hpp"
#include "params.cpp"
#include "translator.hpp"
#include "whisper.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstring>
#include <csignal>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <regex>
#include <string>
#include <thread>
#include <vector>
#include <sys/stat.h>

static std::string removeParens(const std::string &input) {
  std::string result;
  std::string current_token;
  bool discardRound = false;
  bool discardSquare = false;

  for (char ch : input) {
    if (ch == '(') {
      discardRound = true;
    } else if (ch == ')') {
      discardRound = false;
    } else if (ch == '[') {
      discardSquare = true;
    } else if (ch == ']') {
      discardSquare = false;
    } else if (!discardRound && !discardSquare) {
      if (ch == ' ') {
        if (!current_token.empty()) {
          result += current_token + " ";
          current_token.clear();
        }
      } else {
        current_token += ch;
      }
    }
  }

  if (!current_token.empty()) {
    result += current_token;
  }

  return result;
}

AudioManager *g_audioManager = nullptr;

[[noreturn]] static void handle_sigstp([[maybe_unused]] int signal) {
  if (g_audioManager != nullptr) {
    g_audioManager->cleanup();
  }
  exit(0);
}

void displayOutputText(const std::string &text_file_path) {
//  std::future<void> future = std::async(std::launch::async, [text_file_path]() {
//    std::string command =
//        "python ~/project/whisper-audio-streaming/src/display_text_output.py \"" + text_file_path + "\"";
//    std::cout << "Executing command: " << command << std::endl;
//    int result = system(command.c_str());
//    if (result != 0) {
//      std::cerr << "Error executing command: " << command << std::endl;
//    }
//  });
//  future.get();
  std::string command =
      "python ~/project/whisper-audio-streaming/src/display_text_output.py \"" + text_file_path + "\"";
  std::cout << "Executing command: " << command << std::endl;
  int result = system(command.c_str());
  if (result != 0) {
    std::cerr << "Error executing command: " << command << std::endl;
  }
}

const char *FIFO_PATH = "/tmp/whisper_fifo";

void initFifo() {
  remove(FIFO_PATH);
  std::cout << "Creating FIFO named pipe at " << FIFO_PATH << std::endl;
  if ((mkfifo(FIFO_PATH, 0666)) < 0) {
    std::cerr << "Error: unable to create FIFO pipe" << std::endl;
    exit(1);
  } else {
    std::cout << "FIFO pipe created" << std::endl;
  }
}

void writeFifo(const std::string &fn) {
  std::cout << "Writing \"" << fn << "\" to FIFO" << std::endl;

  int fd;
  fd = open(FIFO_PATH, O_WRONLY);
  if (fd < 0) {
    std::cerr << "Error: unable to open FIFO" << std::endl;
    exit(1);
  } else {
    std::cout << "FIFO opened" << std::endl;
    const char *fn_cstr = fn.c_str();
    write(fd, fn_cstr, strlen(fn_cstr));
    std::cout << "FIFO written" << std::endl;
  }
  close(fd);
  std::cout << "FIFO closed" << std::endl;
} 

int main(int argc, char **argv) {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  std::cout.tie(nullptr);

  Params params;
  if (!params.parse(argc, argv)) {
    return 1;
  }

  // Add ESP32 IP parameter
  std::string esp32_ip = "192.168.4.1"; // Default IP
  for (int i = 1; i < argc - 1; i++) {
    if (std::string(argv[i]) == "--esp32-ip" && i + 1 < argc) {
      esp32_ip = argv[i + 1];
      break;
    }
  }

  struct whisper_context_params cparams = whisper_context_default_params();
  cparams.use_gpu = params.use_gpu;
  cparams.flash_attn = params.flash_attn;

  struct whisper_context *ctx =
      whisper_init_from_file_with_params(params.model.c_str(), cparams);
  whisper_full_params wparams =
      whisper_full_default_params(WHISPER_SAMPLING_GREEDY);

  wparams.n_threads = 3;
  wparams.audio_ctx = 0;
  wparams.max_tokens = 0;
  wparams.language = "en";
  wparams.print_progress = false;
  wparams.print_special = false;
  wparams.print_realtime = false;
  wparams.no_timestamps = true;
  wparams.single_segment = true;
  wparams.tdrz_enable = false;
  wparams.temperature = 0.0f;

  printf("[Connecting to ESP32 microphone at %s]\n", esp32_ip.c_str());

  int sample_rate = 16000;
  AudioManager audio_manager(sample_rate, esp32_ip);
  g_audioManager = &audio_manager;
  signal(SIGTSTP, handle_sigstp);

  std::string translated_text;

  if (!audio_manager.start()) {
    std::cerr << "Failed to connect to ESP32. Make sure it's powered on and "
                 "WiFi is connected."
              << std::endl;
    return 1;
  }

  initFifo();

  printf("[Start speaking - Processing in %d-second segments with no gaps]\n\n",
         params.segment_duration_s);

  int segment_count = 0;
  while (audio_manager.pollEvents()) {
    std::vector<float> audio_segment;

    std::cout << "\n===== Segment " << segment_count << " =====\n";

    // Wait for a few seconds of audio to be collected
    if (audio_manager.waitForAudioSegment(audio_segment,
                                          params.segment_duration_s)) {
      // Save audio segment to file
      audio_manager.saveAudioSegment(audio_segment, segment_count);

      // Process audio with Whisper
      if (whisper_full(ctx, wparams, audio_segment.data(),
                       audio_segment.size()) != 0) {
        std::cerr << "Failed to recognize audio segment " << segment_count
                  << std::endl;
        segment_count++;
        continue;
      }

      // Extract recognized text
      std::string audio_text = "";
      const int n_segments = whisper_full_n_segments(ctx);
      for (int i = 0; i < n_segments; ++i) {
        audio_text += whisper_full_get_segment_text(ctx, i);
      }

      std::string clean_text = removeParens(audio_text);

      // Display recognized text
      std::cout << "TEXT:\n\"" << clean_text << "\"" << std::endl;

      // Save text to file
      audio_manager.saveTextOutput(clean_text, segment_count);

      // Save text log file to FIFO
      writeFifo(audio_manager.log_directory + "/text_output_" + std::to_string(segment_count) + ".txt");

      // Display output text
      // displayOutputText(audio_manager.log_directory + "/text_output_" + std::to_string(segment_count) + ".txt");

      // Optional: translate the text if enabled
      if (!params.translate.empty()) {
        translate_text(params.translate, clean_text, translated_text);
        std::cout << "Translation: " << translated_text << std::endl;
      }

      segment_count++;
    }
  }

  audio_manager.stop();
  whisper_free(ctx);

  return 0;
}
