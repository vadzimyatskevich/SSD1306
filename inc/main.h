/**
 * @file main.h
 * @author Vadzim Yatskevich (vadzimyatskevich@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2025-04-12
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef MAIN_H
#define MAIN_H

typedef struct {
    unsigned long user;
    unsigned long nice;
    unsigned long system;
    unsigned long idle;
    unsigned long iowait;
    unsigned long irq;
    unsigned long softirq;
} CPUStats;

#endif /* MAIN_H */