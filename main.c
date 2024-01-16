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

    KOS_INIT_FLAGS(INIT_DEFAULT | INIT_MALLOCSTATS);


    long bitrateold, bitratenew;

    extern int zlib_getlength(char*);




// This will be two polygons, each being half a gradient bar. Note that
// it's ok to mix vertex macro types.
void drawbar(float y, float z, float r, float g, float b) {
    // Top poly (black to white)
    plx_vert_fnp(PLX_VERT, 0.0f, y, z, 1.0f, r, g, b);
    plx_vert_inp(PLX_VERT, 0.0f, y - 20.0f, z, 0xff000000);
    plx_vert_fnp(PLX_VERT, 640.0f, y, z, 1.0f, r, g, b);
    plx_vert_inp(PLX_VERT_EOS, 640.0f, y - 20, z, 0xff000000);

    // Bottom poly (white to black)
    plx_vert_inp(PLX_VERT, 0.0f, y + 20.0f, z, 0xff000000);
    plx_vert_fnp(PLX_VERT, 0.0f, y, z, 1.0f, r, g, b);
    plx_vert_inp(PLX_VERT, 640.0f, y + 20.0f, z, 0xff000000);
    plx_vert_fnp(PLX_VERT_EOS, 640.0f, y, z, 1.0f, r, g, b);
}




int main(int argc, char **argv) {

    int o;

    /* Initializing the KOS sound system */
    snd_stream_init();
    mp3_init();
    mp3_volume(120);
    mp3_start("/rd/test1.mp3", LOOP);

    int done, i;
    float theta, dt;
    float colors[3 * 10] = {
        0.0f, 0.5f, 1.0f,
        0.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.0f,
        0.5f, 0.0f, 0.5f,
        0.5f, 0.5f, 0.5f,
        0.73f, 0.8f, 0.25f,
        0.25f, 0.8f, 0.73f,
        1.0f, 1.0f, 0.0f
    };

    // Init PVR
    pvr_init_defaults();

    // Setup the context
    plx_cxt_init();
    plx_cxt_texture(NULL);
    plx_cxt_culling(PLX_CULL_NONE);

    // Setup the frame
    pvr_wait_ready();
    pvr_scene_begin();
    pvr_list_begin(PVR_LIST_OP_POLY);

    // Submit the context
    plx_cxt_send(PVR_LIST_OP_POLY);

    // Until the user hits start...
    dt = 2 * F_PI / 160.0f;

    for(done = 0, theta = 0.0f; !done;) {
        // Check for start
        MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)

                    if(st->buttons & CONT_START)
                        done = 1;

                MAPLE_FOREACH_END()

        // Setup the frame
        pvr_wait_ready();
        pvr_scene_begin();
        pvr_list_begin(PVR_LIST_OP_POLY);

        // Submit the context
        plx_cxt_send(PVR_LIST_OP_POLY);

        // Draw a sinus bar at our current position and several positions
        // back. Each bar will get its own Z value (descending).
        for(i = 0; i < 10; i++) {
            drawbar(240.0f + fsin(theta - dt * i * 6) * 120.0f, 100.0f - i,
                    colors[i * 3 + 0], colors[i * 3 + 1], colors[i * 3 + 2]);
        }

        pvr_scene_finish();

        // Move our counters
        theta += dt;

        /* Set our starting offset to one letter height away from the
      top of the screen and two widths from the left */
        o = (640 * BFONT_HEIGHT) + (BFONT_THIN_WIDTH * 2);

        /* Test with ISO8859-1 encoding */
        bfont_set_encoding(BFONT_CODE_ISO8859_1);
        bfont_draw_str(vram_s + o, 640, 1, "Keep Going!");
        /* After each string, we'll increment the offset down by one row */
        o += 640 * BFONT_HEIGHT;
        bfont_draw_str(vram_s + o, 640, 1, "You can do it!");
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

        while(theta >= 2 * F_PI)
            theta -= 2 * F_PI;
    }
    return 0;
}




/* uncomment for red+blue repeating squares
    * for(y = 0; y < 480; y++)
        for(x = 0; x < 640; x++) {
            int c = (x ^ y) & 255;
            vram_s[y * 640 + x] = ((c >> 3) << 12)
                                  | ((c >> 2) << 5)
                                  | ((c >> 3) << 0);
        }







    return 0;
} */