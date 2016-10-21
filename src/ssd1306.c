/*
 +-----+-----+----------+------+---+--OrangePiPC--+---+------+---------+-----+--+
 | BCM | wPi |   Name   | Mode | V | Physical | V | Mode | Name     | wPi | BCM |
 +-----+-----+----------+------+---+----++----+---+------+----------+-----+-----+
 |     |     |     3.3v |      |   |  1 || 2  |   |      | 5v       |     |     |
 |   2 |   8 |    SDA.0 | ALT5 | 0 |  3 || 4  |   |      | 5V       |     |     |
 |   3 |   9 |    SCL.0 | ALT5 | 0 |  5 || 6  |   |      | 0v       |     |     |
 |   4 |   7 |   GPIO.7 |  OUT | 1 |  7 || 8  | 0 | ALT3 | TxD3     | 15  | 14  |
 |     |     |       0v |      |   |  9 || 10 | 0 | ALT3 | RxD3     | 16  | 15  |
 |  17 |   0 |     RxD2 |  OUT | 0 | 11 || 12 | 0 | ALT3 | GPIO.1   | 1   | 18  |
 |  27 |   2 |     TxD2 | ALT3 | 0 | 13 || 14 |   |      | 0v       |     |     |
 |  22 |   3 |     CTS2 | ALT3 | 0 | 15 || 16 | 0 | ALT3 | GPIO.4   | 4   | 23  |
 |     |     |     3.3v |      |   | 17 || 18 | 0 | ALT3 | GPIO.5   | 5   | 24  |
 |  10 |  12 |     MOSI | ALT4 | 0 | 19 || 20 |   |      | 0v       |     |     |
 |   9 |  13 |     MISO | ALT4 | 0 | 21 || 22 | 0 | ALT3 | RTS2     | 6   | 25  |
 |  11 |  14 |     SCLK | ALT4 | 0 | 23 || 24 | 0 | ALT4 | CE0      | 10  | 8   |
 |     |     |       0v |      |   | 25 || 26 | 0 | ALT3 | GPIO.11  | 11  | 7   |
 |   0 |  30 |    SDA.1 | ALT4 | 0 | 27 || 28 | 0 | ALT4 | SCL.1    | 31  | 1   |
 |   5 |  21 |  GPIO.21 | ALT3 | 0 | 29 || 30 |   |      | 0v       |     |     |
 |   6 |  22 |  GPIO.22 | ALT3 | 0 | 31 || 32 | 0 | ALT3 | RTS1     | 26  | 12  |
 |  13 |  23 |  GPIO.23 | ALT3 | 0 | 33 || 34 |   |      | 0v       |     |     |
 |  19 |  24 |  GPIO.24 | ALT3 | 0 | 35 || 36 | 0 | ALT3 | CTS1     | 27  | 16  |
 |  26 |  25 |  GPIO.25 | ALT3 | 0 | 37 || 38 | 0 | ALT3 | TxD1     | 28  | 20  |
 |     |     |       0v |      |   | 39 || 40 | 0 | ALT3 | RxD1     | 29  | 21  |
 +-----+-----+----------+------+---+----++----+---+------+----------+-----+-----+
 | BCM | wPi |   Name   | Mode | V | Physical | V | Mode | Name     | wPi | BCM |
 +-----+-----+----------+------+---+--OrangePIPC--+------+----------+-----+-----+
*/
/*
A user-space program to get data from an I2C device.
                Gustavo Zamboni
*/
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
       #include <sys/types.h>
       #include <ifaddrs.h>
       #include <arpa/inet.h>
       #include <sys/socket.h>
       #include <netdb.h>
       #include <linux/if_link.h>


#include "ssd1306.h"

#define NI_MAXHOST 1025 
#define NI_MAXSERV 32

char buf[10];
 int com_serial;
 int failcount;
 
int bus; //i2c bus descriptor
FONT_INFO    *_font; 
// oled buffer
static uint8_t    buffer[SSD1306_LCDWIDTH * SSD1306_LCDHEIGHT / 8];
// ovelay buffer
#ifdef MULTILAYER  
static uint8_t    buffer_ol[SSD1306_LCDWIDTH * SSD1306_LCDHEIGHT / 8];
#endif    

