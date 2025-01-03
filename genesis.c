#include "genesis.h"
#include "genesis_opengl.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
#endif

static GsConfig *active_config = NULL;
static GS_BOOL mainloop_active = GS_FALSE;

GsVtxLayout *gs_create_layout() {
    GsVtxLayout *layout = GS_ALLOC(GsVtxLayout);
    layout->count = 0;
    layout->stride = 0;
    layout->components = 0;
    layout->completed = GS_FALSE;

    return layout;
}

GS_BOOL gs_layout_add(GsVtxLayout *layout, const int index, const GsVtxAttribType type, const int count) {
    GS_ASSERT(layout != NULL);
    GS_ASSERT(layout->count < GS_MAX_VERTEX_LAYOUT_ITEMS);

    GsVtxLayoutItem item = layout->items[layout->count];
    item.index = index;
    item.type = type;
    item.size_total = 0;
    item.size_per_item = 0;
    item.offset = layout->stride;
    item.normalized = GS_FALSE;
    item.components = count;

    switch (type) {
        case GS_ATTRIB_TYPE_UINT8:
            item.size_per_item = (int) sizeof(uint8_t);
            break;
        case GS_ATTRIB_TYPE_INT16:
            item.size_per_item = (int) sizeof(int16_t);
            break;
        case GS_ATTRIB_TYPE_FLOAT:
            item.size_per_item = (int) sizeof(float);
            break;
        default:
            return GS_FALSE;
    }

    item.size_total = item.size_per_item * count;

    layout->items[layout->count] = item;
    layout->count += 1;
    layout->stride += item.size_total;

    return GS_TRUE;
}

void gs_destroy_layout(GsVtxLayout *layout) {
    GS_ASSERT(layout != NULL);

    if (layout->completed) {
        GS_ASSERT(active_config != NULL);
        GS_ASSERT(active_config->backend != NULL);

        active_config->backend->destroy_layout_handle(layout);
    }

    GS_FREE(layout);
}

GS_BOOL gs_init(GsConfig *config) {
    GS_ASSERT(config != NULL);
    GS_ASSERT(config->backend != NULL);
    GS_ASSERT(active_config == NULL);

    if (!config->backend->init(config->backend, config)) {
        return GS_FALSE;
    }

    active_config = config;
    return GS_TRUE;
}

void gs_layout_complete(GsVtxLayout *layout) {
    GS_ASSERT(layout != NULL);
    GS_ASSERT(layout->count > 0);
    GS_ASSERT(layout->completed == GS_FALSE);
    GS_ASSERT(active_config != NULL);
    GS_ASSERT(active_config->backend != NULL);

    active_config->backend->create_layout_handle(layout);

    layout->completed = GS_TRUE;
}

GsConfig *gs_create_config() {
    GsConfig *config = GS_ALLOC(GsConfig);
    config->backend = NULL;
    config->window = NULL;
    config->command_list_count = 0;
    
    return config;
}

void gs_destroy_config(GsConfig *config) {
    GS_ASSERT(config != NULL);
    GS_ASSERT(active_config == NULL || active_config != config);

    GS_FREE(config);
}

GsBackend *gs_create_backend(const GsBackendType type) {
    if (type == GS_BACKEND_GL460) {
        return gs_opengl_create();
    }

    return NULL;
}

void gs_destroy_backend(GsBackend *backend) {
    GS_ASSERT(backend != NULL);
    GS_ASSERT(active_config == NULL || active_config->backend != backend);

    GS_FREE(backend);
}

GsBackendType gs_get_optimal_backend_type() {
    return GS_BACKEND_GL460;
}

void gs_shutdown() {
    GS_ASSERT(active_config != NULL);
    GS_ASSERT(active_config->backend != NULL);

    active_config->backend->shutdown(active_config->backend);
    active_config = NULL;
}

GsPipeline *gs_create_pipeline() {
    GsPipeline *pipeline = GS_ALLOC(GsPipeline);
    pipeline->layout = NULL;

    return pipeline;
}

void gs_pipeline_set_layout(GsPipeline *pipeline, GsVtxLayout *layout) {
    GS_ASSERT(pipeline != NULL);
    GS_ASSERT(layout != NULL);

    pipeline->layout = layout;
}

void gs_destroy_pipeline(GsPipeline *pipeline) {
    GS_ASSERT(pipeline != NULL);
    GS_FREE(pipeline);
}

GsBuffer *gs_create_buffer(const GsBufferType type, const GsBufferIntent intent) {
    GS_ASSERT(active_config != NULL);
    GS_ASSERT(active_config->backend != NULL);

    GsBuffer *buffer = GS_ALLOC(GsBuffer);
    buffer->type = type;
    buffer->intent = intent;
    buffer->handle = NULL;

    active_config->backend->create_buffer_handle(buffer);

    return buffer;
}

