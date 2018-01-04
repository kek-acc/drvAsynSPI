#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <stdint.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <unistd.h>

#include <epicsEvent.h>
#include <epicsExport.h>
#include <epicsMutex.h>
#include <epicsString.h>
#include <epicsThread.h>
#include <epicsTime.h>
#include <epicsTypes.h>
#include <iocsh.h>

#include "asynSPI.h"
#include "drvAsynSPI.h"


static asynSPI spiDummy = {};
epicsShareDef asynSPI *pasynSPI = &spiDummy;


asynStatus drvAsynSPI::flushOctet( asynUser *pasynUser ) {
  return asynSuccess;
}


asynStatus drvAsynSPI::readOctet( asynUser *pasynUser, char *value, size_t maxChars,
                                  size_t *nActual, int *eomReason ) {

  #ifdef STREAM_WORKAROUND
    // the writeHandler method of StreamDevice tries to read 256 bytes on the bus
    // until asynTimeout (or asynError) is returned.
    // But after having set up a valid slave address each read access will
    // produce valid data, so StreamDevice will end up in an endless loop
    // never reaching its break condition.
    //
    // This workaround simply returns asynTimeout on read requests with 256 byte length.
    // Such long messages should not have any use case on SPI busses.
    if( maxChars == 256 ) return asynTimeout;
  #endif

  if( _fd < 0 ) {
    epicsSnprintf( pasynUser->errorMessage, pasynUser->errorMessageSize,
                   "%s: %s disconnected:", portName, _deviceName );
    return asynError;
  }

  if( maxChars <= 0 ) {
    epicsSnprintf( pasynUser->errorMessage, pasynUser->errorMessageSize,
                   "%s: %s maxchars %d Why <=0?", portName, _deviceName, (int)maxChars );
    return asynError;
  }

  int nRead = 0;
  asynStatus status = asynSuccess;
  if( eomReason ) *eomReason = 0;

  int mytimeout = (int)( pasynUser->timeout * 1.e6 );
  if ( mytimeout <= 0  ) {

    nRead = read( _fd, value, maxChars );
    if ( nRead < 0 ) {
      epicsSnprintf( pasynUser->errorMessage, pasynUser->errorMessageSize, 
                     "Error receiving message from device '%s': %d %s", 
                     _deviceName, errno, strerror( errno ) );
      return asynError;
    }

  } else {

    fd_set fdRead;
    struct timeval t;
    
    // calculate timeout values
    t.tv_sec  = mytimeout / 1000000L;
    t.tv_usec = mytimeout % 1000000L;
    
    FD_ZERO( &fdRead );
    FD_SET( _fd, &fdRead );
    
    // wait until timeout or a message is ready to get read
    int err = select( _fd + 1, &fdRead, NULL, NULL, &t );
    
    // the only one file descriptor is ready for read
    if ( err > 0 ) {
      nRead = read( _fd, value, maxChars );
      if( nRead < 0 ) {
        epicsSnprintf( pasynUser->errorMessage, pasynUser->errorMessageSize, 
                       "Error receiving message from device '%s': %d %s", 
                       _deviceName, errno, strerror( errno ) );
        return asynError;
      }
    }
    
    // nothing is ready, timeout occured
    if ( err == 0 ) return asynTimeout;
    if ( err < 0 )  {
      epicsSnprintf( pasynUser->errorMessage, pasynUser->errorMessageSize, 
                     "Error receiving message from device '%s': %d %s", 
                     _deviceName, errno, strerror( errno ) );
      return asynError;
    }
  }

  *nActual = nRead;
  if( eomReason && *nActual >= maxChars ) {
    *eomReason = ASYN_EOM_CNT;
  }

  asynPrint( pasynUser, ASYN_TRACEIO_DRIVER, 
             "%s: read %lu bytes from %s, return %s\n",
             portName, (unsigned long)*nActual, _deviceName,
             pasynManager->strStatus( status ) );
  
  return status; 
}


