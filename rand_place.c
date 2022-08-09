#include "rand_place.h"
#include "intlist.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...)
#endif

typedef enum {
    NONE = -1,
    AREA,
    LINEX,
    LINEY,
    POINT,

    ALL_TYPE
} AREA_TYPE;

enum {
    enum_LDX = 0,
    enum_LDY = 1,
    enum_W = 2,
    enum_H = 3,
    enum_MARK = 4,
    enum_TYPE = 5,
    enum_AREA = 6,

    enum_Num = 7
};

struct head_left {
    int total_area;
    int num;
    IntList *l;
};

struct coord {
    int num;
    int c[0];
};

static inline void
calc_rect_pos(int *p, int x, int y, int w, int h) {
    assert(p != NULL && w > 0 && h > 0);
    p[0] = x; p[1] = y;         // left down
    p[2] = x; p[3] = y + h;     // left top
    p[4] = x + w; p[5] = y;     // right dow
    p[6] = x + w; p[7] = y + h; // right top
}

static inline struct rect *
create_rect(int x, int y, int w, int h) {
    struct rect *r = (struct rect *)malloc(sizeof(struct rect));
    calc_rect_pos(r->p, x, y, w, h);
    r->next = NULL;
    return r;
}

struct map *
create_map(int x, int y, int w, int h) {
    srand((unsigned)time(NULL));
    struct map *m = (struct map *)malloc(sizeof(struct map));
    calc_rect_pos(m->p, x, y, w, h);
    m->holes = NULL;
    m->hole_num = 0;
    m->place[0] = m->place[1] = 0;
    m->w = w;
    m->h = h;
    return m;
}

static inline int
check_in_area(int *p, int w_span, int h_span, int x, int y, int w, int h) {
    if (x >= (p[0] - w_span) && (x + w) <= p[6] && y >= (p[1] - h_span) && (y + h) <= p[7]) {
        return 1;
    }
    return 0;
}

int
dig_hole(struct map *m, int x, int y, int w, int h) {
    if (!check_in_area(m->p, 0, 0, x, y, w, h)) {
        return 0;
    }
    struct rect *list = m->holes;
    if (list == NULL) {
        m->holes = create_rect(x, y, w, h);
    } else {
        while (list->next != NULL) {
            list = list->next;
        }
        list->next = create_rect(x, y, w, h);
    }
    m->hole_num++;
    return 1;
}

static inline void
insert_coord(struct coord *c, int v) {
    for (int i = 0; i < c->num; i++) {
        if (c->c[i] == v) {
            return;
        }
        if (c->c[i] > v) {
            for (int j = c->num; j > i; j--) {
                c->c[j] = c->c[j - 1];
            }
            c->c[i] = v;
            c->num++;
            return;
        }
    }
    c->c[c->num++] = v;
}

static inline void
get_coord(struct coord *x_coord, struct coord *y_coord, struct map *m, int w, int h) {
    struct rect *list = m->holes;
    int *p;
    while (list != NULL) {
        p = list->p;
        if ((p[0] - w) > m->p[0]) {
            insert_coord(x_coord, p[0] - w);
        }
        insert_coord(x_coord, p[0]);
        insert_coord(x_coord, p[6]);
        if ((p[1] - h) > m->p[1]) {
            insert_coord(y_coord, p[1] - h);
        }
        insert_coord(y_coord, p[1]);
        insert_coord(y_coord, p[7]);
        list = list->next;
    }
    p = m->p;
    insert_coord(x_coord, p[6] - w);
    insert_coord(x_coord, p[6]);
    insert_coord(y_coord, p[7] - h);
    insert_coord(y_coord, p[7]);
}

static inline void
add_left_area(struct head_left *hl, int x, int y, int temp_w,
        int temp_h, AREA_TYPE type) {
    int pos = il_insert(hl->l);
    il_set(hl->l, pos, enum_LDX, x);
    il_set(hl->l, pos, enum_LDY, y);
    il_set(hl->l, pos, enum_MARK, 0);
    il_set(hl->l, pos, enum_TYPE, type);
    int area = 0;
    switch (type) {
        case AREA:
            il_set(hl->l, pos, enum_W, temp_w);
            il_set(hl->l, pos, enum_H, temp_h);
            area = temp_w * temp_h * 10;
            break;
        case LINEX:
            il_set(hl->l, pos, enum_W, temp_w);
            il_set(hl->l, pos, enum_H, 0);
            area = temp_w;
            break;
        case LINEY:
            il_set(hl->l, pos, enum_W, 0);
            il_set(hl->l, pos, enum_H, temp_h);
            area = temp_h;
            break;
        case POINT:
            il_set(hl->l, pos, enum_W, 0);
            il_set(hl->l, pos, enum_H, 0);
            area = 1;
            break;
        default:
            return;
    }
    il_set(hl->l, pos, enum_AREA, area);
    hl->total_area += area;
    hl->num++;
}

