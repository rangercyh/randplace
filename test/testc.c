#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../rand_place.h"

void main() {
    int w = 9; int h = 9;
    struct map *m = create_map(0, 0, w, h);
    printf("dig hole = %d\n", dig_hole(m, 3, 0, 3, 3));
    printf("dig hole = %d\n", dig_hole(m, 0, 3, 3, 3));
    printf("dig hole = %d\n", dig_hole(m, 3, 6, 3, 3));
    printf("dig hole = %d\n", dig_hole(m, 6, 3, 3, 3));

    for (int i = 1; i <= 6; i++) {
        if (rand_place(m, 3, 3)) {
            printf("%d %d\n", m->place[0], m->place[1]);
            printf("dig hole = %d\n", dig_hole(m, m->place[0], m->place[1], 3, 3));
        } else {
            printf("nil\n");
        }
    }

    printf("del hole = %d\n", del_hole(m, 9));
    if (rand_place(m, 3, 3)) {
        printf("%d %d\n", m->place[0], m->place[1]);
    } else {
        printf("nil\n");
    }
    if (rand_place(m, 3, 3)) {
        printf("%d %d\n", m->place[0], m->place[1]);
    } else {
        printf("nil\n");
    }

    destory(m);
}
