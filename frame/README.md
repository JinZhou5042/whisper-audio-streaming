## 🔧 Setup Instructions

### 📥 Clone the Repository (with Submodule)

```bash
git clone https://github.com/JinZhou5042/whisper-audio-streaming.git
cd frame
git submodule update --init --recursive # TODO: do we need this?
```


### 🐍 Create a Virtual Environment

```bash
python -m venv env_name
source env_name/bin/activate  # On Windows: env_name\Scripts\activate
```

Then install the required dependencies:

```bash
pip install --upgrade pip
pip install frameutils frame_sdk
```


### 🔄 Pair the Glass via Bluetooth

1. **Charge the device** — while charging, **press and hold** the small circle on the orange part for **5 seconds**.
3. Remove the orange part: you should see **“Ready to Pair Frame E0”** on the glass.
4. Run the Bluetooth pairing script:

```bash
cd customized_tests
python test_bluetooth_callback_api.py
```

After success, you will see:  
> **“Frame is paired Frame E0”** on the glass.

---

### ⚠️ If You Encounter: `Device needs to be re-paired`

1. Open **System Settings > Bluetooth** (on macOS)
2. Locate the device named **"Frame E0"**
3. Click the ℹ️ (info icon) or right-click and select **“Forget This Device”**
4. Then repeat pairing steps (3 and 4 above)

---

### 🔋 Check Glass Battery Level

Make sure you're in the `customized_tests` directory:

```bash
python test_check_battery.py
```

---

### ⏱️ Test Text Display Delay

```bash
python test_text_display_delay.py
```

---

### 📁 Test Text Display From Files

```bash
python test_text_display_from_files.py
```

---

### 🗃️ Test Text Display From Files (Scrolling mode)

```bash
python test_text_display_from_files_scrolling.py
```

