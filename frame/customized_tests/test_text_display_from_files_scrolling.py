import asyncio
import time
from frameutils import Bluetooth
from frame_sdk import Frame
from frame_sdk.display import PaletteColors
import os

async def main():
    async with Frame() as frame:
        b = Bluetooth()

        index = 1  # Start from lyrics1.txt

        while True:
            filename = f"lyrics{index}.txt"
            if not os.path.exists(filename):
                print(f"文件 {filename} 不存在，结束读取")
                break
            print(f"\n开始读取文件: {filename}")

            with open(filename, "r", encoding="utf-8") as f:
                content = f.read()
                start_time = time.time()
                await frame.display.scroll_text(content,color=PaletteColors.YELLOW)
                end_time = time.time()
                delay = end_time - start_time
                print(f"发送: {content}")
                print(f"延迟: {delay:.4f} 秒")
                index += 1  # move to the next file


# Run the main function
asyncio.run(main())