asynStatus drvAsynSPI::writeOctet( asynUser *pasynUser, char const *value, size_t maxChars,
                                   size_t *nActual ){

  int thisWrite = 0;
  asynStatus status = asynSuccess;

  if( _fd < 0 ) {
    epicsSnprintf( pasynUser->errorMessage, pasynUser->errorMessageSize,
                   "%s: %s disconnected:", portName, _deviceName );
    return asynError;
  }
  if( maxChars == 0 ) {
    *nActual = 0;
    return asynSuccess;
  }

  int nleft = maxChars;

  if( nleft > 0 ) {

    int mytimeout = (int)( pasynUser->timeout * 1.e6 );
    if ( mytimeout <= 0 ) {

      thisWrite = write( _fd, value, nleft );
      if ( thisWrite < 0 ) {
        epicsSnprintf( pasynUser->errorMessage, pasynUser->errorMessageSize, 
                       "%s: %s write error: %s", 
                       portName, _deviceName, strerror( errno ) );
        return asynError;
      }

    } else {

      fd_set fdWrite;
      struct timeval t;
      
      // calculate timeout values
      t.tv_sec  = mytimeout / 1000000L;
      t.tv_usec = mytimeout % 1000000L;
      
      FD_ZERO( &fdWrite );
      FD_SET( _fd, &fdWrite );
      
      // wait until timeout or device os read to write
      int err = select( _fd + 1, NULL, &fdWrite, NULL, &t );
      
      // the only one file descriptor is ready for writing
      if ( err > 0) {
        thisWrite = write( _fd, value, nleft );
        if ( thisWrite < 0 ) {
          epicsSnprintf( pasynUser->errorMessage, pasynUser->errorMessageSize, 
                         "Error receiving message from device '%s': %d %s", 
                         _deviceName, errno, strerror( errno ) );
          return asynError;
        }
        nleft -= thisWrite;
      }
      
      // nothing is ready, timeout occured
      if ( err == 0 ) return asynTimeout;
      if ( err < 0 )  {
        epicsSnprintf( pasynUser->errorMessage, pasynUser->errorMessageSize, 
                       "Error receiving message from device '%s': %d %s", 
                       _deviceName, errno, strerror( errno ) );
        return asynError;
      }
    }

  }

  *nActual = maxChars - nleft;

  asynPrint( pasynUser, ASYN_TRACEIO_DRIVER, 
             "%s: wrote %lu bytes to %s, return %s\n",
             portName, (unsigned long)*nActual, _deviceName,
             pasynManager->strStatus( status ) );
  
  return status; 
}


asynStatus drvAsynSPI::connect( asynUser *pasynUser ) {
  if( _fd >= 0 ) {
    epicsSnprintf( pasynUser->errorMessage,pasynUser->errorMessageSize,
                   "%s: Link to %s already open!", portName, _deviceName );
    return asynError;
  }
  asynPrint( pasynUser, ASYN_TRACEIO_DRIVER,
             "%s: Open connection to %s\n", portName, _deviceName );

  if( ( _fd = open( _deviceName, O_RDWR ) ) < 0 ) {
    epicsSnprintf( pasynUser->errorMessage,pasynUser->errorMessageSize,
                   "%s: Can't open %s: %s", portName, _deviceName, strerror( errno ) );
    return asynError;
  }

  // Seetings SPI mode
  if( ioctl( _fd, SPI_IOC_WR_MODE, &_spi_mode ) < 0 ) {
    epicsSnprintf( pasynUser->errorMessage,pasynUser->errorMessageSize,
                   "%s: Can't set spi_mode of %s: %s", portName, _deviceName, strerror( errno ) );
    return asynError;
  }

  u_int8_t mode;
  if( ioctl( _fd, SPI_IOC_RD_MODE, &mode ) >= 0 ){
    asynPrint( pasynUser, ASYN_TRACEIO_DRIVER, 
               "%s: set %d to spi_mode of %s\n", portName, mode, _deviceName);
  }

  // Seetings SPI speed
  if( ioctl( _fd, SPI_IOC_WR_MAX_SPEED_HZ, &_max_speed ) < 0 ) {
    epicsSnprintf( pasynUser->errorMessage,pasynUser->errorMessageSize,
                   "%s: Can't set max_speed of %s: %s", portName, _deviceName, strerror( errno ) );
    return asynError;
  }

  u_int32_t speed;
  if( ioctl( _fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed ) >= 0 ){
    asynPrint( pasynUser, ASYN_TRACEIO_DRIVER, 
               "%s: set %d to max_speed of %s\n", portName, speed, _deviceName);
  }

  pasynManager->exceptionConnect( pasynUser );
  return asynSuccess;
}


