# Quick Start

First, git clone the repository:
```bash
git https://github.com/JinZhou5042/whisper-audio-streaming.git
cd whisper-audio-streaming
```

This repository depends on the whisper.cpp project, any guidance about the project can be found [here](https://github.com/ggerganov/whisper.cpp). Before building the project, go to the whisper.cpp directory, download the model needed and compile the project:
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

