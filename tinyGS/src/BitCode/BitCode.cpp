/*
  BitCode.cpp - BitCode class

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

//////////////////////////////////////////////////////////////////////

 This is a brief explanation of the principle of operation perfomed
  to extract an AX.25 frame from the packet received in FSK.
  
  For this to work there are several assumptions:
  
  1. The RF Chip receive in NRZ. 
  
  2. Satellite transmit in NRZS, which means that, every time the
     satellite is going to send a ZERO, the state of the channel will
     change (from f1 -> f2 or from f2 -> f1) and when a ONE is going to
     be transmitted no change in the channel is performed, that is,
     frequency remains in f1 or in f2.  
  
  3. Satellite is in f1 before transmitting the first ZERO, and then
     change to f2 for that first ZERO, which in NRZ means to receive
     a ONE.

                             fd
                           <----->
                 / \      |      / \
                  |               |
                  |               |
     --------------------------------------> f
                          fc
			  
		  f1    --->     f2 (for sending first ZERO)
		  
  4. The RF Chip will be configured with a specific Synch Word to trigger 
     the reception. Given that the satellite codify the information 
     transmitted in NRZS and the Chip receives in NRZ, the Synch Word 
     has to be previously translated from NRZS to NRZ. In order to 
     create the Synch Word first we take the flag 0x7E and the first two
     bytes of data as transmitted by the satellite, and convert it to NRZ.

     For a satellite transmitting 0x7E, 0x49 and 0X39 as first three bytes,
     the synchword is calculated, assuming that for the first ZERO on the 
     most significant bit of first byte 0x7E, a transition from ZERO to ONE
     is needed, thus, the first bit on the left, in the first byte of the 
     synchword is ONE. From this point on, any time a data ZERO is found, 
     the bit in the synchword changes from 0->1 or from 1->0, keeping the 
     previous state everytime a ONE is found in the data.
     
         7         E         4         9         3         9    
      -------   -------   -------   -------   -------   -------   
      0 1 1 1   1 1 1 0   0 1 0 0   1 0 0 1   0 0 1 1   1 0 0 1  <-- DATA
     
   0->1 1 1 1   1 1 1 0   1 1 0 1   1 0 1 1   0 1 1 1   1 0 1 1  <-- SW
      -------   -------   -------   -------   -------   -------
         F         E         D         B         7         B
      -----------------   -----------------   -----------------   
	     254                 219                  123

     Synchword = [254, 219, 123]

  5. The algorithm assumes that first byte of the synchword, which will
     be added to the packet together with the rest of the synchword during 
     the process, is always 0xFE (254), for this reason, as we can seen 
     below, the first byte of the packet (0x7E) is skipped.
     
     When data packet is received, the process to extract the AX.25 frame 
     is as follows:
   
                            NRZ
                                       \ \ ____
  ))))                    bit(t)  -----| |     \
                                       | |      o---- NRZI(S)
      |                   bit(t+1)-----| | ____/      
      |                                / /
      |                          
     / \                                       ADD SYNCHWORD TO
    /   \       RF CHIP         NRZI(S)       THE PACKET RECEIVED    
   /     \     ---------        ------        -------------------              
  /       \___| FSK NRZ | ---> | NXOR | ---> | FRAME HDLC(AX.25) | 
               ---------        ------        -------------------  
                                                       |
                                                      \|/
                                            -----------------------
                                           | SKIP FIRST 0X7E FLAG  |
                                            -----------------------
                                                       |
                                                      \|/
                                             ---------------------
                                            | REMOVE BIT STUFFING |
                                             ---------------------
                                                       |
                                                      \|/
                                            -----------------------
                                           | DETECT END FLAG 0X7E  |
                                            -----------------------
                                                       |
                                                      \|/
 -------------------------       ----------------------------------- 
| CALCULATE CRC & COMPARE | <-- | SPLIT FRAME INVERTED AX.25  & CRC |
 -------------------------       -----------------------------------   
            |
	   \|/
 ------------------------------------------         -------------  
| INVERT BITS FOR EACH BYTE OF AX.25 FRAME | ----> | FRAME AX.25 | 
 ------------------------------------------	    -------------      
 
*/
//////////////////////////////////////////////////////////////////////*/
#include <stdio.h>
#include "BitCode.h"
#include <stdint.h> //uint8_t

