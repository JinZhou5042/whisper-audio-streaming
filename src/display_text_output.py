import os
import errno
import select
import asyncio
import time
from frameutils import Bluetooth
from frame_sdk import Frame
from frame_sdk.display import PaletteColors


FIFO_PATH = "/tmp/whisper_fifo"
HOME_PATH = os.getenv("HOME")


async def display(fn):
    async with Frame() as frame:
        b = Bluetooth()

        with open(fn, "r", encoding = "utf-8") as f:
            content = f.read()
        print(f"Fetched content: {content}")

        print(f"Displaying content...")
        start_time = time.time()
        await frame.display.scroll_text(content, color=PaletteColors.YELLOW)
        end_time = time.time()
        delay = end_time - start_time

        print(f"Completed")
        print(f"Latency: {delay:.4f}s")


print("Reading from FIFO...")

with open(FIFO_PATH) as fifo:
    print("FIFO opened")
    while True:
        select.select([fifo], [], [fifo])
        fn = fifo.read().strip()
        if fn == "":
            continue
        print(f"Fetched log file name: {fn}")
        fn = HOME_PATH + "/project/whisper-audio-streaming/build/" + fn
        if not os.path.exists(fn):
            print(f"Error: file {fn} does not exist")
            exit(1)

        asyncio.run(display(fn))
        
