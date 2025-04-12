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
#include <sys/sysinfo.h>
#include "ssd1306.h"
#include "main.h"

#define NI_MAXHOST 1025
#define NI_MAXSERV 32

volatile sig_atomic_t done = 0;
struct sigaction action;
// Глобальные переменные для кэширования
static time_t last_network_update = 0;
static char cached_ip[INET_ADDRSTRLEN] = "";

extern FONT_INFO *_font;
extern int bus; // i2c bus descriptor

/**
 * @brief
 *
 * @param stats
 */
void GetCPUStats(CPUStats *stats)
{
    FILE *file = fopen("/proc/stat", "r");
    if (!file)
    {
        perror("Failed to open /proc/stat");
        exit(1);
    }

    char line[256];
    if (fgets(line, sizeof(line), file))
    {
        sscanf(line, "cpu %lu %lu %lu %lu %lu %lu %lu",
               &stats->user, &stats->nice, &stats->system, &stats->idle,
               &stats->iowait, &stats->irq, &stats->softirq);
    }

    fclose(file);
}

/**
 * @brief
 *
 * @return float
 */
float GetCPULoad()
{
    CPUStats stats1, stats2;

    // Первое измерение
    GetCPUStats(&stats1);
    sleep(1); // Ждем 1 секунду

    // Второе измерение
    GetCPUStats(&stats2);

    // Вычисляем разницу между измерениями
    unsigned long user = stats2.user - stats1.user;
    unsigned long nice = stats2.nice - stats1.nice;
    unsigned long system = stats2.system - stats1.system;
    unsigned long idle = stats2.idle - stats1.idle;
    unsigned long iowait = stats2.iowait - stats1.iowait;
    unsigned long irq = stats2.irq - stats1.irq;
    unsigned long softirq = stats2.softirq - stats1.softirq;

    // Общее время работы CPU
    unsigned long total = user + nice + system + idle + iowait + irq + softirq;
    // Время работы (не idle)
    unsigned long working = user + nice + system + irq + softirq;

    // Процент загрузки
    if (total == 0)
        return 0.0f;
    return (float)working / (float)total * 1.0f;
}

/**
 * @brief Get the Mem Usage object
 *
 * @return float
 */
/**
 * @brief Get current RAM usage as a percentage (0.0 to 1.0)
 * @return RAM usage (0.0–1.0) or -1 on error
 */
float GetMemUsage()
{
    int fd;
    char buffer[1024];
    ssize_t bytesRead;

    // Open /proc/meminfo
    fd = open("/proc/meminfo", O_RDONLY);
    if (fd < 0)
    {
        perror("Failed to open /proc/meminfo");
        return -1.0f;
    }

    // Read the file
    bytesRead = read(fd, buffer, sizeof(buffer) - 1);
    close(fd);

    if (bytesRead <= 0)
    {
        perror("Failed to read /proc/meminfo");
        return -1.0f;
    }
    buffer[bytesRead] = '\0'; // Null-terminate

    // Parse values (modern Linux format)
    unsigned long memTotal = 0, memFree = 0, memAvailable = 0, cached = 0, buffers = 0;

    // Use more flexible parsing (in case order changes slightly)
    char *pos = buffer;
    while (pos && *pos)
    {
        if (sscanf(pos, "MemTotal: %lu kB", &memTotal) == 1)
        {
            pos = strchr(pos, '\n');
            if (pos)
                pos++;
        }
        else if (sscanf(pos, "MemFree: %lu kB", &memFree) == 1)
        {
            pos = strchr(pos, '\n');
            if (pos)
                pos++;
        }
        else if (sscanf(pos, "MemAvailable: %lu kB", &memAvailable) == 1)
        {
            pos = strchr(pos, '\n');
            if (pos)
                pos++;
        }
        else if (sscanf(pos, "Buffers: %lu kB", &buffers) == 1)
        {
            pos = strchr(pos, '\n');
            if (pos)
                pos++;
        }
        else if (sscanf(pos, "Cached: %lu kB", &cached) == 1)
        {
            pos = strchr(pos, '\n');
            if (pos)
                pos++;
        }
        else
        {
            pos = strchr(pos, '\n');
            if (pos)
                pos++;
        }
    }

    // Validate data
    if (memTotal == 0)
    {
        fprintf(stderr, "Invalid MemTotal in /proc/meminfo\n");
        return -1.0f;
    }

    // Modern calculation (prefer MemAvailable for accurate free memory)
    if (memAvailable > 0)
    {
        return 1.0f - ((float)memAvailable / memTotal);
    }
    // Fallback to traditional calculation
    else
    {
        return 1.0f - ((float)(memFree + buffers + cached) / memTotal);
    }
}

/**
 * @brief
 *
 * @return int
 */