static uint8_t      _width    = SSD1306_LCDWIDTH;
static uint8_t      _height   = SSD1306_LCDHEIGHT;
       FONT_INFO    *_font;
 
void i2c_write( uint8_t addr, 
                uint8_t * value, 
                int nbytes);



//////////
// Init I2Cdevice
//////////
int i2c_init(char * bus, int addr)
{
    int file;

    if ((file = open(bus, O_RDWR)) < 0)
    {
        printf("Failed to open the bus.\n");
        /* ERROR HANDLING; you can check errno to see what went wrong */
        com_serial=0;
        exit(EXIT_FAILURE);
    }
 
    if (ioctl(file, I2C_SLAVE, addr) < 0)
        {
            printf("Failed to acquire bus access and/or talk to slave.\n");
            /* ERROR HANDLING; you can check errno to see what went wrong */
            com_serial=0;
            exit(EXIT_FAILURE);
        }
    printf("i2c_init OK. %d \n", file);
    return file;
}
 
/*
 
 
 */ 
void  ssd1306Init(uint8_t vccstate)
{
  _font = (FONT_INFO*)&ubuntuMono_24ptFontInfo;
  
// Initialisation sequence
   ssd1306TurnOff();
//  1. set mux ratio
    ssd1306Command(   SSD1306_SETMULTIPLEX );
    ssd1306Command(   0x3F );
//  2. set display offset
    ssd1306Command(   SSD1306_SETDISPLAYOFFSET );
    ssd1306Command(   0x0 );
//  3. set display start line
    ssd1306Command(   SSD1306_SETSTARTLINE | 0x0 ); 
    ssd1306Command( SSD1306_MEMORYMODE);                    // 0x20
    ssd1306Command( 0x00);                                  // 0x0 act like ks0108
//  4. set Segment re-map A0h/A1h    
    ssd1306Command(   SSD1306_SEGREMAP | 0x1);
//   5. Set COM Output Scan Direction C0h/C8h
    ssd1306Command(   SSD1306_COMSCANDEC);
//  6. Set COM Pins hardware configuration DAh, 12
    ssd1306Command(   SSD1306_SETCOMPINS);
    ssd1306Command(   0x12);
//  7. Set Contrast Control 81h, 7Fh
    ssd1306Command(   SSD1306_SETCONTRAST );
    if (vccstate == SSD1306_EXTERNALVCC) { 
        ssd1306Command(   0x9F );
    } else { 
        ssd1306Command(   0xff );
    }
//  8. Disable Entire Display On A4h
    ssd1306Command(   SSD1306_DISPLAYALLON_RESUME);
//  9. Set Normal Display A6h 
    ssd1306Command(   SSD1306_NORMALDISPLAY);
//  10. Set Osc Frequency  D5h, 80h 
    ssd1306Command(   SSD1306_SETDISPLAYCLOCKDIV);
    ssd1306Command(   0x80);
//  11. Enable charge pump regulator 8Dh, 14h 
    ssd1306Command(   SSD1306_CHARGEPUMP );
    if (vccstate == SSD1306_EXTERNALVCC) { 
        ssd1306Command(   0x10);
    } else { 
        ssd1306Command(   0x14);
    }
//  12. Display On AFh 
    ssd1306TurnOn();

}

/**
 *  \brief Renders the contents of the pixel buffer on the LCD
 *  
 *  \return n/a
 *  
 *  \details n/a
 */
void ssd1306Refresh(void) 
{
  ssd1306Command(SSD1306_SETLOWCOLUMN  | 0x0);  // low col = 0
  ssd1306Command(SSD1306_SETHIGHCOLUMN | 0x0);  // hi col = 0
  ssd1306Command(SSD1306_SETSTARTLINE  | 0x0);  // line #0

  #ifdef MULTILAYER
  ssd1306MixFrameBuffer();
  ssd1306Data( (uint8_t *)buffer_ol, sizeof(buffer_ol)  );
  #else
  HAL_SPI_Transmit  ( &hspi2,  (uint8_t *) buffer, sizeof(buffer), 200) ;
  ssd1306Data( (uint8_t *)buffer, sizeof(buffer)  );
  #endif

}

/**
 *  \brief Enable the OLED panel
 *  
 *  \return n/a
 *  
 *  \details send enable command to the display controller
 */
