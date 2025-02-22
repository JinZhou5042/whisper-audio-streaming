# Quick Start

This repository depends on the whisper.cpp project, any guidance about the project can be found [here](https://github.com/ggerganov/whisper.cpp). First, git clone the repository with submodules:
```bash
git clone --recursive https://github.com/JinZhou5042/whisper-audio-streaming.git
cd whisper-audio-streaming
```

If you have already cloned the repository without submodules, you can initialize and update the submodules by running:
```bash
git submodule update --init --recursive
```

Before building the project, go to the whisper.cpp directory, download the model needed and compile the project:
```bash
cd third_party/whisper.cpp

# download the model
sh ./models/download-ggml-model.sh base.en
# build the project
cmake -B build
cmake --build build --config Release
# transcribe an audio file, there should be some output in the console
./build/bin/whisper-cli -f samples/jfk.wav
```

Then, install various dependencies on your specific platform.

For macOS:
```bash
brew install sdl2 curl fftw
```

For Ubuntu:
```bash
sudo apt-get install libsdl2-dev libcurl4-openssl-dev libfftw3-dev
```

Then, build the project:
```bash
mkdir build
cd build
cmake ..
make -j8
```

Then, run the program:
```bash
./bin/stream
```

## Configuration

Before running the application, you need to set your Google Translate API key:

For macOS/Linux:
```bash
export GOOGLE_TRANSLATE_API_KEY="your-api-key-here"
```
