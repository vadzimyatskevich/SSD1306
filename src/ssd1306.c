/* 
 * File:   newmain.c
 * Author: vadzim
 *
 * Created on November 23, 2016, 9:04 AM
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
#include <signal.h>


#include "ssd1306.h"



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


/**
 * @brief Init I2Cdevice
 * @param bus
 * @param addr
 * @return 
 */
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
 
/**
 * @brief Init display controller
 * @param vccstate
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
 * @brief Renders the contents of the pixel buffer on the LCD
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
 *  @brief Enable the OLED panel
 */
void ssd1306TurnOn(void)
{
    ssd1306Command(   SSD1306_DISPLAYON );
}

/**
 *  @brief Disable the OLED panel
 */
void ssd1306TurnOff(void)
{
//    CMD(c);
    ssd1306Command(   SSD1306_DISPLAYOFF );
}

/**
 * @brief Send command to display
 * @param comm
 */
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

/**
 * @brief Send data to display
 * @param data
 * @param size
 */
void  ssd1306Data( uint8_t *data, int size  ) {
    uint8_t *ptr;

    ptr = calloc( size + 1, sizeof(uint8_t));     // memory allocated using calloc, add + com + data
    if(ptr == NULL) {
        printf("Error! memory not allocated.\n");
        exit(0);
    }
    *ptr = 0x40;                               // first send "Control byte"
    memcpy( ptr + 1, data, size ); 
    ssd1306Command(SSD1306_SETLOWCOLUMN  | 0x0 );
    ssd1306Command(SSD1306_SETHIGHCOLUMN | 0x0 );
    ssd1306Command(SSD1306_SETSTARTLINE  | 0x0 );
    i2c_write((uint8_t)SSD1306_ADDRESS, (uint8_t *)ptr, (size + 1));
    free(ptr);
}

/**
 * @brief Clears the screen buffer
 * @param layer
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
 * @brief Draws a single pixel in image buffer
 * @param x The x position (0..127)
 * @param y The y position (0..63)
 * @param color The color (BLACK, WHITE, INVERSE)
 * @param layer
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
 * 
 * @param x0
 * @param y0
 * @param x1
 * @param y1
 * @param color
 * @param layer
 */
void ssd1306DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color, uint16_t layer) {
    int16_t steep = (abs(y1 - y0) > abs(x1 - x0));
    if (steep) {
        SWAP(x0, y0);
        SWAP(x1, y1);
    }

    if (x0 > x1) {
        SWAP(x0, x1);
        SWAP(y0, y1);
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    int16_t err   = dx / 2;
    int16_t ystep;

    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
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
 * @brief Set font
 * @param f
 */
void  ssd1306SetFont(FONT_INFO * f) {
    _font = f;
}

/**
 * @brief Draws a filled rectangle
 * @param x
 * @param y
 * @param w
 * @param h
 * @param color
 * @param layer
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
        ssd1306DrawLine( x,    y, x1,    y, color, layer);
        ssd1306DrawLine( x,   y1, x1,   y1, color, layer);
        ssd1306DrawLine( x,  y+1,  x, y1-1, color, layer);
        ssd1306DrawLine( x1, y+1, x1, y1-1, color, layer);
    } else {
        ssd1306DrawLine( x,  y, x1,  y, color, layer);
        ssd1306DrawLine( x, y1, x1, y1, color, layer);
    }
}

/**
 * @brief Draw a character
 * @param x
 * @param y
 * @param c
 * @param size
 * @param color
 * @param layer
 * @return 
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
 * @brief  Draws a string using the supplied font data.
 * @param x
 * @param y
 * @param text
 * @param size
 * @param color
 * @param layer
 */
void  ssd1306DrawString(int16_t x, int16_t y, int8_t *text, uint8_t size, 
                        uint16_t color, uint16_t layer)
{
    static uint16_t l, pos;
    pos =  0;
    for (l = 0; l < strlen(text); l++)
    {
        pos = pos + ssd1306DrawChar(x + (pos * size + 1), y, text[l], size, color, layer);
    }
}
 
/**
 * @brief Mix layers
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

/**
 * 
 * @param addr
 * @param value
 * @param nbytes
 */
void i2c_write( uint8_t addr, uint8_t * value, int nbytes)
{
    uint8_t *ptr;
    ptr = calloc((nbytes + 1), sizeof(uint8_t));     // memory allocated using calloc add + data
    if(ptr == NULL)
    {
        printf("Error! memory not allocated.\n");
        exit(EXIT_FAILURE);
    }

    *ptr = addr;                                     // first send address
    memcpy( ptr + 1, value, nbytes );                // then some data
    if (write(bus, value, nbytes) != nbytes)
    {
            fprintf(stderr, "Error writing %i bytes\n",nbytes);
            com_serial=0;
            failcount++;
    } 
    free(ptr);
}