void ssd1306TurnOn(void)
{
//    CMD(SSD1306_DISPLAYON);
        ssd1306Command(   SSD1306_DISPLAYON );
}
/**
 *  \brief Disable the OLED panel
 *  
 *  \return n/a
 *  
 *  \details n/a
 */
void ssd1306TurnOff(void)
{
//    CMD(c);
    ssd1306Command(   SSD1306_DISPLAYOFF );
}

void  ssd1306Command(  uint8_t comm  ) {
    uint8_t *ptr;

    ptr = calloc( 2, sizeof(uint8_t));     // memory allocated using calloc, add + com + data
    if(ptr == NULL) {
        printf("Error! memory not allocated.\n");
        exit(0);
    }
    *ptr = 0x00;                               // first send "Control byte"
    *(ptr+1) = comm;                           // then send command
    i2c_write((uint8_t)SSD1306_ADDRESS, (uint8_t *)ptr, 2);
    free(ptr);
}

void  ssd1306Data( uint8_t *data, int size  ) {
    uint8_t *ptr;

    ptr = calloc( size + 1, sizeof(uint8_t));     // memory allocated using calloc, add + com + data
    if(ptr == NULL) {
        printf("Error! memory not allocated.\n");
        exit(0);
    }
    *ptr = 0x40;                               // first send "Control byte"
    memcpy( ptr + 1, data, size ); 
//    printf("ssd1306Data! ptr: %hhx, size: %d\n", *ptr, size);
    ssd1306Command(SSD1306_SETLOWCOLUMN  | 0x0 );
    ssd1306Command(SSD1306_SETHIGHCOLUMN | 0x0 );
    ssd1306Command(SSD1306_SETSTARTLINE  | 0x0 );
    i2c_write((uint8_t)SSD1306_ADDRESS, (uint8_t *)ptr, (size + 1));
    free(ptr);
}
/**
 *  \brief Clears the screen buffer
 *  
 *  \param[in] layer to clear
 *  \return Return_Description
 *  
 */
void ssd1306ClearScreen(uint16_t layer) 
{
  if (layer & 0x1)
  memset(buffer, 0x00, 1024);
#ifdef MULTILAYER  
  if (layer & 0x2)
  memset(buffer_ol, 0x00, 1024);
#endif
}
/**
 *  \brief Draws a single pixel in image buffer
 *  \param x The x position (0..127)
 *  \param y The y position (0..63)
 *  \param color The color (BLACK, WHITE, INVERSE)

 */
void   ssd1306DrawPixel(int16_t x, int16_t y, uint16_t color, uint16_t layer) 
{
  if ((x >= SSD1306_LCDWIDTH) || (x < 0) || (y >= SSD1306_LCDHEIGHT) || (y < 0))
  return;

  if (layer & LAYER0)
  switch (color) 
  {
    case WHITE:   buffer[x+ (y/8)*SSD1306_LCDWIDTH] |=  (1 << (y&7)); break;
    case BLACK:   buffer[x+ (y/8)*SSD1306_LCDWIDTH] &= ~(1 << (y&7)); break; 
    case INVERSE: buffer[x+ (y/8)*SSD1306_LCDWIDTH] ^=  (1 << (y&7)); break; 
  }
#ifdef MULTILAYER  
  if (layer & LAYER1)
  switch (color) 
  {
    case WHITE:   buffer_ol[x+ (y/8)*SSD1306_LCDWIDTH] |=  (1 << (y&7)); break;
    case BLACK:   buffer_ol[x+ (y/8)*SSD1306_LCDWIDTH] &= ~(1 << (y&7)); break; 
    case INVERSE: buffer_ol[x+ (y/8)*SSD1306_LCDWIDTH] ^=  (1 << (y&7)); break; 
  }
#endif
}

/**
 *  \brief Brief
 *  
 *  \param[in] x0 point 1 x coord
 *  \param[in] y0 point 1 y coord
 *  \param[in] x1 point 2 x coord
 *  \param[in] y1 point 2 y coord
 *  \param[in] color The color (BLACK, WHITE, INVERSE)
 *  \param[in] layer The layer to draw (LAYER0, LAYER1)
 *  \return void
 *  
 *  \details Details
 */
