/*!
 *  \author   Fachbereich 5 - FH Aachen
 *  \date     2024
 *  \version  1.0
 */

#ifndef SPI_H_
#define SPI_H_

#include <stdint.h>

void spi_init();

void spi_cs_enable();
void spi_cs_disable();

uint8_t spi_write_read(uint8_t byte);
uint8_t spi_read();
void spi_write(uint8_t byte);

void spi_writeData(const void *data, uint8_t length);
void spi_writeDataProgMem(const void *data, uint8_t length);

#endif