//////////////////////////////////////////////////////////////////////
//         BYTE
//    8 7 6 5 4 3 2 1   <-- POSICION
//////////////////////////////////////////////////////////////////////
uint8_t BitCode::read_bit_from_byte(uint8_t byte, int posicion_bit){
  uint8_t test=1;
  uint8_t resultado=0;
  //printf("Byte %X \n",byte);
  //printf("Posicion de bit %i \n",posicion_bit);
  test<<=posicion_bit-1;
  //printf("Test %X \n",test);
  resultado=byte&test;
  //printf("Resultado Operacion & %X \n",resultado);
  if (resultado==0){return 0;}else{return 1;}
}
//////////////////////////////////////////////////////////////////////
uint8_t BitCode::char2hexValue(uint8_t caracter){  
  switch (caracter)
  {
    case 48: return 0;break;    //0
    case 49: return 1;break;    //1
    case 50: return 2;break;    //2
    case 51: return 3;break;    //3
    case 52: return 4;break;    //4
    case 53: return 5;break;    //5
    case 54: return 6;break;    //6
    case 55: return 7;break;    //7
    case 56: return 8;break;    //8
    case 57: return 9;break;    //9
    case 65: return 10;break;   //A
    case 66: return 11;break;   //B
    case 67: return 12;break;   //C
    case 68: return 13;break;   //D
    case 69: return 14;break;   //E
    case 70: return 15;break;   //F
  }
  return 99;
}

//////////////////////////////////////////////////////////////////////
uint8_t BitCode::compone_byte_en_hexadecimal(uint8_t msc, uint8_t lsc){
  uint8_t msd=0;//most significative digit
  uint8_t lsd=0;//lowest significative digit
  uint8_t aux=0;
  //printf("Caracter mas significativo: %d\n",msc);
  msd=char2hexValue(msc);
  //printf("Digito mas significativo del futuro Hexadecimal: %d\n",msd);
  //printf("Caracter menos significativo: %d\n",lsc);
  lsd=char2hexValue(lsc);
  //printf("Digito menos significativo del futuro Hexadecimal: %d\n",lsd);
  aux=msd;
  //printf("Valor auxiliar antes de desplazar: %d\n",aux);
  aux=aux<<4;
  //printf("Valor auxiliar después de desplazar: %d\n",aux);
  aux=aux|lsd;
  return aux;
}

//////////////////////////////////////////////////////////////////////
int BitCode::nrz2nrzi (char *cadena_nrz, size_t size, char *salida, uint8_t *salidabin){
uint8_t primer_bit=0;
uint8_t segundo_bit=0;
uint8_t byte_recibido=0;
uint8_t byte_procesado=0;
uint8_t bit=0;
size_t bini=0;

    //Cada carácter es intrepretado como un código ASCII. Sin embargo se desea que cada carácter sea
    //interpretado como un carácter hexadecimal, y agrupados de dos a dos formarían un byte.
    //char primer_bit = 0;
    //uint8_t a[3] = {0xFE,0XCB,0X2B};
    for (int p=0; p<size;p+=2){ 
      byte_recibido=compone_byte_en_hexadecimal(cadena_nrz[p],cadena_nrz[p+1]);
      //printf("Valor a transformar: %02X -->",byte_recibido);
      byte_procesado=0;
      for (int i=8; i>0;i--){
        segundo_bit=read_bit_from_byte(byte_recibido,i);
        //printf("Valor del primer bit %i Valor del segundo bit %i \n",primer_bit,segundo_bit);
        //Si hay cambio de bit entonces es un cero y no hacemos nada ya que el byte_receptor
        //se inicializa al cero. Al mover el índice "i", el valor al que correspondería asignar
        //esa posición i queda pues a cero.
        if (primer_bit == segundo_bit){  
  	    bit=(0x01)<<(i-1);         
  	    byte_procesado = byte_procesado | bit;//
  	    }
	  primer_bit=segundo_bit;
	 }
      sprintf(salida + p , "%02X" , byte_procesado);
      salidabin[bini]=byte_procesado;
      bini++;
    } 
  return 0;
}

