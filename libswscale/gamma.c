#include "swscale_internal.h"

typedef struct GammaContext
{
    uint16_t *table;
} GammaContext;

// gamma_convert expects 16 bit rgb format
// it writes directly in src slice thus it must be modifiable (done through cascade context)
static int gamma_convert(SwsContext *c, SwsFilterDescriptor *desc, int sliceY, int sliceH)
{
    GammaContext *instance = desc->instance;
    uint16_t *table = instance->table;
    int srcW = desc->src->width;

    int i;
    for (i = 0; i < sliceH; ++i) {
        uint8_t ** src = desc->src->plane[0].line;
        int src_pos = sliceY+i - desc->src->plane[0].sliceY;

        uint16_t *src1 = (uint16_t*)*(src+src_pos);
        int j;
        for (j = 0; j < srcW; ++j) {
            uint16_t r = AV_RL16(src1 + j*4 + 0);
            uint16_t g = AV_RL16(src1 + j*4 + 1);
            uint16_t b = AV_RL16(src1 + j*4 + 2);

            AV_WL16(src1 + j*4 + 0, table[r]);
            AV_WL16(src1 + j*4 + 1, table[g]);
            AV_WL16(src1 + j*4 + 2, table[b]);
        }

    }
    return sliceH;
}


int ff_init_gamma_convert(SwsFilterDescriptor *desc, SwsSlice * src, uint16_t *table)
{
    GammaContext *li = av_malloc(sizeof(GammaContext));
    if (!li)
        return AVERROR(ENOMEM);
    li->table = table;

    desc->instance = li;
    desc->src = src;
    desc->dst = NULL;
    desc->process = &gamma_convert;

    return 0;
}
