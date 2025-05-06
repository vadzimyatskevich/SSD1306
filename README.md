# OrangePI\_OLED

Add an OLED display with system info to your Orange Pi board.

## Connection for Orange Pi Lite

|  OLED |  Name | Pin |
| :---: | :---: | :-: |
| `VDD` |  3.3V |  1  |
| `SDA` | SDA.0 |  3  |
| `SCK` | SCL.0 |  5  |
| `GND` |   0V  |  9  |

This setup may also work on other H5, H3, and H2+ boards running Armbian. Use the I2C-0 pins.

To change the I2C bus, modify line 266 in `main.c`:

```c
    /* Init I2C bus  */
    bus = i2c_init((char *)&"/dev/i2c-0", 0x3c); // dev, slavei2caddr
```

## How to Run the App

1. Clone the repository:

   ```bash
   git clone https://github.com/vadzimyatskevich/SSD1306.git
   ```
2. Navigate into the project directory:

   ```bash
   cd SSD1306
   ```
3. Build the project:

   ```bash
   make
   ```
4. Run the application:

   ```bash
   sudo ./build/ArmbianOLED
   ```

## Run at Boot

1. Edit the root user's crontab:

   ```bash
   sudo crontab -e
   ```
2. Add the following line (replace `<your-username>` with your actual username):

   ```bash
   @reboot /home/<your-username>/SSD1306/build/ArmbianOLED
   ```

   Example:

   ```bash
   @reboot /home/orangepi/SSD1306/build/ArmbianOLED
   ```
3. Reboot your board:

   ```bash
   sudo reboot
   ```

## Tested On

* Orange Pi Lite
* Orange Pi Zero
* Armbian ([https://www.armbian.com/download/](https://www.armbian.com/download/))

## Demo

[![How it works](https://github.com/vadzimyatskevich/OrangePI_OLED/blob/master/img/pic_1.JPG)](https://www.youtube.com/watch?v=xUK7WmqTY78)
![How it works](https://github.com/vadzimyatskevich/OrangePI_OLED/blob/master/img/pic_2.jpg?raw=true)