asynStatus drvAsynSPI::disconnect( asynUser *pasynUser ) {
  asynPrint( pasynUser, ASYN_TRACEIO_DRIVER,
             "%s: disconnect %s\n", portName, _deviceName );
  if( 0 <= _fd ) {
    close( _fd );
    _fd = -1;
    pasynManager->exceptionDisconnect(pasynUser);
  } 
  return asynSuccess;
}


drvAsynSPI::drvAsynSPI( const char *portName, const char *ttyName, int spi_mode, u_int32_t max_speed, int autoConnect ) :
  asynPortDriver( portName,
                  0, // maxAddr
                  0, // paramTableSize
                  asynCommonMask | asynOctetMask | asynDrvUserMask, // Interface mask
                  asynCommonMask | asynOctetMask,  // Interrupt mask
                  ASYN_CANBLOCK, // asynFlags
                  autoConnect,  // Autoconnect
                  0,  // Default priority
                  0 ), // Default stack size
  _fd( -1 )
{
  asynStatus status;
  _deviceName = epicsStrDup( ttyName );

  switch(spi_mode){
      case 0:  _spi_mode = SPI_MODE_0; break;
      case 1:  _spi_mode = SPI_MODE_1; break;
      case 2:  _spi_mode = SPI_MODE_2; break;
      case 3:  _spi_mode = SPI_MODE_3; break;
      default: _spi_mode = SPI_MODE_0; break;
  }

  _max_speed = max_speed;

  // Register SPI interface
  spi.interfaceType = asynSpiType;
  spi.pinterface = pasynSPI;
  spi.drvPvt = this;

  status = pasynManager->registerInterface( portName, &spi );
  if( status ) {
    asynPrint( pasynUserSelf, ASYN_TRACE_ERROR,
               "drvAsynSPI::drvAsynSPI, error registering SPI interface\n" );
  }

  if( autoConnect ) {
    pasynManager->exceptionDisconnect( pasynUserSelf );
    status = this->connect( pasynUserSelf );
    if( status ) {
      asynPrint( pasynUserSelf, ASYN_TRACE_ERROR,
                 "drvAsynSPI::drvAsynSPI, error calling connect %s\n",
                 pasynUserSelf->errorMessage );
    }
  }
}

// Configuration routines.  Called directly, or from the iocsh function below 
extern "C" {
  int drvAsynSPIConfigure( const char *portName, const char *ttyName, int spi_mode, int max_speed, int autoConnect = 1 ) {
    if( !portName ) {
      printf( "Port name missing.\n" );
      return -1;
    }
    if( !ttyName ) {
      printf( "TTY name missing.\n" );
      return -1;
    }
    new drvAsynSPI( portName, ttyName, spi_mode, max_speed, autoConnect );
    return( asynSuccess );
  }
  static const iocshArg initSPIArg0 = { "portName", iocshArgString };
  static const iocshArg initSPIArg1 = { "ttyName",  iocshArgString };
  static const iocshArg initSPIArg2 = { "spi_mode",  iocshArgInt };
  static const iocshArg initSPIArg3 = { "max_speed",  iocshArgInt };
  static const iocshArg initSPIArg4 = { "autoConnect",  iocshArgInt };
  static const iocshArg * const initSPIArgs[] = { &initSPIArg0, &initSPIArg1, &initSPIArg2, &initSPIArg3, &initSPIArg4 };
  static const iocshFuncDef initSPIFuncDef = { "drvAsynSPIConfigure", 5, initSPIArgs };
  static void initSPICallFunc( const iocshArgBuf *args ) {
    drvAsynSPIConfigure( args[0].sval, args[1].sval, args[2].ival, args[3].ival, args[4].ival );
  }

  //----------------------------------------------------------------------------
  //! @brief   Register functions to EPICS
  //----------------------------------------------------------------------------
  void drvAsynSPIRegister( void ) {
    static int firstTime = 1;
    if ( firstTime ) {
      iocshRegister( &initSPIFuncDef, initSPICallFunc );
      firstTime = 0;
    }
  }
  
  epicsExportRegistrar( drvAsynSPIRegister );
}