static inline void
check_bound(int *p, AREA_TYPE *t, int w_span, int h_span, int x, int y, int w, int h) {
    int up[4] = { p[0], p[7] - h_span, p[6], p[7] };
    int right[4] = { p[6] - w_span, p[1], p[6], p[7] };
    if (x >= up[0] && (x + w) <= up[2] && y >= up[1] && (y + h) <= up[3]) {
        t[AREA] = 0;
        t[LINEY] = 0;
        if (y != up[1]) {
            t[LINEX] = 0;
            t[POINT] = 0;
        }
    }
    if (x >= right[0] && (x + w) <= right[2] && y >= right[1] && (y + h) <= right[3]) {
        t[AREA] = 0;
        t[LINEX] = 0;
        if (x != right[0]) {
            t[LINEY] = 0;
            t[POINT] = 0;
        }
    }
}

static inline void
check_area(struct head_left *hl, struct map *m, int w_span, int h_span,
        int x, int y, int w, int h) {
    DEBUG_PRINT("============== check_area (%d,%d,%d,%d)\n", x, y, w, h);
    struct rect *r = m->holes;
    AREA_TYPE type[ALL_TYPE] = { 1, 1, 1, 1 };
    int check_x_line;
    int check_y_line;
    while (r != NULL) {
        if (check_in_area(r->p, w_span, h_span, x, y, w, h)) {
            type[AREA] = 0;
            check_x_line = x - (r->p[0] - w_span);
            if (check_x_line != 0) {
                type[LINEY] = 0;
            }
            check_y_line = y - (r->p[1] - h_span);
            if (check_y_line != 0) {
                type[LINEX] = 0;
            }
            if (check_x_line != 0 && check_y_line != 0) {
                type[POINT] = 0;
            }
        }
        if (!type[AREA] && !type[LINEX] && !type[LINEY] && !type[POINT]) {
            break;
        }
        r = r->next;
    }
    if (type[AREA] | type[LINEX] | type[LINEY] | type[POINT]) {
        // check bound
        check_bound(m->p, type, w_span, h_span, x, y, w, h);
    }

    if (type[AREA] == 1) {
        add_left_area(hl, x, y, w, h, AREA);
        return;
    }
    if (type[LINEX] == 1 || type[LINEY] == 1) {
        if (type[LINEX] == 1) {
            add_left_area(hl, x, y, w, h, LINEX);
        }
        if (type[LINEY] == 1) {
            add_left_area(hl, x, y, w, h, LINEY);
        }
        return;
    }
    if (type[POINT] == 1) {
        add_left_area(hl, x, y, w, h, POINT);
    }
}

static inline void
print_coord(struct coord *x_coord, struct coord *y_coord) {
    DEBUG_PRINT("-------------\nx-coord: ");
    for (int k1 = 0; k1 < x_coord->num; k1++) {
        DEBUG_PRINT("%d ", x_coord->c[k1]);
    }
    DEBUG_PRINT("\ny-coord: ");
    for (int k2 = 0; k2 < y_coord->num; k2++) {
        DEBUG_PRINT("%d ", y_coord->c[k2]);
    }
    DEBUG_PRINT("\n-------------\n");
}

static inline void
get_left_area(struct head_left *hl, struct map *m, int w, int h,
        struct coord *x_coord, struct coord *y_coord) {
    int lastx = m->p[0];
    int lasty = m->p[1];
    int temp_w, temp_h;
    for (int i = 0; i < x_coord->num; i++) {
        temp_w = x_coord->c[i] - lastx;
        if (temp_w > 0) {
            for (int j = 0; j < y_coord->num; j++) {
                temp_h = y_coord->c[j] - lasty;
                if (temp_h > 0) {
                    check_area(hl, m, w, h, lastx, lasty, temp_w, temp_h);
                }
                lasty = y_coord->c[j];
            }
        }
        lastx = x_coord->c[i];
        lasty = m->p[1];
    }
}

static inline void
print_left_area(struct head_left *hl) {
    DEBUG_PRINT("============== total_area %d %d\n", hl->total_area, hl->num);
    assert(hl->num == il_size(hl->l));
    for (int i = 0; i < hl->num; i++) {
        if (il_get(hl->l, i, enum_MARK) == 0) {
            DEBUG_PRINT("result x,y,w,h(%d,%d ====> %d,%d) (%d,%d)\n",
                il_get(hl->l, i, enum_LDX),
                il_get(hl->l, i, enum_LDY),
                il_get(hl->l, i, enum_W),
                il_get(hl->l, i, enum_H),
                il_get(hl->l, i, enum_TYPE),
                il_get(hl->l, i, enum_AREA));
        }
    }
}

static inline void
release_resource(struct coord *x, struct coord *y, struct head_left *hl) {
    il_destroy(hl->l);
    free(hl);
    free(x);
    free(y);
}