void ssd1306DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color, uint16_t layer) {
  int16_t steep = (abs(y1 - y0) > abs(x1 - x0));
  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }

  if (x0 > x1) {
    swap(x0, x1);
    swap(y0, y1);
  }

  int16_t dx, dy;
  dx            = x1 - x0;
  dy            = abs(y1 - y0);

  int16_t err   = dx / 2;
  int16_t ystep;

  if (y0 < y1) {
    ystep       = 1;
  } else {
    ystep       = -1;
  }

  for (; x0<=x1; x0++) {
    if (steep) {
      ssd1306DrawPixel(y0, x0, color, layer);
    } else {
      ssd1306DrawPixel(x0, y0, color, layer);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}

/** 
 *    @brief Draw a character
 */
void  ssd1306SetFont(FONT_INFO * f) {
  _font = f;
}

/**
 *    @brief Draws a filled rectangle
 */
void ssd1306FillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color, uint16_t layer) {
	uint8_t x0, x1, y1;

	x0 = x;
	x1 = x + w;
	y1 = y + h;
	for(; y < y1; y++)
			for(x = x0; x < x1; x++)
					ssd1306DrawPixel( x, y, color, layer);
}

/*!
 *    @brief Draws a rectangle
 */
void ssd1306DrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color, uint16_t layer) {
	int16_t x1, y1;
	if ( (w == 0) | (h == 0)) return;
	x1 = x + w - 1;
	y1 = y + h - 1;
	
	if((h > 2 ) | (w > 2)) {
		ssd1306DrawLine(	 x,    y,   x1,  y, color, layer);
		ssd1306DrawLine(	 x,   y1,   x1, y1, color, layer);
		ssd1306DrawLine(	 x,  y+1,  x, y1-1, color, layer);
		ssd1306DrawLine(  x1,  y+1, x1, y1-1, color, layer);
	} else {
		ssd1306DrawLine(	 x,    y,   x1,  y, color, layer);
		ssd1306DrawLine(	 x,   y1,   x1, y1, color, layer);
	}
}

/** 
 *    @brief Draw a character
 */
int16_t  ssd1306DrawChar(int16_t x, int16_t y, uint8_t c, uint8_t size, 
uint16_t color, uint16_t layer) {
      int16_t i,j,k, _x;
      uint16_t line;

  if( (c < _font->startChar) ||   // skip if character don't exist
      (c > _font->endChar))        // skip if character don't exist
  return 0;
  
  c = c - _font->startChar;
  // skip invisible characters
//  if( (x >= _width)            ||   // Clip right
//      (y >= _height)           ||   // Clip bottom
//      ((x + _font->charInfo[c].widthBits * size - 1) < 0) || // Clip left
//      ((y + _font->charInfo[c].heightBits * size - 1) < 0))  // Clip top
//  return 0;
  
  line = _font->charInfo[c].offset; 
  // scan height
  for ( i=0; i < _font->charInfo[c].heightBits; i++ ) {
      k =  (_font->charInfo[c].widthBits-1)/8 + 1; //number of bytes in a row
      _x = 0;
      do {
        int16_t l = _font->data[line]; 
          // scan width
          for ( j = 0; j < 8; j++ ) {
            if ( l & 0x80 ) {
                if (size == 1) {
                    ssd1306DrawPixel( x+_x, y+i, color, layer ); 
                } else {
                    ssd1306DrawRect(x+(_x*size), y+(i*size), size, size, color, layer);
                }
              
            }
          l <<= 1;
          _x++;
        }
        k--;
        line++;
      } while (k > 0);
  }
//    ssd1306Refresh();
  return _font->charInfo[c].widthBits; // return characher width
}

/**
 *    @brief  Draws a string using the supplied font data.
 *    @param[in]  x
 *                Starting x co-ordinate
 *    @param[in]  y
 *                Starting y co-ordinate
 *    @param[in]  text
 *                The string to render
 *    @param[in]  font
 *                Pointer to the FONT_DEF to use when drawing the string
 *    @section Example
 *    @code 
 *    // Refresh the screen to see the results
 *    ssd1306Refresh();
 *    @endcode
*/
void  ssd1306DrawString(int16_t x, int16_t y, uint8_t *text, uint8_t size, 
                        uint16_t color, uint16_t layer)
{
  static uint16_t l, pos;
  pos =  0;
  for (l = 0; l < strlen(text); l++)
  {
//    tmp = text[l] - _font->startChar;//
    pos = pos + ssd1306DrawChar(x + (pos * size + 1), y, text[l], size, color, layer);
  }
}

