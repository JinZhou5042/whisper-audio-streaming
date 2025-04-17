#include "audio_manager.hpp"
#include <cmath>
#include <stdexcept>

AudioManager::AudioManager(int sample_rate, const std::string &server_ip, int server_port) 
    : sample_rate_(sample_rate), 
      server_ip_(server_ip),
      server_port_(server_port) {

    // Initialize socket
    #ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("Failed to initialize Winsock");
    }
    #endif

    // Create UDP socket
    sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_ == INVALID_SOCKET) {
        #ifdef _WIN32
        WSACleanup();
        #endif
        throw std::runtime_error("Failed to create socket");
    }

    // Set socket timeout
    #ifdef _WIN32
    DWORD timeout = 1000; // 1 second
    setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));
    #else
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));
    #endif

    std::cout << "AudioManager initialized, ready to connect to ESP32 at " 
              << server_ip_ << ":" << server_port_ << std::endl;
}

AudioManager::~AudioManager() {
    if (running_) {
        stop();
    }
    
    // Close socket
    #ifdef _WIN32
    closesocket(sock_);
    WSACleanup();
    #else
    ::close(sock_);
    #endif
}

bool AudioManager::start() {
    if (running_) {
        return true;
    }
    
    // Connect to the UDP server (ESP32)
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port_);
    
    #ifdef _WIN32
    inet_pton(AF_INET, server_ip_.c_str(), &server_addr.sin_addr);
    #else
    server_addr.sin_addr.s_addr = inet_addr(server_ip_.c_str());
    #endif
    
    // Send initialization packet
    const char *hello_msg = "hello";
    if (sendto(sock_, hello_msg, strlen(hello_msg), 0,
               (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed: sendto error" << std::endl;
        return false;
    }
    
    running_ = true;
    capturing_ = true;
    samples_collected_ = 0;
    
    // Start receive thread
    receive_thread_ = std::thread(&AudioManager::_receive_loop, this);
    std::cout << "Started audio streaming from ESP32" << std::endl;
    
    return true;
}

bool AudioManager::stop() {
    if (!running_) {
        return false;
    }
    
    running_ = false;
    capturing_ = false;
    
    if (receive_thread_.joinable()) {
        receive_thread_.join();
    }
    
    return true;
}

void AudioManager::resetBuffer() {
    std::lock_guard<std::mutex> lock(mutex_);
    audio_buffer.clear();
    samples_collected_ = 0;
}

bool AudioManager::waitForAudioSegment(std::vector<float>& audio_context) {
    // Wait until we have collected 8 seconds of audio
    size_t required_samples = sample_rate_ * segment_duration_seconds_;
    
    while (samples_collected_ < required_samples) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if (!running_) return false;
    }
    
    {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // If we have more than 8 seconds, just take the most recent 8 seconds
        if (audio_buffer.size() > required_samples) {
            audio_context.assign(
                audio_buffer.end() - required_samples,
                audio_buffer.end()
            );
        } else {
            audio_context = audio_buffer;
        }
    }
    
    return !audio_context.empty();
}

bool AudioManager::pollEvents() {
    return running_;
}

void AudioManager::cleanup() {
    stop();
}

bool AudioManager::saveAudioSegment(const std::vector<float>& audio_data, int segment_count) {
    std::string filename = "audio_input_" + std::to_string(segment_count) + ".wav";
    std::ofstream wav_file(filename, std::ios::binary);
    
    if (!wav_file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }
    
    // Write WAV header
    writeWavHeader(wav_file, audio_data.size() * sizeof(float));
    
    // Write audio data
    wav_file.write(reinterpret_cast<const char*>(audio_data.data()), 
                  audio_data.size() * sizeof(float));
    
    wav_file.close();
    std::cout << "Saved audio segment to " << filename << std::endl;
    return true;
}

bool AudioManager::saveTextOutput(const std::string& text, int segment_count) {
    std::string filename = "text_output_" + std::to_string(segment_count) + ".txt";
    std::ofstream text_file(filename);
    
    if (!text_file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }
    
    text_file << text;
    text_file.close();
    std::cout << "Saved transcription to " << filename << std::endl;
    return true;
}

void AudioManager::_receive_loop() {
    const size_t buffer_size = 2048;
    char buffer[buffer_size];
    struct sockaddr_in sender_addr;
    socklen_t sender_addr_size = sizeof(sender_addr);
    
    while (running_) {
        try {
            int received_bytes = recvfrom(sock_, buffer, buffer_size, 0,
                                        (struct sockaddr *)&sender_addr, &sender_addr_size);
            
            if (received_bytes > 0) {
                // Process as int16 data
                int16_t *int16_data = reinterpret_cast<int16_t *>(buffer);
                int sample_count = received_bytes / 2; // Each sample is 2 bytes
                
                // Check for leading zeros and skip if necessary
                int start_idx = 0;
                if (sample_count >= 2 && int16_data[0] == 0 && int16_data[1] == 0) {
                    start_idx = 2;
                }
                
                // Convert int16 samples to float and add to buffer
                convertInt16ToFloat(&int16_data[start_idx], sample_count - start_idx);
            }
        } catch (const std::exception &e) {
            std::cerr << "Error receiving data: " << e.what() << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

void AudioManager::convertInt16ToFloat(const int16_t* int16_data, size_t sample_count) {
    if (sample_count == 0) return;
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Reserve space to avoid reallocations
    size_t original_size = audio_buffer.size();
    audio_buffer.resize(original_size + sample_count);
    
    // Convert int16 to float [-1.0, 1.0]
    for (size_t i = 0; i < sample_count; i++) {
        // Normalize to [-1, 1] range
        audio_buffer[original_size + i] = int16_data[i] / 32768.0f;
    }
    
    samples_collected_ += sample_count;
}

void AudioManager::writeWavHeader(std::ofstream& file, size_t data_size_bytes) {
    // RIFF header
    file.write("RIFF", 4);
    uint32_t file_size = 36 + data_size_bytes;
    file.write(reinterpret_cast<const char*>(&file_size), 4);
    file.write("WAVE", 4);

    // fmt subchunk
    file.write("fmt ", 4);
    uint32_t subchunk1_size = 16;
    uint16_t audio_format = 3;  // IEEE float
    uint16_t num_channels = 1;  // Mono
    uint32_t byte_rate = sample_rate_ * num_channels * sizeof(float);
    uint16_t block_align = num_channels * sizeof(float);
    uint16_t bits_per_sample = 32;  // 32-bit float

    file.write(reinterpret_cast<const char*>(&subchunk1_size), 4);
    file.write(reinterpret_cast<const char*>(&audio_format), 2);
    file.write(reinterpret_cast<const char*>(&num_channels), 2);
    file.write(reinterpret_cast<const char*>(&sample_rate_), 4);
    file.write(reinterpret_cast<const char*>(&byte_rate), 4);
    file.write(reinterpret_cast<const char*>(&block_align), 2);
    file.write(reinterpret_cast<const char*>(&bits_per_sample), 2);

    // data subchunk
    file.write("data", 4);
    file.write(reinterpret_cast<const char*>(&data_size_bytes), 4);
}
