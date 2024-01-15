#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>


#include <kos.h>
#include <kos/init.h>
#include <dc/video.h>
#include <dc/biosfont.h>
#include <png/png.h>
#include <sys/cdefs.h>
#include <arch/types.h>
#include <dc/maple.h>
#include <dc/maple/controller.h>
#include <dc/maple/vmu.h>
#include <arch/arch.h>
#include <plx/texture.h>
#include <plx/context.h>
#include <plx/prim.h>
#include <plx/font.h>
#include <zlib/zlib.h>
#include <unistd.h>
#include <mp3/sndserver.h>
#include <oggvorbis/sndoggvorbis.h>


#define LOOP 10
/* These macros tell KOS how to initialize itself. All of this initialization
   happens before main() gets called, and the shutdown happens afterwards. So
   you need to set any flags you want here. Here are some possibilities:

   INIT_NONE        -- don't do any auto init
   INIT_IRQ         -- Enable IRQs
   INIT_NET         -- Enable networking (including sockets)
   INIT_MALLOCSTATS -- Enable a call to malloc_stats() right before shutdown

   You can OR any or all of those together. If you want to start out with
   the current KOS defaults, use INIT_DEFAULT (or leave it out entirely). */
long bitrateold, bitratenew;
    KOS_INIT_FLAGS(INIT_DEFAULT);


    extern int zlib_getlength(char*);

    /* texture stuff */
    pvr_ptr_t font_tex;
    pvr_ptr_t back_tex;
    char *data;


/* init background */
void back_init(void) {
    back_tex = pvr_mem_malloc(512 * 512 * 2);
    png_to_texture("/rd/couldbebluer.png", back_tex, PNG_NO_ALPHA);
}



/* draw background */
void draw_back(void) {
    pvr_poly_cxt_t cxt;
    pvr_poly_hdr_t hdr;
    pvr_vertex_t vert;

    pvr_poly_cxt_txr(&cxt, PVR_LIST_OP_POLY, PVR_TXRFMT_RGB565, 512, 512, back_tex, PVR_FILTER_BILINEAR);
    pvr_poly_compile(&hdr, &cxt);
    pvr_prim(&hdr, sizeof(hdr));

    vert.argb = PVR_PACK_COLOR(1.0f, 1.0f, 1.0f, 1.0f);
    vert.oargb = 0;
    vert.flags = PVR_CMD_VERTEX;

    vert.x = 0.0f;
    vert.y = 0.0f;
    vert.z = 1.0f;
    vert.u = 0.0f;
    vert.v = 0.0f;
    pvr_prim(&vert, sizeof(vert));

    vert.x = 640.0f;
    vert.y = 0.0f;
    vert.z = 1.0f;
    vert.u = 1.0f;
    vert.v = 0.0f;
    pvr_prim(&vert, sizeof(vert));

    vert.x = 1.0f;
    vert.y = 480.0f;
    vert.z = 1.0f;
    vert.u = 0.0f;
    vert.v = 1.0f;
    pvr_prim(&vert, sizeof(vert));

    vert.x = 640.0f;
    vert.y = 480.0f;
    vert.z = 1.0f;
    vert.u = 1.0f;
    vert.v = 1.0f;
    vert.flags = PVR_CMD_VERTEX_EOL;
    pvr_prim(&vert, sizeof(vert));
}


int main(int argc, char **argv) {

    /* init kos ? */
    pvr_init_defaults();

    /* init background */
    back_init();

    /* Initializing the KOS sound system */
    snd_stream_init();
    mp3_init();
    mp3_volume(120);
    mp3_start("/rd/test1.mp3", LOOP);


    auto i = 2;
    int x, y, o;

   /* Uncomment to get repeating squares, it's commented out now to test other code
    * for(y = 0; y < 480; y++)
        for(x = 0; x < 640; x++) {
            int c = (x ^ y) & 255;
            vram_s[y * 640 + x] = ((c >> 3) << 12)
                                  | ((c >> 2) << 5)
                                  | ((c >> 3) << 0);
        }

    /* Set our starting offset to one letter height away from the
       top of the screen and two widths from the left */
    o = (640 * BFONT_HEIGHT) + (BFONT_THIN_WIDTH * 2);

    /* Test with ISO8859-1 encoding */
    bfont_set_encoding(BFONT_CODE_ISO8859_1);
    bfont_draw_str(vram_s + o, 640, 1, "KEEP GOING");
    /* After each string, we'll increment the offset down by one row */
    o += 640 * BFONT_HEIGHT;
    bfont_draw_str(vram_s + o, 640, 1, "YOU CAN DO IT!");
    o += 640 * BFONT_HEIGHT;



    /* Drawing the special symbols is a bit convoluted. First we'll draw some
       standard text as above. */
    bfont_set_encoding(BFONT_CODE_ISO8859_1);
    bfont_draw_str(vram_s + o, 640, 1, "To exit, press ");

    /* Then we set the mode to raw to draw the special character. */
    bfont_set_encoding(BFONT_CODE_RAW);
    /* Adjust the writing to start after "To exit, press " and draw the one char */
    bfont_draw_wide(vram_s + o + (BFONT_THIN_WIDTH * 15), 640, 1, BFONT_STARTBUTTON);

    /* If Start is pressed, exit the app */
    cont_btn_callback(0, CONT_START, (cont_btn_callback_t)arch_exit);

    /* Just trap here waiting for the button press */
    for(;;) { usleep(50); }

    return 0;
}