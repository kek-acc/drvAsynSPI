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

#ifndef __DRV_ASYN_SPI_H__
#define __DRV_ASYN_SPI_H__


#include <asynDriver.h>
#include <asynPortDriver.h>


class drvAsynSPI : public asynPortDriver {
 public:
  drvAsynSPI( const char *portName, const char *ttyName, int spi_mode, u_int32_t max_speed, int autoConnect );

  // These are the methods that we override from asynPortDriver
  virtual asynStatus flushOctet( asynUser *pasynUser );
  virtual asynStatus readOctet( asynUser *pasynUser, char *value, size_t maxChars,
                                size_t *nActual, int *eomReason );
  virtual asynStatus writeOctet( asynUser *pasynUser, char const *value, size_t maxChars,
                                 size_t *nActual );

  virtual asynStatus connect( asynUser *pasynUser );
  virtual asynStatus disconnect( asynUser *pasynUser );

 private:

  int   _fd;
  char* _deviceName;
  u_int8_t _spi_mode;
  u_int32_t _max_speed;
  asynInterface spi;
    
};


#endif