//////////////////////////////////////////////////////////////////////
void BitCode::vuelca_byte_buffer(uint8_t byte){
  //printf("%02X ",byte);
}
//////////////////////////////////////////////////////////////////////
//Escribe en byte buffer, en el bit posición k, el valor dado por dato.
//////////////////////////////////////////////////////////////////////
void BitCode::write_bit_on_byte(unsigned char *byte, int k, int dato){
  unsigned char byte_aux = 1;
  //Al rotar se introducen ceros por la derecha.
  byte_aux = byte_aux << (k-1); 
  //Para setear a cero, hacemos una mascara complementado byte_aux para convetir los ceros en unos,
  //y el uno del bit a setear en cero. Al hacer el AND bit a bit, dejaremos lo demás bits como estan
  //y el bit con la posición del bit a cero, quedará a cero.
  if (dato==0){*byte = *byte & ~byte_aux;} 
  if (dato==1){*byte = *byte | byte_aux;}
}
//////////////////////////////////////////////////////////////////////
int BitCode::remove_bit_stuffing (char *cadena_nrzi, size_t sizeCadena, char *salida, size_t *sizeSalida,uint8_t *salidabin, size_t *bini){
  int unos_seguidos=0;
  int k=0;
  uint8_t byte_recibido=0;
  uint8_t byte_procesado=0;
  int j=0;
  int bit1=0;
  int bit2=0;
  int bit3=0;
  bool error_de_trama=true;
  bool flag_encontrado=false;
  bool saltar_un_bit=false;
  //int bini=0;
  *sizeSalida=0;
  //printf("%c",cadena_nrzi[0]);  
  //printf("%c ",cadena_nrzi[1]);  
  k=8;
  //Start in i=0 if initial AX.25 flag was removed if not begin in i=2
  byte_recibido=compone_byte_en_hexadecimal(cadena_nrzi[2],cadena_nrzi[3]);
  bit2=read_bit_from_byte(byte_recibido,8);
  bit3=read_bit_from_byte(byte_recibido,7);
  j=6;
  for (int i=2;i<sizeCadena;i+=2){
    //printf("Caracter bajo analisis: %i\n",i);
    byte_recibido=compone_byte_en_hexadecimal(cadena_nrzi[i],cadena_nrzi[i+1]);
    //printf("%02X %d\n",byte_recibido,byte_recibido);
    //Recorremos el byte recibido, bit a bit
    while (j>0){
      bit1=bit2;
      bit2=bit3;
      //printf("Bit a procesar: %i ",j);
      bit3=read_bit_from_byte(byte_recibido,j);
      //printf("%i",estado);
      if (!saltar_un_bit && !flag_encontrado){
	      if (bit1==0){
        //Realmente no haría falta escribir un cero ya que el byte_procesado se inicializa a cero
        //así que es lo mismo que mover el índice k.
        write_bit_on_byte(&byte_procesado,k,0); //Escribe en en byte buffer, en el bit posición k, un cero.
        k--; //Movemos el índice un bit a la derecha
        unos_seguidos=0;
        //Comprobar si hemos llenado el byte buffer
        if (k==0){
              //vuelca_byte_buffer(byte_procesado);
              sprintf(salida + *sizeSalida , "%02X" , byte_procesado);
              salidabin[*bini]=byte_procesado;
              (*bini)++;
              *sizeSalida+=2;
              k=8;
              byte_procesado=0;
              } 
            }else{
                unos_seguidos++;
                write_bit_on_byte(&byte_procesado,k,1);//Escribe en byte buffer, en el bit posición k, un uno.
                k--;
                if (k==0){
                    //vuelca_byte_buffer(byte_procesado);
                    sprintf(salida + *sizeSalida , "%02X" , byte_procesado);
                    *sizeSalida+=2;
                    salidabin[*bini]=byte_procesado;
                    (*bini)++;
                    k=8;
                    byte_procesado=0;
            } 
            if (unos_seguidos==5){
            if (bit2==1){//Si el sexto bit tambien es un 1 entonces ya habremos acabado (esto no se debería dar más que en el flag)
              j=0;i=sizeCadena;//Dado que el siguiente bit al bit quinto no es un cero damos por terminada la trama
              if (bit3==0){
                flag_encontrado=true;
                //Si se llega al final de la trama AX.25, el numero de bytes ha de ser entero, con lo que
                //el ultimo octeto lo ocuparia el flag de fin de trama y la cuenta de "k" en el byte de 
                //almacenamiento deberia ser de "2" ya que se habrian almacenado los primeros 5 unos del flag
                //y "k" estaria apuntando al siguiente bit a almacenar.
                //          bit 1
                //          |
                //8 7 6 5 4 3 2 1
                //0 1 1 1 1 1 1 0
                //printf("k=%i\n",k);
                if (k==2) {error_de_trama=false;
                }else{
                error_de_trama=true;
                }
                //printf("Encontrado fin de trama HDLC\n");
              }else{
                error_de_trama=true;
                //printf("Error de trama\n");
              }
            }else{  
            unos_seguidos=0;//Hemos encontrado un bit de stuffing
            saltar_un_bit=true;
            }
          }          
	      }
      }else{
           saltar_un_bit=false;
           }
      j--;//Apuntamos al siguiente bit del byte recibido
    }
    j=8;
  }
  
  if (flag_encontrado && !error_de_trama){
    return 0;}
  else{
    return 1;
  }
}    

