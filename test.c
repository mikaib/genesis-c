#include <stdio.h>
#include "genesis.h"

int main() {
    GsBackendType type = gs_get_optimal_backend_type();
    GsBackend* backend = gs_create_backend(type);

    GsConfig* config = gs_create_config();
    config->backend = backend;
    config->window = NULL;

    GsVtxLayout* layout = gs_create_layout();
    gs_layout_add(layout, 0, GS_ATTRIB_TYPE_FLOAT, 2); // position
    gs_layout_add(layout, 1, GS_ATTRIB_TYPE_FLOAT, 4); // color

    GsCommandList *list = gs_create_command_list();
    gs_command_list_begin(list);
    gs_clear(list, GS_CLEAR_COLOR | GS_CLEAR_DEPTH, 0, 0, 0, 1);
    gs_command_list_end(list);

    gs_init(config);

    // ... every frame
    gs_command_list_submit(list);
    // every frame ...

    gs_shutdown();

    gs_destroy_command_list(list);
    gs_destroy_layout(layout);
    gs_destroy_config(config);
    gs_destroy_backend(backend);

    return 0;
}