static inline int
check_area_same(IntList *list, int l, int t) {
    int same = 0;
    // check l in t
    int l_type = il_get(list, l, enum_TYPE);
    int l_ldx = il_get(list, l, enum_LDX);
    int l_ldy = il_get(list, l, enum_LDY);
    int t_type = il_get(list, t, enum_TYPE);
    int t_ldx = il_get(list, t, enum_LDX);
    int t_ldy = il_get(list, t, enum_LDY);
    int t_w = il_get(list, t, enum_W);
    int t_h = il_get(list, t, enum_H);
    switch (l_type) {
        case LINEX:
            if (t_type == LINEX) {
                same = (l_ldx == t_ldx) && (l_ldy == t_ldy);
            }
            if (t_type == AREA) {
                same = (l_ldx == t_ldx) && ((l_ldy == t_ldy) || (l_ldy == (t_ldy + t_h)));
            }
            break;
        case LINEY:
            if (t_type == LINEY) {
                same = (l_ldx == t_ldx) && (l_ldy == t_ldy);
            }
            if (t_type == AREA) {
                same = (l_ldy == t_ldy) && ((l_ldx == t_ldx) || (l_ldx == (t_ldx + t_w)));
            }
            break;
        case POINT:
            if (t_type == LINEX) {
                same = (l_ldy == t_ldy) && ((l_ldx == t_ldx) || (l_ldx == (t_ldx + t_w)));
            }
            if (t_type == LINEY) {
                same = (l_ldx == t_ldx) && ((l_ldy == t_ldy) || (l_ldy == (t_ldy + t_h)));
            }
            if (t_type == AREA) {
                same = ((l_ldx == t_ldx) && (l_ldy == t_ldy)) ||
                       ((l_ldx == t_ldx) && (l_ldy == (t_ldy + t_h))) ||
                       ((l_ldx == (t_ldx + t_w)) && (l_ldy == t_ldy)) ||
                       ((l_ldx == (t_ldx + t_w)) && (l_ldy == (t_ldy + t_h)));
            }
            break;
        default:
            assert(0);
            break;
    }
    return same;
}

static inline void
merge_left_area(struct head_left *hl) {
    for (int i = 0; i < hl->num; i++) {
        if (il_get(hl->l, i, enum_TYPE) != AREA) {
            for (int j = 0; j < hl->num; j++) {
                if (i != j && il_get(hl->l, i, enum_MARK) == 0) {
                    if (check_area_same(hl->l, i, j)) {
                        il_set(hl->l, i, enum_MARK, 1);
                        hl->total_area -= il_get(hl->l, i, enum_AREA);
                        break;
                    }
                }
            }
        }
    }
}

int
rand_place(struct map *m, int w, int h) {
    assert(w > 0 && h > 0);
    if (w > m->w || h > m->h) {
        return 0;
    }
    int len = m->hole_num * 3 + 2;
    struct coord *x_coord = (struct coord *)malloc(sizeof(struct coord) + len * sizeof(int));
    x_coord->num = 0;
    struct coord *y_coord = (struct coord *)malloc(sizeof(struct coord) + len * sizeof(int));
    y_coord->num = 0;

    get_coord(x_coord, y_coord, m, w, h);
    print_coord(x_coord, y_coord);

    struct head_left *hl = (struct head_left *)malloc(sizeof(struct head_left));
    hl->total_area = 0;
    hl->num = 0;
    hl->l = (IntList *)malloc(sizeof(IntList));
    il_create(hl->l, enum_Num);
    get_left_area(hl, m, w, h, x_coord, y_coord);
    print_left_area(hl);

    // merge_left_area(hl);
    // print_left_area(hl);

    int find = 0;
    if (hl->num > 0 && hl->total_area > 0) {
        int r = rand() % hl->total_area + 1;
        int total = 0;
        int ldx, ldy, rw, rh;
        for (int i = 0; i < hl->num; i++) {
            if (il_get(hl->l, i, enum_MARK) == 0) {
                total += il_get(hl->l, i, enum_AREA);
                if (r <= total) {
                    ldx = il_get(hl->l, i, enum_LDX);
                    ldy = il_get(hl->l, i, enum_LDY);
                    rw = il_get(hl->l, i, enum_W);
                    rh = il_get(hl->l, i, enum_H);
                    m->place[0] = ldx + rand() % (rw + 1);
                    m->place[1] = ldy + rand() % (rh + 1);
                    find = 1;
                    break;
                }
            }
        }
    }
    release_resource(x_coord, y_coord, hl);
    return find;
}

void
destory(struct map *m) {
    assert(m != NULL);
    struct rect *list, *temp;
    list = m->holes;
    while (list != NULL) {
        temp = list->next;
        free(list);
        list = temp;
    }
    free(m);
}
