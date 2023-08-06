/*
  Satellites.cpp - Satellites class
  
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

#include "Satellites.h"

/*
Output:
0: Raw
1: AX25
*/
int Satellites::coding(int noradid){
    int aux=0;
  switch (noradid)
  {
  case 46276://UPMSAT-2
    aux=1;
    /* code */
    break;
  
  case 51658://INS-2TD
    aux=1;
    /* code */
    break;

  case 43798://ASTROCAST 0.1
    aux=1;
    /* code */
    break;

  default:
    aux=0;
    break;
  }
  return aux;
}
