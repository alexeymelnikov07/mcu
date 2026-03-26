from PIL import Image
import serial
import time

def main():
    image = Image.open('./image_to_display.png')
    width, height = image.size
    if width>320: width=320
    if height>240: height=240

    ser = serial.Serial(port='COM7', baudrate=115200, timeout=0.0)
    try:
        for i in range (height):
            for j in range (width):
                r, g, b = image.getpixel((i, j))
                color = '#{:02x}{:02x}{:02x}'.format(r, g, b)
                ser.write(f"{i} {j} {color}\n".encode('ascii'))
                time.sleep(0.001)
    finally:
        time.sleep(0.1)
        ser.close()

if __name__ == "__main__":
    main()