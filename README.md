# OrangePI_OLED
Add OLED display with sys info to your Orange PI board


**Display connection:**

| OLED |    Name   |  Pin  | 
|:----:|:---------:|:-----:|
|`VDD` |    3.3v   |  1    |
|`SDA` |   SDA.0   |  3    |
|`SCK` |   SCL.0   |  5    |
|      |  GPIO.7   |  7    |
|`GND` |    0v     |  9    |


To run app:

1. `cd /<path to project>`
2. `make`
3. `./build/OrangePI_ssd1306`

To run app at boot time:

1. `sudo crontab -e`
2. add row: `@reboot /<path to project>/build/OrangePI_ssd1306`
3. reboot board & enjoy :)


Tested with Armbian image https://www.armbian.com/download/, Orange PI Lite and Orange PI Zero boards.

![How it works](https://github.com/vadzimyatskevich/OrangePI_OLED/blob/master/img/pic_1.JPG?raw=true)
![How it works](https://github.com/vadzimyatskevich/OrangePI_OLED/blob/master/img/pic_2.JPG?raw=true)