void BitCode::invierte_bits_de_un_byte(uint8_t br, uint8_t *bs){
  *bs=0;
  uint8_t aux=0;
  aux=read_bit_from_byte(br,1);*bs=(*bs)|aux;*bs=(*bs)<<1;
  aux=read_bit_from_byte(br,2);*bs=(*bs)|aux;*bs=(*bs)<<1;
  aux=read_bit_from_byte(br,3);*bs=(*bs)|aux;*bs=(*bs)<<1;
  aux=read_bit_from_byte(br,4);*bs=(*bs)|aux;*bs=(*bs)<<1;
  aux=read_bit_from_byte(br,5);*bs=(*bs)|aux;*bs=(*bs)<<1;
  aux=read_bit_from_byte(br,6);*bs=(*bs)|aux;*bs=(*bs)<<1;
  aux=read_bit_from_byte(br,7);*bs=(*bs)|aux;*bs=(*bs)<<1;
  aux=read_bit_from_byte(br,8);*bs=(*bs)|aux;
}

void BitCode::invierte_bytes_de_un_array(char *entrada,size_t size,char *salida, uint8_t *salidabin, size_t *bini){
  uint8_t byte_recibido=0;
  uint8_t byte_procesado=0;
  *bini=0;
  //size_t bini=0;
  //printf("Procedimiento invierte_bytes_de_un_array\n");
  for (int i=0;i<size;i+=2){
    byte_recibido=compone_byte_en_hexadecimal(entrada[i],entrada[i+1]);
    invierte_bits_de_un_byte(byte_recibido,&byte_procesado);
    sprintf(salida + i , "%02X" , byte_procesado);
    //printf("%02X",byte_procesado);
    salidabin[*bini]=byte_procesado;
    //*bini=*bini+1;
    (*bini)++;
  }
  //printf("\n");
}

void BitCode::nrz2ax25(char *entrada, size_t buffSize, char *ax25, uint8_t *ax25bin,size_t *sizeAx25bin){
         /////////////////
        char *ax25hdlc;
        char *ax25inv;
        uint8_t *ax25hdlcbin;
        uint8_t *ax25invbin;
        size_t sizeAx25inv=0;
        size_t sizeAx25invbin=0;
        int bitstuff=0;
       /////////////////
        ax25hdlcbin = new uint8_t[buffSize];
        ax25hdlc=new char[buffSize];
        ax25inv=new char[buffSize];
        ax25hdlcbin=new uint8_t[buffSize];
        ax25invbin=new uint8_t[buffSize];
       /////////////////
        BitCode::nrz2nrzi(entrada,buffSize,ax25hdlc,ax25hdlcbin);
        bitstuff=BitCode::remove_bit_stuffing(ax25hdlc,buffSize,ax25inv,&sizeAx25inv,ax25invbin,&sizeAx25invbin);
        if (bitstuff==0){
          BitCode::invierte_bytes_de_un_array(ax25inv,sizeAx25inv,ax25,ax25bin,sizeAx25bin);		  
                    	
          int newsize=*sizeAx25bin-2;
	  /////////////////////////////////////////////////////////////////////////////
	  //https://github.com/jgromes/RadioLib/blob/master/src/protocols/AX25/AX25.cpp
	  /////////////////////////////////////////////////////////////////////////////
          RadioLibCRCInstance.size = 16;
          RadioLibCRCInstance.poly = RADIOLIB_CRC_CCITT_POLY;
          RadioLibCRCInstance.init = RADIOLIB_CRC_CCITT_INIT;
          RadioLibCRCInstance.out = RADIOLIB_CRC_CCITT_OUT;
          RadioLibCRCInstance.refIn = false;
          RadioLibCRCInstance.refOut = false;
          uint16_t fcs=RadioLibCRCInstance.checksum(ax25invbin,newsize);
	  /////////////////////////////////////////////////////////////////////////////
          uint16_t crcfield=ax25invbin[*sizeAx25bin-2]*256+ax25invbin[*sizeAx25bin-1];
  
          if (fcs!=crcfield){
              sprintf(ax25,"CRC error!");
              *sizeAx25bin=10;
              for (int i=0;i<*sizeAx25bin;i++){
                ax25bin[i]=(char)ax25[i];
              }
            }
        }else{
            sprintf(ax25,"Frame error!");
            *sizeAx25bin=12;
            for (int i=0;i<*sizeAx25bin;i++){
              ax25bin[i]=(char)ax25[i];
            }
        }

}
