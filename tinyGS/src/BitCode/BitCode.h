/*
  BitCode.h - BitCode class

  Copyright (C) 2022 -2023 @estbhan

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef BITCODE_H
#define BITCODE_H
#include <stdint.h> //uint8_t
#include <RadioLib.h>

class BitCode{
    public:
//////////////////////////////////////////////////////////////////////
//         BYTE
//    8 7 6 5 4 3 2 1   <-- POSICION
//////////////////////////////////////////////////////////////////////
static uint8_t read_bit_from_byte(uint8_t byte, int posicion_bit);
//////////////////////////////////////////////////////////////////////
static uint8_t char2hexValue(uint8_t caracter);
//////////////////////////////////////////////////////////////////////
static uint8_t compone_byte_en_hexadecimal(uint8_t msc, uint8_t lsc);
//////////////////////////////////////////////////////////////////////
static size_t stringSize(char *cadena);
//////////////////////////////////////////////////////////////////////
static int nrz2nrzi (char *cadena_nrz, size_t size, char *salida, uint8_t *salidabin);
//////////////////////////////////////////////////////////////////////
static void vuelca_byte_buffer(uint8_t byte);
//////////////////////////////////////////////////////////////////////
static void write_bit_on_byte(uint8_t *byte, int k, int dato);
//////////////////////////////////////////////////////////////////////
static int remove_bit_stuffing (char *cadena_nrzi, size_t sizeCadena, char *salida, size_t *sizeSalida,uint8_t *salidabin, size_t *bini);
//////////////////////////////////////////////////////////////////////
static void invierte_bits_de_un_byte(uint8_t br, uint8_t *bs);
//////////////////////////////////////////////////////////////////////
static void invierte_bytes_de_un_array(char *entrada,size_t size,char *salida, uint8_t *salidabin, size_t *bini);
//////////////////////////////////////////////////////////////////////
static void nrz2ax25(char *entrada, size_t buffSize, char *salida, uint8_t *salidabin,size_t *sizeAx25bin);
};
#endif
