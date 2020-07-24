/* 
 * File:   main.c
 * Author: vadzim
 *
 * Created on December 9, 2016, 3:09 PM
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


#define NI_MAXHOST 1025 
#define NI_MAXSERV 32

volatile sig_atomic_t done = 0;
struct sigaction action;

extern FONT_INFO    *_font; 
extern int bus; //i2c bus descriptor

/**
 * 
 * @return 
 */
float GetCPULoad() {
    int FileHandler;
    char FileBuffer[1024];
    float load;

    FileHandler = open("/proc/loadavg", O_RDONLY);
    if(FileHandler < 0) {
        return -1; }
    read(FileHandler, FileBuffer, sizeof(FileBuffer) - 1);
    sscanf(FileBuffer, "%f", &load);
    close(FileHandler);
    return load;
}

/**
 * 
 * @return 
 */
float GetMemUsage() {
    int FileHandler;
    char FileBuffer[1024];
    int memTotal, memFree, memAvail, memBuf,memCached;
    float result;
    FileHandler = open("/proc/meminfo", O_RDONLY);
    if(FileHandler < 0) {
        return -1; }
    read(FileHandler, FileBuffer, sizeof(FileBuffer) - 1);
    sscanf(FileBuffer, "MemTotal:         %d kB\n MemFree:          %d kB\n MemAvailable:          %d kB\n Buffers:           %d kB\n Cached:           %d kB",
     &memTotal, &memFree, &memAvail, &memBuf, &memCached);
    close(FileHandler);
    result = 1.0 - (float)(memFree + memCached) / memTotal;
    return result;
}
/**
 * 
 * @return 
 */
int GetCPUTemp() {
   int FileHandler;
   char FileBuffer[1024];
   int CPU_temp;
   FileHandler = open("/sys/devices/virtual/thermal/thermal_zone0/temp", O_RDONLY);
   if(FileHandler < 0) {
      return -1; }
   read(FileHandler, FileBuffer, sizeof(FileBuffer) - 1);
   sscanf(FileBuffer, "%d", &CPU_temp);
   close(FileHandler);
   return CPU_temp / 1000;
}

void sig_handler(int signo)
{
    done = 1;
    printf("received signo :%d \n", signo);
}


/*
 * 
 */
int main(int argc, char** argv) {
    time_t mytime;
    struct tm *tm;
    uint8_t time_buffer[80];
    uint8_t text_buffer[100];    
    struct ifaddrs *ifaddr, *ifa;
    int family, s, n, row;
    char host[NI_MAXHOST];

    
    // Print pid, so that we can send signals from other shells
    printf("My pid is: %d\n", getpid());    
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = sig_handler;
    // Intercept SIGHUP and SIGINT
    if (sigaction(SIGINT, &action, NULL) == -1) {
        perror("Error: cannot handle SIGINT"); // Should not happen
    }
    if (sigaction(SIGTERM, &action, NULL) == -1) {
        perror("Error: cannot handle SIGTERM"); // Should not happen
    }

    /* Init I2C bus  */
    bus = i2c_init((char*)&"/dev/i2c-0", 0x3c); //dev, slavei2caddr
    /* */
    ssd1306Init(SSD1306_SWITCHCAPVCC);

    while (done == 0) {
        row = 2;
        _font = (FONT_INFO*)&ubuntuMono_8ptFontInfo;
        /* Display time */
        mytime = time(NULL);
        tm = localtime (&mytime);
        ssd1306ClearScreen(LAYER0 | LAYER1) ;
        strftime(time_buffer, 80,"  %H:%M:%S %x", tm);
        ssd1306DrawString(0,  row * 8, time_buffer, 1, WHITE, LAYER0);
        row++;

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
            if (ifa->ifa_addr == NULL)
                continue;
            
            family = ifa->ifa_addr->sa_family;
            /* Display interface name and family (including symbolic
                form of the latter for the common families) */
            if ((strncmp ("eth", ifa->ifa_name, 3 ) == 0) && family == AF_INET) {
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
                snprintf ( text_buffer, sizeof(text_buffer), "%s: %s",ifa->ifa_name, host );
                ssd1306DrawString(0,  row * 8, text_buffer, 1, WHITE, LAYER0); 
                row++;
            } 
            if ((strncmp ("wlan", ifa->ifa_name, 4 ) == 0) && family == AF_INET) {
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
                snprintf ( text_buffer, sizeof(text_buffer), "%s: %s",ifa->ifa_name, host );
                ssd1306DrawString(0,  row * 8, text_buffer, 1, WHITE, LAYER0); 
                row++;
            } 
        }
        freeifaddrs(ifaddr);
        
        /* CPU usage
         * 
         */
        float c = GetCPULoad() ;
        snprintf ( text_buffer, sizeof(text_buffer), "CPU loadavg: %0.2f", c );
        printf("%s\n", text_buffer);
        ssd1306DrawString(0,  row * 8, text_buffer, 1, WHITE, LAYER0); 
        row++;
        
        /* Memory usage */
        float m = GetMemUsage();
        snprintf ( text_buffer, sizeof(text_buffer), "Mem used: %3.0f%%", m*100 );
        printf("%s\n", text_buffer);
        ssd1306DrawString(4,  2, text_buffer, 1, WHITE, LAYER0); 
        ssd1306DrawRect(0, 0, 127, 13, INVERSE, LAYER0);
        ssd1306FillRect(2, 2, (int)(123 * m), 9, INVERSE, LAYER0);
        
        /* CPU temperature  */
        int t = GetCPUTemp() ;
        snprintf ( text_buffer, sizeof(text_buffer), "CPU temp: %3d C", t );
        printf("%s\n", text_buffer);
        ssd1306DrawString(0,  row * 8, text_buffer, 1, WHITE, LAYER0); 
        row++;

        /* Refresh screen */
        ssd1306Refresh();
        SSD1306MSDELAY(1000);
    }


    _font = (FONT_INFO*)&ubuntuMono_24ptFontInfo;
    ssd1306ClearScreen(LAYER0 | LAYER1) ;
    ssd1306Refresh();
    close(bus);
    printf("All done!\r\n");
    return (EXIT_SUCCESS);
}

