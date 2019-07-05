/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2019  Free Software Foundation, Inc.
 *
 *  GRUB is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  GRUB is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GRUB.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <grub/dl.h>
#include <grub/env.h>
#include <grub/mm.h>
#include <grub/command.h>
#include <grub/video.h>

GRUB_MOD_LICENSE ("GPLv3+");

struct deepin_gfxmode_node {
    unsigned int width;
    unsigned int height;
    struct deepin_gfxmode_node *next;
};

typedef struct deepin_gfxmode_node deepin_gfxmode_node_t;

struct hook_context {
    unsigned int current_width;
    unsigned int current_height;
    deepin_gfxmode_node_t list;
};

struct string_builder {
    char* buf;
    int cap;
    int len;
};

typedef struct string_builder string_builder_t;


void deepin_gfxmode_list_add(deepin_gfxmode_node_t *p_head, unsigned int width, unsigned int height);
void deepin_gfxmode_list_display(deepin_gfxmode_node_t* p_head);
int deepin_gfxmode_list_contains(deepin_gfxmode_node_t *p_head, unsigned int width, unsigned int height);
void deepin_gfxmode_list_free(deepin_gfxmode_node_t* p_head);
char *deepin_gfxmode_list_join(struct hook_context *ctx);

static void string_builder_init(string_builder_t *sb, int cap);
static void string_builder_grow(string_builder_t *sb, int n);
static void string_builder_write_char(string_builder_t *sb, char c);
static void string_builder_write_string(string_builder_t *sb, char *src);
static void string_builder_end(string_builder_t *sb);

int deepin_gfxmode_list_contains(deepin_gfxmode_node_t *p_head, unsigned int width, unsigned int height) {
    deepin_gfxmode_node_t* p;
    for (p = p_head->next; p != NULL; p = p->next) {
        if (p->width == width && p->height == height) {
            return 1;
        }
    }
    return 0;
}

void deepin_gfxmode_list_add(deepin_gfxmode_node_t *p_head, unsigned int width, unsigned int height) {
    if (deepin_gfxmode_list_contains(p_head, width, height)) {
        return;
    }

    deepin_gfxmode_node_t *new_node;
    new_node = (deepin_gfxmode_node_t*)grub_malloc(sizeof(deepin_gfxmode_node_t));
    new_node->width = width;
    new_node->height = height;
    new_node->next = p_head->next;
    p_head->next = new_node;
}

void deepin_gfxmode_list_display(deepin_gfxmode_node_t* p_head) {
    deepin_gfxmode_node_t *p;
    for (p = p_head->next; p != NULL; p = p->next) {
        grub_printf("%ux%u\n", p->width, p->height);
    }
}

void deepin_gfxmode_list_free(deepin_gfxmode_node_t* p_head) {
    deepin_gfxmode_node_t *p, *prev;
    p = p_head->next;
    while (p != NULL) {
        prev = p;
        p = p->next;
        grub_free(prev);
    }
}


static void
string_builder_init(string_builder_t *sb, int cap) {
    if (cap < 10 ) {
        cap = 10;
    }

    sb->buf = (char *)grub_malloc(cap);
    sb->cap = cap;
    sb->len = 0;
}

static void
string_builder_grow(string_builder_t *sb, int n) {
    int new_cap = sb->cap * 2 + n;
    char* new_buf = (char *)grub_malloc(new_cap);
    grub_memcpy(new_buf, sb->buf, sb->len);
    grub_free(sb->buf);
    sb->buf = new_buf;
    sb->cap = new_cap;
    grub_printf("grow %d\n", new_cap);
}

static void
string_builder_write_char(string_builder_t *sb, char c) {
    if (sb->len == sb->cap) {
        string_builder_grow(sb,0);
    }

    sb->buf[sb->len] = c;
    sb->len++;
}

static void
string_builder_write_string(string_builder_t *sb, char *src) {
    int src_len = grub_strlen(src);
    if (sb->len + src_len + 1>= sb->cap) {
        string_builder_grow(sb, src_len+1);
    }
    grub_strcpy(sb->buf + sb->len, src);
    sb->len += src_len;
}



void string_builder_end(string_builder_t *sb) {
    string_builder_write_char(sb, 0);
}

char *deepin_gfxmode_list_join(struct hook_context *ctx) {
    deepin_gfxmode_node_t *p_head = &ctx->list;

    deepin_gfxmode_node_t *p;
    int count = 0;
    int current_idx = -1;

    for (p = p_head->next; p != NULL; p = p->next) {
        if (p->width == ctx->current_width && p->height == ctx->current_height) {
            current_idx = count;
            break;
        }
        count++;
    }
    //grub_printf("current index: %d\n", current_idx);

    string_builder_t sb;
    string_builder_init(&sb, 256);

    char *current_idx_str = grub_xasprintf("%d", current_idx);
    string_builder_write_string(&sb, current_idx_str);
    grub_free(current_idx_str);

    string_builder_write_char(&sb, ',');
    for (p = p_head->next; p != NULL; p = p->next) {
        char *str = grub_xasprintf("%ux%u", p->width, p->height);
        string_builder_write_string(&sb, str);
        grub_free(str);
        if (p->next != NULL) {
            string_builder_write_char(&sb, ',');
        }
    }

    string_builder_end(&sb);
    return sb.buf;
}

static int
hook (const struct grub_video_mode_info *info, void *hook_arg)
{
    struct hook_context *ctx = hook_arg;

    if ((ctx->current_width == info->width &&
        ctx->current_height == info->height) ||
        info->width >= 1024 ) {
        deepin_gfxmode_list_add(&ctx->list, info->width, info->height);
    }
    return 0;
}

static grub_err_t grub_cmd_deepin_gfxmode(struct grub_command *cmd __attribute__((unused)),
    int argc __attribute__((unused)),
    char *argv[] __attribute__((unused))) {

    grub_video_driver_id_t id;
    grub_video_adapter_t adapter;
    id = grub_video_get_driver_id ();
    FOR_VIDEO_ADAPTERS (adapter)
    {
        struct grub_video_mode_info info;
        if (!adapter->iterate) {
            continue;
        }

        if (adapter->id == id) {
            struct hook_context ctx;

            if (grub_video_get_info (&info) == GRUB_ERR_NONE) {
                //grub_printf("width: %u\n", info.width);
                //grub_printf("height: %u\n", info.height);
                //grub_printf("bpp: %u\n", info.bpp);
                ctx.current_width = info.width;
                ctx.current_height = info.height;
            } else {
                grub_errno = GRUB_ERR_NONE;
                ctx.current_width = 0;
                ctx.current_height = 0;
            }

            ctx.list.width = 0;
            ctx.list.height = 0;
            ctx.list.next = NULL;
            adapter->iterate(hook, &ctx);
            char *deepin_gfxmode = deepin_gfxmode_list_join(&ctx);
            grub_env_set("DEEPIN_GFXMODE", deepin_gfxmode);
            grub_free(deepin_gfxmode);
            deepin_gfxmode_list_free(&ctx.list);
        }
    }
    return 0;
}

static grub_command_t cmd_deepin_gfxmode;

GRUB_MOD_INIT(deepin_gfxmode) {
    cmd_deepin_gfxmode = grub_register_command("deepin_gfxmode", grub_cmd_deepin_gfxmode,
        0, "deepin gfxmode");
}

GRUB_MOD_FINI(deepin_gfxmode) {
    grub_unregister_command(cmd_deepin_gfxmode);
}