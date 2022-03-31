# OrangePI_OLED
Add OLED display with sys info to your Orange PI board


**connection Orange PI Lite:**

| OLED |    Name   |  Pin  |
|:----:|:---------:|:-----:|
|`VDD` |    3.3v   |  1    |
|`SDA` |   SDA.0   |  3    |
|`SCK` |   SCL.0   |  5    |
|`GND` |    0v     |  9    |

May work on other H5, H3 and H2+ boards uder Armbian. Use i2c-0 pins.To change I2C bus make corresponding changes in main.c row 127 (`bus = i2c_init((char*)&"/dev/i2c-0", 0x3c);`)

To run app:

1. `git clone https://github.com/vadzimyatskevich/SSD1306.git`
1. `cd SSD1306`
2. `make`
3. `sudo ./build/ArmbianOLED`

To run app at boot time:

1. `sudo crontab -e`
2. add row: `@reboot /home/<your username here>/SSD1306/build/ArmbianOLED` ex `@reboot /home/orangepi/SSD1306/build/ArmbianOLED`
3. reboot board & enjoy :)


Tested with Armbian image https://www.armbian.com/download/, Orange PI Lite and Orange PI Zero boards.
[![How it works](https://github.com/vadzimyatskevich/OrangePI_OLED/blob/master/img/pic_1.JPG)](https://www.youtube.com/watch?v=xUK7WmqTY78)
![How it works](https://github.com/vadzimyatskevich/OrangePI_OLED/blob/master/img/pic_2.jpg?raw=true)
