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
