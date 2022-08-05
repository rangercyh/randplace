#include "rand_place.h"
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

struct left_area {
    int ldx;
    int ldy;

    int w;
    int h;

    int mark;   // delete mark
    AREA_TYPE type;
    int area;
    struct left_area *next;
};

struct head_left {
    int total_area;
    int num;
    struct left_area *l;
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
    struct left_area *a = (struct left_area *)malloc(sizeof(struct left_area));
    a->ldx = x;
    a->ldy = y;
    a->mark = 0;
    a->type = type;
    a->next = NULL;
    switch (type) {
        case AREA:
            a->w = temp_w;
            a->h = temp_h;
            a->area = temp_w * temp_h * 10; // area span 10 times distinct from line point
            break;
        case LINEX:
            a->w = temp_w;
            a->h = 0;
            a->area = temp_w;
            break;
        case LINEY:
            a->w = 0;
            a->h = temp_h;
            a->area = temp_h;
            break;
        case POINT:
            a->w = 0;
            a->h = 0;
            a->area = 1;
            break;
        default:
            return;
    }
    if (hl->l == NULL) {
        hl->l = a;
    } else {
        struct left_area *l = hl->l;
        while (l->next != NULL) {
            l = l->next;
        }
        l->next = a;
    }
    hl->total_area += a->area;
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
        r = r->next;
    }
    // check bound
    check_bound(m->p, type, w_span, h_span, x, y, w, h);

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
    struct left_area *temp = hl->l;
    while (temp != NULL) {
        DEBUG_PRINT("result x,y,w,h(%d,%d ====> %d,%d) (%d,%d)\n", temp->ldx,
            temp->ldy, temp->w, temp->h, temp->type, temp->area);
        temp = temp->next;
    }
}

static inline void
release_resource(struct coord *x, struct coord *y, struct head_left *hl) {
    struct left_area *p = hl->l;
    while (p != NULL) {
        hl->l = hl->l->next;
        free(p);
        p = hl->l;
    }
    free(hl);
    free(x);
    free(y);
}

static inline int
check_area_same(struct left_area *l, struct left_area *t) {
    int same = 0;
    // check l in t
    switch (l->type) {
        case LINEX:
            if (t->type == LINEX) {
                same = (l->ldx == t->ldx) && (l->ldy == t->ldy);
            }
            if (t->type == AREA) {
                same = (l->ldx == t->ldx) && ((l->ldy == t->ldy) || (l->ldy == (t->ldy + t->h)));
            }
            break;
        case LINEY:
            if (t->type == LINEY) {
                same = (l->ldx == t->ldx) && (l->ldy == t->ldy);
            }
            if (t->type == AREA) {
                same = (l->ldy == t->ldy) && ((l->ldx == t->ldx) || (l->ldx == (t->ldx + t->w)));
            }
            break;
        case POINT:
            if (t->type == LINEX) {
                same = (l->ldy == t->ldy) && ((l->ldx == t->ldx) || (l->ldx == (t->ldx + t->w)));
            }
            if (t->type == LINEY) {
                same = (l->ldx == t->ldx) && ((l->ldy == t->ldy) || (l->ldy == (t->ldy + t->h)));
            }
            if (t->type == AREA) {
                same = ((l->ldx == t->ldx) && (l->ldy == t->ldy)) ||
                       ((l->ldx == t->ldx) && (l->ldy == (t->ldy + t->h))) ||
                       ((l->ldx == (t->ldx + t->w)) && (l->ldy == t->ldy)) ||
                       ((l->ldx == (t->ldx + t->w)) && (l->ldy == (t->ldy + t->h)));
            }
            break;
        default:
            assert("error type area");
            break;
    }
    return same;
}

static inline void
merge_left_area(struct head_left *hl) {
    struct left_area *l = hl->l, *temp;
    while (l != NULL) {
        if (l->type != AREA) {
            temp = hl->l;
            while (temp != NULL) {
                if (temp != l && (temp->mark == 0)) {
                    if (check_area_same(l, temp)) {
                        l->mark = 1;
                        break;
                    }
                }
                temp = temp->next;
            }
        }
        l = l->next;
    }
    while (hl->l != NULL && hl->l->mark == 1) {
        temp = hl->l;
        hl->l = hl->l->next;
        hl->num--;
        hl->total_area -= temp->area;
        free(temp);
    }
    if (hl->l != NULL) {
        l = hl->l;
        temp = l->next;
        while (temp != NULL) {
            if (temp->mark == 1) {
                l->next = temp->next;
                hl->total_area -= temp->area;
                hl->num--;
                free(temp);
                temp = l->next;
            } else {
                l = l->next;
                temp = l->next;
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
    hl->l = NULL;
    get_left_area(hl, m, w, h, x_coord, y_coord);
    print_left_area(hl);

    merge_left_area(hl);
    print_left_area(hl);

    int find = 0;
    if (hl->num > 0 && hl->total_area > 0) {
        int r = rand() % hl->total_area + 1;
        int total = 0; int num = 0;
        struct left_area *l = hl->l;
        while (l != NULL) {
            total += l->area;
            num++;
            if (r <= total) {
                m->place[0] = l->ldx + rand() % (l->w + 1);
                m->place[1] = l->ldy + rand() % (l->h + 1);
                find = 1;
                break;
            }
            l = l->next;
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
