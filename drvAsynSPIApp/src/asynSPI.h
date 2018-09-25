//******************************************************************************
// Copyright 2018 Shinya Sasaki <shinya.sasaki@kek.jp>
//
// This file is part of drvAsynSPI
//
// drvAsynSPI is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// drvAsynSPI is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// drvAsynSPI is derived from drvAsynI2C, Copyright 2015 Florian Feldbauer
// drvAsynI2C is distributed under the terms of the GNU GPL
//
// Original files of drvAsynI2C were obtained from following URL
// https://github.com/ffeldbauer/drvAsynI2C
//
//******************************************************************************

#ifndef __ASYN_SPI_H__
#define __ASYN_SPI_H__

#include <shareLib.h>

#define asynSpiType "asynSPI"

typedef struct asynSPI asynSPI;
struct asynSPI {
  // no methods needed for this interface.
  // it's just used to change the behaviour
  // of streamDevice when accessing the
  // asynOctet interface...
};
epicsShareExtern asynSPI *pasynSPI;

#endif
