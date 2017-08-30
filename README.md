# OrangePI_OLED
Add OLED display with sys info to your Orange PI board


**connection Orange PI Lite:**

| OLED |    Name   |  Pin  |
|:----:|:---------:|:-----:|
|`VDD` |    3.3v   |  1    |
|`SDA` |   SDA.0   |  3    |
|`SCK` |   SCL.0   |  5    |
|`GND` |    0v     |  9    |

Any other H3 and H2+ boards use i2c-0 pins

To run app:

1. `cd /<path to project>`
2. `make`
3. `sudo ./build/OrangePI_ssd1306`

To run app at boot time:

1. `sudo crontab -e`
2. add row: `@reboot /<path to project>/build/OrangePI_ssd1306`
3. reboot board & enjoy :)


Tested with Armbian image https://www.armbian.com/download/, Orange PI Lite and Orange PI Zero boards.
[![How it works](https://github.com/vadzimyatskevich/OrangePI_OLED/blob/master/img/pic_1.JPG)](https://www.youtube.com/watch?v=xUK7WmqTY78)
![How it works](https://github.com/vadzimyatskevich/OrangePI_OLED/blob/master/img/pic_2.jpg?raw=true)
