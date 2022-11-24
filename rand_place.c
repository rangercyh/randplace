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
    NONE_TYPE = 0,
    AREA = 0x08,
    LINEX = 0x04,
    LINEY = 0x02,
    POINT = 0x01,

    ALL_TYPE = 0x0f
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
create_rect(int x, int y, int w, int h, int idx) {
    struct rect *r = (struct rect *)malloc(sizeof(struct rect));
    calc_rect_pos(r->p, x, y, w, h);
    r->idx = idx;
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
    m->idx = 0;
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
    struct rect *t = create_rect(x, y, w, h, ++m->idx);
    t->next = m->holes;
    m->holes = t;
    m->hole_num++;
    return m->idx;
}

int
del_hole(struct map *m, int idx) {
    struct rect **list = &m->holes;
    struct rect *temp;
    while (*list) {
        temp = *list;
        if (temp->idx == idx) {
            *list = temp->next;
            free(temp);
            m->hole_num--;
            return 1;
        } else {
            list = &temp->next;
        }
    }
    return 0;
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
        if (p[0] > m->p[0]) {
            insert_coord(x_coord, p[0]);
        }
        insert_coord(x_coord, p[6]);
        if ((p[1] - h) > m->p[1]) {
            insert_coord(y_coord, p[1] - h);
        }
        if (p[1] > m->p[1]) {
            insert_coord(y_coord, p[1]);
        }
        insert_coord(y_coord, p[7]);
        list = list->next;
    }
    p = m->p;
    if ((p[6] - w) > p[0]) {
        insert_coord(x_coord, p[6] - w);
    }
    insert_coord(x_coord, p[6]);
    if ((p[7] - h) > p[1]) {
        insert_coord(y_coord, p[7] - h);
    }
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
            area = (temp_w + 1) * (temp_h + 1);
            break;
        case LINEX:
            il_set(hl->l, pos, enum_W, temp_w);
            il_set(hl->l, pos, enum_H, 0);
            area = temp_w + 1;
            break;
        case LINEY:
            il_set(hl->l, pos, enum_W, 0);
            il_set(hl->l, pos, enum_H, temp_h);
            area = temp_h + 1;
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
        *t &= ~AREA;
        *t &= ~LINEY;
        if (y != up[1]) {
            *t &= ~LINEX;
            *t &= ~POINT;
        }
    }
    if (x >= right[0] && (x + w) <= right[2] && y >= right[1] && (y + h) <= right[3]) {
        *t &= ~AREA;
        *t &= ~LINEX;
        if (x != right[0]) {
            *t &= ~LINEY;
            *t &= ~POINT;
        }
    }
}

static inline AREA_TYPE
check_area(struct head_left *hl, struct map *m, int w_span, int h_span,
        int x, int y, int w, int h, int l_area, int d_area, int ld_area) {
    DEBUG_PRINT("============== check_area (%d,%d,%d,%d) l,d,ld(%d,%d,%d)\n",
        x, y, w, h, l_area, d_area, ld_area);
    struct rect *r = m->holes;
    AREA_TYPE type = ALL_TYPE;
    int check_x_line;
    int check_y_line;
    while (r != NULL) {
        if (check_in_area(r->p, w_span, h_span, x, y, w, h)) {
            type &= ~AREA;
            check_x_line = x - (r->p[0] - w_span);
            if (check_x_line != 0) {
                type &= ~LINEY;
            }
            check_y_line = y - (r->p[1] - h_span);
            if (check_y_line != 0) {
                type &= ~LINEX;
            }
            if (check_x_line != 0 && check_y_line != 0) {
                return NONE_TYPE;
            }
        }
        r = r->next;
    }

    // check bound
    check_bound(m->p, &type, w_span, h_span, x, y, w, h);
    DEBUG_PRINT("type check (%d)\n", type);
    if (type == NONE_TYPE) {
        return NONE_TYPE;
    }

    if (type & AREA) {
        add_left_area(hl, x, y, w, h, AREA);
        return AREA;
    } else {
        if (type & LINEY || type & LINEX) {
            if (type & LINEY) {
                if (l_area == AREA) {
                    return NONE_TYPE;
                }
                add_left_area(hl, x, y, w, h, LINEY);
            }
            if (type & LINEX) {
                if (d_area == AREA) {
                    return NONE_TYPE;
                }
                add_left_area(hl, x, y, w, h, LINEX);
            }
            return type & ~POINT;
        }
        if (type & POINT) {
            if ((l_area == AREA || l_area & LINEX || l_area & LINEY) ||
                (d_area == AREA || d_area & LINEY) ||
                (ld_area == AREA)) {
                return NONE_TYPE;
            }
            add_left_area(hl, x, y, w, h, POINT);
            return POINT;
        }
        return NONE_TYPE;
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
    AREA_TYPE d_area = NONE_TYPE;
    AREA_TYPE pre_y_area[y_coord->num];
    AREA_TYPE ld_area = NONE_TYPE;
    for (int k = 0; k < y_coord->num; k++) {
        pre_y_area[k] = NONE_TYPE;
    }
    for (int i = 0; i < x_coord->num; i++) {
        temp_w = x_coord->c[i] - lastx;
        for (int j = 0; j < y_coord->num; j++) {
            temp_h = y_coord->c[j] - lasty;
            d_area = check_area(hl, m, w, h, lastx, lasty, temp_w, temp_h,
                pre_y_area[j], d_area, (j > 0 ? ld_area : NONE_TYPE));
            DEBUG_PRINT("check_area result ====%d\n", d_area);
            ld_area = pre_y_area[j];
            pre_y_area[j] = d_area;
            lasty = y_coord->c[j];
        }
        d_area = NONE_TYPE;
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
    hl->l = il_create(enum_Num);
    get_left_area(hl, m, w, h, x_coord, y_coord);
    print_left_area(hl);

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
