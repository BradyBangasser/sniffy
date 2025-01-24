#include "status.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

static struct _StatusNode *statuses = NULL;
static uint8_t len = 0;
static struct winsize ws = {0};

void _print_status() {
    struct _StatusNode *curs = statuses;

    printf(
            "\033[H"
            "\033[J"
          );

    while (curs != NULL) {
        uint16_t width = ws.ws_col - 4;
        uint16_t progress = width * (curs->s.current * 100 / curs->s.expected) / 100;
        uint16_t i;

        printf(" J%lX %s %s (%d/%d)\n", (uint64_t) curs, STATUS_STATE_STRINGS[curs->s.state], "person", curs->s.current, curs->s.expected);
        printf(" [");
        for (i = 0; i < progress; i++) printf("=");
        for (; i < width; i++) printf("-");
        printf("] ");
        fflush(stdout);
        curs = curs->next;
    }
}

void update_state() { _print_status(); }

uint8_t status_init() {
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    return 0;
}

Status *status_state_create(uint32_t expected, enum StatusState initial_state) {
    if (ws.ws_col == 0) {
        status_init();
    }

    struct _StatusNode *node = calloc(sizeof(struct _StatusNode), 1);
    if (node == NULL) {
        return NULL;
    }

    node->s.expected = expected;
    node->s.state = initial_state;

    struct _StatusNode *curs = statuses;

    if (curs == NULL) {
        statuses = node;
    } else {
        while (curs->next != NULL) curs = curs->next;
        curs->next = node;
        node->prev = curs;
    }

    _print_status();
    return &node->s;
}
