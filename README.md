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


To run app at boot time:
1. sudo crontab -e
2. add row: @reboot /<path to project>/ssd1306/build/OrangePI_ssd1306