//////////
// Set pointer address
//////////
//void i2c_set_pointer(int address,int value,int bus)
//    {
//    /*
//    //printf("end: 0x%x%x\n", address,value);
//    if (i2c_smbus_write_byte_data(file, address, value)<0)
//        {
//        fprintf(stderr, "Warning - write failed\n");
//        } 
//    */
//    char buf[10];
//    buf[0]=address;
//    buf[1]=value;
// 
//    if (write(bus, buf, 2) != 2)
//        {
//        fprintf(stderr, "Error setting pointer\n");
//        com_serial=0;
//        failcount++;
//        } 
//    else
//        {
//        //printf("w_0x%0*x\n", 2, buf[0]);
//        //printf("w_0x%0*x\n", 2, buf[1]);
//        com_serial=1;
//        failcount=0;
//        }
// 
//    }
 
//////////
// Read n bytes
//////////
// 
//char * i2c_read(int add1, int add2, int nbytes,int bus)
//    {
//    int n;
// 
//    i2c_set_pointer(add1,add2,bus);
// 
//    if (read(bus, buf, nbytes) != nbytes)
//        {
//        fprintf(stderr, "Error reading %i bytes\n",nbytes);
//        com_serial=0;
//        failcount++;
//        } 
//    else
//        {
//        for (n=0;n<nbytes;n++)
//            {
//            //printf("r_0x%0*x\n", 2, buf[n]);
//            }
//        com_serial=1;
//        failcount=0;
//        return buf;
//        }
//    return 0;
//    }

/**
 *  \brief ssd1306MixFrameBuffer
 *  
 *  \return Return_Description
 *  
 *  \details Details
 */
#ifdef MULTILAYER  
void ssd1306MixFrameBuffer(void)
{
  uint16_t i;
  for(i = 0; i < (SSD1306_LCDWIDTH * SSD1306_LCDHEIGHT / 8); i++)
  {
    buffer_ol[i] |= buffer[i];
  }
#endif    
}
//////////
// Write n bytes
//////////
void i2c_write( uint8_t addr, 
                uint8_t * value, 
                int nbytes)
{
//    int i;
    uint8_t *ptr;
    ptr = calloc((nbytes + 1), sizeof(uint8_t));     // memory allocated using calloc add + data
    if(ptr == NULL)
    {
        printf("Error! memory not allocated.\n");
        exit(EXIT_FAILURE);
    }

    *ptr = addr;                                     // first send address
    memcpy( ptr + 1, value, nbytes );                // then some data
    

//    printf("Elements of array: \n");
//    for(i = 0; i < (nbytes+1); ++i)
//    {
//        printf("%#010hhx\n", *(ptr+i));
//    }

    if (write(bus, value, nbytes) != nbytes)
    {
            fprintf(stderr, "Error writing %i bytes\n",nbytes);
            com_serial=0;
            failcount++;
    } 
    free(ptr);
 }

int GetCPULoad() {
   int FileHandler;
   char FileBuffer[1024];
   float load;

   FileHandler = open("/proc/loadavg", O_RDONLY);
   if(FileHandler < 0) {
      return -1; }
   read(FileHandler, FileBuffer, sizeof(FileBuffer) - 1);
   sscanf(FileBuffer, "%f", &load);
   close(FileHandler);
   return (int)(load * 100);
}

float GetMemUsage() {
   int FileHandler;
   char FileBuffer[1024];
   int memTotal;
   int memFree;
   float result;
   FileHandler = open("/proc/meminfo", O_RDONLY);
   if(FileHandler < 0) {
      return -1; }
   read(FileHandler, FileBuffer, sizeof(FileBuffer) - 1);
//   printf("%s", FileBuffer);
   sscanf(FileBuffer, "MemTotal:         %d kB\n MemFree:          %d kB", &memTotal, &memFree);
   close(FileHandler);
   printf("tot %d \nfree %d \n", memTotal, memFree);
   result = (float)memFree / memTotal * 100;
//   printf("result %f \n", result);
   return result;
}

