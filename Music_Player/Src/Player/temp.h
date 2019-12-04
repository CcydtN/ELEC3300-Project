#ifndef SRC_PLAYER_TEMP_H_
#define SRC_PLAYER_TEMP_H_

#include "config_types.h"
#include "ivorbiscodec.h"
#include "ivorbisfile.h"

#include "vorbisfile.c"
#include "info.c"
#include "framing.c"
#include "bitwise.c"
#include "mapping0.c"
#include "floor0.c"
#include "floor1.c"
#include "codebook.c"
#include "res012.c"
#include "dsp.c"
#include "floor_lookup.c"
#include "mdct.c"

void oggPlayer(char *fname);

void pullData(void);

void rearrData(void);

void TIM_reINIT(uint16_t psc, uint16_t arr);

void Start_DMA(void);

void closefile(void);

#endif /* SRC_PLAYER_TEMP_H_ */