void gs_destroy_buffer(GsBuffer *buffer) {
    GS_ASSERT(buffer != NULL);
    GS_ASSERT(active_config != NULL);
    GS_ASSERT(active_config->backend != NULL);

    active_config->backend->destroy_buffer_handle(buffer);
    GS_FREE(buffer);
}

void gs_buffer_set_data(GsBuffer *buffer, void *data, int size) {
    GS_ASSERT(buffer != NULL);
    GS_ASSERT(data != NULL);
    GS_ASSERT(size > 0);
    GS_ASSERT(active_config != NULL);
    GS_ASSERT(active_config->backend != NULL);

    active_config->backend->set_buffer_data(buffer, data, size);
}

void gs_buffer_set_partial_data(GsBuffer *buffer, void *data, int size, int offset) {
    GS_ASSERT(buffer != NULL);
    GS_ASSERT(data != NULL);
    GS_ASSERT(size > 0);
    GS_ASSERT(offset >= 0);
    GS_ASSERT(active_config != NULL);
    GS_ASSERT(active_config->backend != NULL);

    active_config->backend->set_buffer_partial_data(buffer, data, size, offset);
}

void gs_destroy_unmanaged_buffer_data(GsUnmanagedBufferData *data) {
    GS_ASSERT(data != NULL);
    GS_FREE(data->data);
    GS_FREE(data);
}

GsCommandList *gs_create_command_list() {
    GsCommandList *list = GS_ALLOC(GsCommandList);
    list->count = 0;
    list->pipeline = NULL;

    return list;
}

void gs_destroy_command_list(GsCommandList *list) {
    GS_ASSERT(list != NULL);
    gs_command_list_clear(list); // Free any remaining data before freeing the list itself.
    GS_FREE(list);
}

void gs_command_list_begin(GsCommandList *list) {
    GS_ASSERT(list != NULL);
    gs_command_list_clear(list);
}

void gs_command_list_end(GsCommandList *list) {
    // TODO: Do any validation or processing here.
}

void gs_command_list_submit(GsCommandList *list) {
    GS_ASSERT(list != NULL);
    GS_ASSERT(active_config != NULL);
    GS_ASSERT(active_config->command_list_count < GS_MAX_COMMAND_SUBMISSIONS);

    active_config->command_lists[active_config->command_list_count] = list;
    active_config->command_list_count += 1;
}

void gs_handle_internal_command(const GsCommandListItem item) {
    printf("Genesis unhandled command type: %d\n", item.type);
    // TODO: Implement any internal commands here.
}

void gs_frame() {
    GS_ASSERT(active_config != NULL);
    GS_ASSERT(active_config->backend != NULL);

    for (int i = 0; i < active_config->command_list_count; i++) {
        active_config->backend->submit(active_config->backend, active_config->command_lists[i]);
    }

    active_config->command_list_count = 0;
}

void gs_discard_frame() {
    GS_ASSERT(active_config != NULL);
    active_config->command_list_count = 0;
}

void gs_command_list_add(GsCommandList *list, const GsCommandType type, void *data, const int size) {
    GS_ASSERT(list != NULL);
    GS_ASSERT(list->count < GS_MAX_COMMAND_LIST_ITEMS);

    GsCommandListItem item;
    item.data = data;
    item.size = size;
    item.type = type;

    list->items[list->count] = item;
    list->count += 1;
}

void gs_command_list_clear(GsCommandList *list) {
    GS_ASSERT(list != NULL);

    for (int i = 0; i < list->count; i++) {
        GS_FREE(list->items[i].data);
        list->items[i].data = NULL;
    }

    list->count = 0;
}

void gs_clear(GsCommandList *list, const GsClearFlags flags, const float r, const float g, const float b, const float a) {
    GS_ASSERT(list != NULL);

    GsClearCommand *data = GS_ALLOC(GsClearCommand);
    data->r = r;
    data->g = g;
    data->b = b;
    data->a = a;
    data->flags = flags;

    gs_command_list_add(list, GS_COMMAND_CLEAR, data, sizeof(GsClearCommand));
}

void gs_set_viewport(GsCommandList *list, const int x, const int y, const int w, const int h) {
    GS_ASSERT(list != NULL);

    GsViewportCommand *data = GS_ALLOC(GsViewportCommand);
    data->x = x;
    data->y = y;
    data->width = w;
    data->height = h;

    gs_command_list_add(list, GS_COMMAND_SET_VIEWPORT, data, sizeof(GsViewportCommand));
}