int GetCPUTemp()
{
    int FileHandler;
    char FileBuffer[1024];
    int CPU_temp;
    FileHandler = open("/sys/devices/virtual/thermal/thermal_zone0/temp", O_RDONLY);
    if (FileHandler < 0)
    {
        return -1;
    }
    read(FileHandler, FileBuffer, sizeof(FileBuffer) - 1);
    sscanf(FileBuffer, "%d", &CPU_temp);
    close(FileHandler);
    return CPU_temp / 1000;
}

/**
 * @brief
 *
 * @param signo
 */
void sig_handler(int signo)
{
    done = 1;
    printf("received signo :%d \n", signo);
}

/**
 * @brief
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char **argv)
{
    time_t mytime;
    struct tm *tm;
    uint8_t time_buffer[80];
    uint8_t text_buffer[100];
    struct ifaddrs *ifaddr, *ifa;
    int family, s, n, row, offset;
    char host[NI_MAXHOST];
    offset = 3;
    // Print pid, so that we can send signals from other shells
    printf("My pid is: %d\n", getpid());
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = sig_handler;
    // Intercept SIGHUP and SIGINT
    if (sigaction(SIGINT, &action, NULL) == -1)
    {
        perror("Error: cannot handle SIGINT"); // Should not happen
    }
    if (sigaction(SIGTERM, &action, NULL) == -1)
    {
        perror("Error: cannot handle SIGTERM"); // Should not happen
    }

    /* Init I2C bus  */
    bus = i2c_init((char *)&"/dev/i2c-0", 0x3c); // dev, slavei2caddr
    /* */
    ssd1306Init(SSD1306_SWITCHCAPVCC);

    while (done == 0)
    {
        row = 3;
        _font = (FONT_INFO *)&ubuntuMono_8ptFontInfo;
        /* Display time */
        mytime = time(NULL);
        tm = localtime(&mytime);
        ssd1306ClearScreen(LAYER0 | LAYER1);
        strftime(time_buffer, 80, "    %H:%M %x", tm);
        ssd1306DrawString(0, row * 8 + offset, time_buffer, 1, WHITE, LAYER0);
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
        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
        {
            if (ifa->ifa_addr == NULL)
                continue;

            family = ifa->ifa_addr->sa_family;

            if (family == AF_INET)
            {
                void *addr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
                inet_ntop(AF_INET, addr, host, NI_MAXHOST);
                // Пропускаем loopback (127.0.0.1) и docker-интерфейсы
                if (strncmp(ifa->ifa_name, "lo", 2) != 0 &&
                    strncmp(ifa->ifa_name, "docker", 6) != 0)
                {
                    snprintf(text_buffer, sizeof(text_buffer), "%s", host);
                    ssd1306DrawString(0, row * 8 + offset, text_buffer, 1, WHITE, LAYER0);
                    row++;
                }
            }
        }

        freeifaddrs(ifaddr);

        /*
         * CPU usage
         */
        float c = GetCPULoad();
        snprintf(text_buffer, sizeof(text_buffer), "CPU load: %3.0f %%", (c * 100.0f));
        // printf("%s\n", text_buffer);
        ssd1306DrawString(4, 16, text_buffer, 1, WHITE, LAYER0);
        ssd1306DrawRect(0, 14, 127, 13, INVERSE, LAYER0);
        ssd1306FillRect(2, 16, (int)(123 * c), 9, INVERSE, LAYER0);

        /* Memory usage */
        float m = GetMemUsage();
        snprintf(text_buffer, sizeof(text_buffer), "Mem used: %3.0f%%", m * 100);
        // printf("%s\n", text_buffer);
        ssd1306DrawString(4, 2, text_buffer, 1, WHITE, LAYER0);
        ssd1306DrawRect(0, 0, 127, 13, INVERSE, LAYER0);
        ssd1306FillRect(2, 2, (int)(123 * m), 9, INVERSE, LAYER0);

        /* CPU temperature  */
        int t = GetCPUTemp();
        snprintf(text_buffer, sizeof(text_buffer), "CPU temp: %3d C", t);
        // printf("%s\n", text_buffer);
        ssd1306DrawString(0, row * 8 + offset, text_buffer, 1, WHITE, LAYER0);
        row++;

        /* Refresh screen */
        ssd1306Refresh();
        SSD1306MSDELAY(1000);

        memset(text_buffer, 0, sizeof(text_buffer)); // Fill with zeros
    }

    _font = (FONT_INFO *)&ubuntuMono_24ptFontInfo;
    ssd1306ClearScreen(LAYER0 | LAYER1);
    ssd1306Refresh();
    close(bus);
    printf("All done!\r\n");
    return (EXIT_SUCCESS);
}