int main (void) {
    int x, y;
    time_t mytime;
    struct tm *tm;
    uint8_t time_buffer[80];
    uint8_t text_buffer[100];    
    struct ifaddrs *ifaddr, *ifa;
    int s, n;
    char host[NI_MAXHOST];
    /* Init I2C bus  */
    bus = i2c_init((char*)&"/dev/i2c-0", 0x3c); //dev, slavei2caddr

    
    ssd1306Init(SSD1306_SWITCHCAPVCC);

    while (1) {
        _font = (FONT_INFO*)&ubuntuMono_8ptFontInfo;
        /* Display time */
        mytime = time(NULL);
        tm = localtime (&mytime);
        ssd1306ClearScreen(LAYER0 | LAYER1) ;
        strftime(time_buffer, 80,"%H:%M %x", tm);
        ssd1306DrawString(0,  55, time_buffer, 1, WHITE, LAYER0); 

        /* Display IP */
        /* Get network information */
        if (getifaddrs(&ifaddr) == -1) 
        {
            perror("getifaddrs");
            exit(EXIT_FAILURE);
        }
        /* Walk through linked list, maintaining head pointer so we
          can free list later */
        for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {
            if ((ifa->ifa_addr == NULL) || (ifa->ifa_addr->sa_family != AF_INET))
                continue;

            /* Display interface name and family (including symbolic
                form of the latter for the common families) */
            if (strcmp ("wlan0", ifa->ifa_name ) == 0) {
                s = getnameinfo(ifa->ifa_addr,
                       (ifa->ifa_addr->sa_family == AF_INET) ? sizeof(struct sockaddr_in) :
                                             sizeof(struct sockaddr_in6),
                       host, NI_MAXHOST,
                       NULL, 0, NI_NUMERICHOST);
                if (s != 0) {
                   printf("getnameinfo() failed: %s\n", gai_strerror(s));
                   exit(EXIT_FAILURE);
                }
                printf("%-8s <%s>\n", ifa->ifa_name, host);
                snprintf ( text_buffer, sizeof(text_buffer), "wlan0: %s", host );
                ssd1306DrawString(0,  47, text_buffer, 1, WHITE, LAYER0); 
//                printf("<%d>\n", n);
//                printf("<%s>\n", text_buffer);
//                break; // IP found, exit loop
            }
            if (strcmp ("eth0", ifa->ifa_name ) == 0) {
                s = getnameinfo(ifa->ifa_addr,
                       (ifa->ifa_addr->sa_family == AF_INET) ? sizeof(struct sockaddr_in) :
                                             sizeof(struct sockaddr_in6),
                       host, NI_MAXHOST,
                       NULL, 0, NI_NUMERICHOST);
                if (s != 0) {
                   printf("getnameinfo() failed: %s\n", gai_strerror(s));
                   exit(EXIT_FAILURE);
                }
                printf("%-8s <%s>\n", ifa->ifa_name, host);
                snprintf ( text_buffer, sizeof(text_buffer), "eth0:  %s", host );
                ssd1306DrawString(0,  39, text_buffer, 1, WHITE, LAYER0); 
//                printf("<%d>\n", n);
//                printf("<%s>\n", text_buffer);
//                break; // IP found, exit loop
            }
        }
        freeifaddrs(ifaddr);
        /* CPU usage
         */
        _font = (FONT_INFO*)&ubuntuMono_16ptFontInfo;
        int c = GetCPULoad() ;
        snprintf ( text_buffer, sizeof(text_buffer), "CPU: %3d%%", c );
        printf("%s\n", text_buffer);
        ssd1306DrawString(0,  0, text_buffer, 1, WHITE, LAYER0); 
        /* Memory usage
         */
        float m = GetMemUsage();
        snprintf ( text_buffer, sizeof(text_buffer), "Mem: %3.0f%%", m );
        printf("%s\n", text_buffer);
        ssd1306DrawString(0,  16, text_buffer, 1, WHITE, LAYER0); 
        /* Refresh screen
         */
        ssd1306Refresh();
        SSD1306MSDELAY(1000);
    }

    printf("All done!\r\n");
    close(bus);
    return 0;
}