void gs_use_pipeline(GsCommandList *list, GsPipeline *pipeline) {
    GS_ASSERT(list != NULL);
    GS_ASSERT(pipeline != NULL);

    GsPipelineCommand *data = GS_ALLOC(GsPipelineCommand);
    data->pipeline = pipeline;

    gs_command_list_add(list, GS_COMMAND_USE_PIPELINE, data, sizeof(GsPipelineCommand));
}

void gs_use_buffer(GsCommandList *list, GsBuffer *buffer) {
    GS_ASSERT(list != NULL);
    GS_ASSERT(buffer != NULL);

    GsUseBufferCommand *data = GS_ALLOC(GsUseBufferCommand);
    data->buffer = buffer;

    gs_command_list_add(list, GS_COMMAND_USE_BUFFER, data, sizeof(GsUseBufferCommand));
}

void gs_draw_arrays(GsCommandList *list, const int start, const int count) {
    GS_ASSERT(list != NULL);

    GsDrawArraysCommand *data = GS_ALLOC(GsDrawArraysCommand);
    data->start = start;
    data->count = count;

    gs_command_list_add(list, GS_COMMAND_DRAW_ARRAYS, data, sizeof(GsDrawArraysCommand));
}

void gs_draw_indexed(GsCommandList *list, const int count) {
    GS_ASSERT(list != NULL);

    GsDrawIndexedCommand *data = GS_ALLOC(GsDrawIndexedCommand);
    data->count = count;

    gs_command_list_add(list, GS_COMMAND_DRAW_INDEXED, data, sizeof(GsDrawIndexedCommand));
}

void gs_set_scissor(GsCommandList *list, const int x, const int y, const int w, const int h) {
    GS_ASSERT(list != NULL);

    GsScissorCommand *data = GS_ALLOC(GsScissorCommand);
    data->x = x;
    data->y = y;
    data->width = w;
    data->height = h;
    data->enable = GS_TRUE;

    gs_command_list_add(list, GS_COMMAND_SET_SCISSOR, data, sizeof(GsScissorCommand));
}

void gs_disable_scissor(GsCommandList *list) {
    GS_ASSERT(list != NULL);

    GsScissorCommand *data = GS_ALLOC(GsScissorCommand);
    data->enable = GS_FALSE;

    gs_command_list_add(list, GS_COMMAND_SET_SCISSOR, data, sizeof(GsScissorCommand));
}

GsShader *gs_create_shader(const GsShaderType type, const char *source) {
    GS_ASSERT(active_config != NULL);
    GS_ASSERT(active_config->backend != NULL);

    GsShader *shader = GS_ALLOC(GsShader);
    shader->type = type;
    shader->handle = NULL;

    active_config->backend->create_shader_handle(shader, source);

    return shader;
}

void gs_destroy_shader(GsShader *shader) {
    GS_ASSERT(shader != NULL);
    GS_ASSERT(active_config != NULL);
    GS_ASSERT(active_config->backend != NULL);

    active_config->backend->destroy_shader_handle(shader);

    GS_FREE(shader);
}

GsProgram *gs_create_program() {
    GsProgram *program = GS_ALLOC(GsProgram);
    program->vertex = NULL;
    program->fragment = NULL;
    program->completed = GS_FALSE;
    program->handle = NULL;

    return program;
}

void gs_program_attach_shader(GsProgram *program, GsShader *shader) {
    GS_ASSERT(program != NULL);
    GS_ASSERT(shader != NULL);

    switch (shader->type) {
        case GS_SHADER_TYPE_VERTEX:
            program->vertex = shader;
            break;
        case GS_SHADER_TYPE_FRAGMENT:
            program->fragment = shader;
            break;
        default:
            break;
    }
}

void gs_program_complete(GsProgram *program) {
    GS_ASSERT(program != NULL);
    GS_ASSERT(program->completed == GS_FALSE);
    GS_ASSERT(active_config != NULL);
    GS_ASSERT(active_config->backend != NULL);

    active_config->backend->create_program_handle(program);

    program->completed = GS_TRUE;
}

void gs_destroy_program(GsProgram *program) {
    GS_ASSERT(program != NULL);
    GS_ASSERT(active_config != NULL);
    GS_ASSERT(active_config->backend != NULL);

    if (program->completed) {
        active_config->backend->destroy_program_handle(program);
    }

    GS_FREE(program);
}

void gs_create_mainloop(void (*mainloop)()) {
    mainloop_active = GS_TRUE;

    #ifdef __EMSCRIPTEN__
        emscripten_set_main_loop(mainloop, 0, 1);
    #else
        while (mainloop_active == GS_TRUE) {
            mainloop();
        }
    #endif
}

void gs_stop_mainloop() {
    mainloop_active = GS_FALSE;
}

