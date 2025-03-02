# Quick Start

This project depends on [whisper.cpp](https://github.com/ggerganov/whisper.cpp.git). Before proceeding, ensure you have cloned the repository with submodules:

```bash
git clone --recursive https://github.com/JinZhou5042/whisper-audio-streaming.git
cd whisper-audio-streaming
```

If you cloned without submodules, initialize them manually:

```bash
git submodule update --init --recursive
```

## 1. Set Up Whisper.cpp

Download & Compile the Model

Navigate to the whisper.cpp directory:

```bash
cd third_party/whisper.cpp
```

For Linux, download the GGML model:

```bash
sh ./models/download-ggml-model.sh base.en
```


For macOS (Recommended), use the CoreML backend for a significant performance boost:

```bash
pip install ane_transformers
pip install openai-whisper
pip install coremltools

sh ./models/download-ggml-model.sh base.en
sh ./models/generate-coreml-model.sh base.en
```

At this point, whisper.cpp is fully set up.

## 2. Install Dependencies

Before building the project, ensure required libraries are installed:

macOS

```bash
brew install sdl2 curl fftw
```

Ubuntu

```bash
sudo apt-get install libsdl2-dev libcurl4-openssl-dev libfftw3-dev
```

## 3. Build the Project

Return to the main directory and build using CMake:

```bash
cmake -B build
cd build && make -j4
```

## 4. Run the Program

After a successful build, execute:

```bash
./bin/stream
```

## 5. Configuration: Enable Translation

For language translation, set your Google Translate API key:

```bash
export GOOGLE_TRANSLATE_API_KEY="your-api-key-here"
```

Now, your setup is complete! 🚀