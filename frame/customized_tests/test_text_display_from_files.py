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
                for line in f:
                    line = line.strip()
                    if not line:
                        continue                

                    start_time = time.time()
                    await frame.display.show_text(line, color=PaletteColors.WHITE)
                    #await frame.display.scroll_text(line,color=PaletteColors.YELLOW)
                    end_time = time.time()

                    delay = end_time - start_time
                    print(f"发送: {line}")
                    print(f"延迟: {delay:.4f} 秒")

                    await asyncio.sleep(8)

  
                index += 1  # move to the next file

# Run the main function
asyncio.run(main())
