#include <yed/plugin.h>

typedef char *str;
use_tree_c(str, str, strcmp);

tree(str, str) map;

void unload(yed_plugin *self);
void map_extension(int n_args, char **args);
void maybe_change_ft(yed_buffer *buff);
void maybe_change_ft_event(yed_event *event);

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler buff_post_load_handler;
    yed_event_handler buff_pre_write_handler;

    YED_PLUG_VERSION_CHECK();

    map = tree_make(str, str);

    yed_plugin_set_command(self, "map-extension", map_extension);

    buff_post_load_handler.kind = EVENT_BUFFER_POST_LOAD;
    buff_post_load_handler.fn   = maybe_change_ft_event;
    buff_pre_write_handler.kind = EVENT_BUFFER_PRE_WRITE;
    buff_pre_write_handler.fn   = maybe_change_ft_event;

    yed_plugin_add_event_handler(self, buff_post_load_handler);
    yed_plugin_add_event_handler(self, buff_pre_write_handler);

    return 0;
}

void unload(yed_plugin *self) {
    tree_it(str, str)  it;
    char              *key;

    while (tree_len(map) > 0) {
        it  = tree_begin(map);
        key = tree_it_key(it);
        free(tree_it_val(it));
        tree_delete(map, key);
        free(key);
    }

    tree_free(map);
}

void map_extension(int n_args, char **args) {
    char                                         *ext;
    char                                         *ft;
    tree_it(str, str)                             it;
    char                                         *key;
    tree_it(yed_buffer_name_t, yed_buffer_ptr_t)  bit;

    if (n_args != 2) {
        yed_cerr("expected 2 arguments, but got %d", n_args);
        return;
    }

    ext = strdup(args[0]);
    ft  = strdup(args[1]);

    it = tree_lookup(map, ext);
    if (tree_it_good(it)) {
        key = tree_it_key(it);
        free(tree_it_val(it));
        tree_delete(map, key);
        free(key);
    }

    tree_insert(map, ext, ft);

    yed_cprint("mapped extension '%s' to ft %s", ext, ft);

    tree_traverse(ys->buffers, bit) {
        maybe_change_ft(tree_it_val(bit));
    }
}

void maybe_change_ft(yed_buffer *buff) {
    const char        *ext;
    tree_it(str, str)  it;

    if (buff->ft != FT_UNKNOWN) {
        return;
    }
    if (buff->path == NULL) {
        return;
    }
    if ((ext = get_path_ext(buff->path)) == NULL) {
        return;
    }

    it = tree_lookup(map, (char*)ext);
    if (!tree_it_good(it)) {
        return;
    }

    yed_buffer_set_ft(buff, yed_get_ft(tree_it_val(it)));
}

void maybe_change_ft_event(yed_event *event) {
    if (event->buffer) {
        maybe_change_ft(event->buffer);
    }
}
