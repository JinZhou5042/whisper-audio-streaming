#ifndef AUDIO_MANAGER_HPP
#define AUDIO_MANAGER_HPP

#include <atomic>
#include <chrono>
#include <deque> // Using deque for efficient front removal
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef int socklen_t;
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif

class AudioManager {
public:
  AudioManager(int sample_rate, const std::string &server_ip = "192.168.4.1",
               int server_port = 5001);
  ~AudioManager();

  bool start();
  bool stop();
  bool waitForAudioSegment(std::vector<float> &audio_context, int segment_duration_s);
  bool pollEvents();
  void cleanup();

  // New methods for segment handling
  bool saveAudioSegment(const std::vector<float> &audio_data,
                        int segment_count);
  bool saveTextOutput(const std::string &text, int segment_count);

  std::string log_directory;

private:
  void _receive_loop();
  void writeWavHeader(std::ofstream &file, size_t data_size_bytes);
  void convertInt16ToFloat(const int16_t *int16_data, size_t sample_count);

  int sample_rate_;

  std::mutex mutex_;
  std::atomic<bool> capturing_{false};

  // Using deque for efficient front removal
  std::deque<float> audio_buffer;

  // UDP connection
  std::string server_ip_;
  int server_port_;
  SOCKET sock_;
  std::thread receive_thread_;
  std::atomic<bool> running_{false};
};

#endif // AUDIO_MANAGER_HPP
