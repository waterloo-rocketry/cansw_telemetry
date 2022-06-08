/* 
 * File:   main.c
 * Author: blu
 *
 * Created on May 21, 2022, 11:45 AM
 */

#include <xc.h>

/*
 * 
 */
int main(int argc, char** argv) {
    
    ANSAbits.ANSELA0 = 0;
    TRISAbits.TRISA12 = 0;
    
    const long max = 50000;
    
    while (1) {
        LATAbits.LATA12 = 0;
        for (long i = 0; i < max; i++);
        LATAbits.LATA12 = 1;
        for (long i = 0; i < max; i++);
    }
}

