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
