#ifndef rand_place_h
#define rand_place_h

#include <stdint.h>

#define COORD 2
#define RECT_POS 4

struct rect {
    int idx;
    int p[RECT_POS * COORD]; // ldx ldy, ltx lty, rdx rdy, rtx rty
    struct rect *next;
};

struct map {
    int p[RECT_POS * COORD];
    struct rect *holes;
    int hole_num;
    int w; int h;

    int idx;
    int place[COORD];   // rand_place result x,y
};

// left down x, left down y, width, height
struct map *
create_map(int x, int y, int w, int h);

int
dig_hole(struct map *m, int x, int y, int w, int h);

int
del_hole(struct map *m, int idx);

int
rand_place(struct map *m, int w, int h);

void
destory(struct map *m);

#endif
