import asyncio
import os
import sys
import time

from frame_sdk import Frame
from frame_sdk.display import PaletteColors


async def display_text_from_file(filename):
    if not os.path.exists(filename):
        print(f"Error: File {filename} does not exist")
        return False

    print(f"Reading text from: {filename}")

    try:
        with open(filename, "r", encoding="utf-8") as f:
            content = f.read().strip()
            if not content:
                print("Warning: File is empty")
                return True

            print(f"Content: {content}")

            async with Frame() as frame:
                start_time = time.time()
                await frame.display.scroll_text(content, color=PaletteColors.YELLOW)
                end_time = time.time()
                display_time = end_time - start_time

                print(f"Display completed in {display_time:.4f} seconds")

        return True
    except Exception as e:
        print(f"Error displaying text: {e}")
        return False


async def main():
    if len(sys.argv) < 2:
        print("Usage: python display_text_output.py <text_file_path>")
        return

    text_file_path = sys.argv[1]
    await display_text_from_file(text_file_path)


if __name__ == "__main__":
    asyncio.run(main())
