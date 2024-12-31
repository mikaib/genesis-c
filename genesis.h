#ifndef GENESIS_H
#define GENESIS_H

#ifdef __cplusplus
extern "C"
{
#endif

#define GS_VERSION_MAJOR 0
#define GS_VERSION_MINOR 1
#define GS_VERSION_PATCH 0

#define GS_MAX_VERTEX_LAYOUT_ITEMS 16
#define GS_MAX_COMMAND_LIST_ITEMS 1024
#define GS_MAX_COMMAND_SUBMISSIONS 1024

#define GS_MALLOC(size) malloc(size)
#define GS_ALLOC(obj) GS_ALLOC_MULTIPLE(obj, 1)
#define GS_ALLOC_MULTIPLE(obj, count) (obj*)malloc(sizeof(obj) * count)
#define GS_FREE(obj) free(obj)
#define GS_ASSERT(cond) if(!(cond)) { printf("Assertion failed: %s\n", #cond); exit(1); }
#define GS_MEMSET(ptr, value, size) memset(ptr, value, size)

#define GS_BOOL int
#define GS_TRUE 1
#define GS_FALSE 0

typedef enum {
    GS_BACKEND_GL460
} GsBackendType;

typedef enum {
    GS_ATTRIB_TYPE_UINT8,
    GS_ATTRIB_TYPE_INT16,
    GS_ATTRIB_TYPE_FLOAT
} GsVtxAttribType;

typedef enum {
    GS_COMMAND_NONE,
    GS_COMMAND_CLEAR,
    GS_COMMAND_SET_VIEWPORT,
    GS_COMMAND_USE_PIPELINE,
    GS_COMMAND_USE_BUFFER,
    GS_COMMAND_DRAW_ARRAYS,
    GS_COMMAND_DRAW_INDEXED,
    GS_COMMAND_SET_SCISSOR,
} GsCommandType;

typedef enum {
    GS_CLEAR_COLOR = 1,
    GS_CLEAR_DEPTH = 2,
    GS_CLEAR_STENCIL = 4
} GsClearFlags;

typedef enum {
    GS_BUFFER_TYPE_VERTEX,
    GS_BUFFER_TYPE_INDEX
} GsBufferType;

typedef enum {
    GS_BUFFER_INTENT_DRAW_STATIC,
    GS_BUFFER_INTENT_DRAW_DYNAMIC,
    GS_BUFFER_INTENT_DRAW_STREAM,
    GS_BUFFER_INTENT_READ_STATIC,
    GS_BUFFER_INTENT_READ_DYNAMIC,
    GS_BUFFER_INTENT_READ_STREAM,
    GS_BUFFER_INTENT_COPY_STATIC,
    GS_BUFFER_INTENT_COPY_DYNAMIC,
    GS_BUFFER_INTENT_COPY_STREAM
} GsBufferIntent;

typedef enum {
    GS_SHADER_TYPE_VERTEX,
    GS_SHADER_TYPE_FRAGMENT
} GsShaderType;

typedef struct GsBackend GsBackend;
typedef struct GsConfig GsConfig;
typedef struct GsVtxLayout GsVtxLayout;
typedef struct GsVtxLayoutItem GsVtxLayoutItem;
typedef struct GsCommandList GsCommandList;
typedef struct GsCommandListItem GsCommandListItem;
typedef struct GsPipeline GsPipeline;
typedef struct GsShader GsShader;
typedef struct GsProgram GsProgram;
typedef struct GsBuffer GsBuffer;
typedef struct GsUnmanagedBufferData GsUnmanagedBufferData;
typedef struct GsClearCommand GsClearCommand;
typedef struct GsViewportCommand GsViewportCommand;
typedef struct GsPipelineCommand GsPipelineCommand;
typedef struct GsUseBufferCommand GsUseBufferCommand;
typedef struct GsDrawArraysCommand GsDrawArraysCommand;
typedef struct GsDrawIndexedCommand GsDrawIndexedCommand;
typedef struct GsScissorCommand GsScissorCommand;

typedef struct GsConfig {
    // config
    GsBackend *backend;
    void *window;

    // state
    GsCommandList *command_lists[GS_MAX_COMMAND_SUBMISSIONS];
    int command_list_count;
} GsConfig;

typedef struct GsBackend {
    GsBackendType type;

    // core
    GS_BOOL (*init)(GsBackend *backend, GsConfig *config);
    void (*shutdown)(GsBackend *backend);
    void (*submit)(GsBackend *backend, GsCommandList *list);

    // buffer
    void (*create_buffer_handle)(GsBuffer *buffer);
    void (*set_buffer_data)(GsBuffer *buffer, void *data, int size);
    void (*set_buffer_partial_data)(GsBuffer *buffer, void *data, int size, int offset);
    void (*destroy_buffer_handle)(GsBuffer *buffer);

    // shader
    void (*create_shader_handle)(GsShader *shader, const char *source);
    void (*destroy_shader_handle)(GsShader *shader);

    // program,
    void (*create_program_handle)(GsProgram *program);
    void (*destroy_program_handle)(GsProgram *program);

    // layout
    void (*create_layout_handle)(GsVtxLayout *layout);
    void (*destroy_layout_handle)(GsVtxLayout *layout);
} GsBackend;

typedef struct GsVtxLayoutItem {
    int index;
    int offset;
    int size_total; // size_per_item * components
    int size_per_item; // sizeof(type)
    int components;

    GsVtxAttribType type;
    GS_BOOL normalized;
} GsVtxLayoutItem;

typedef struct GsVtxLayout {
    GsVtxLayoutItem items[GS_MAX_VERTEX_LAYOUT_ITEMS];
    int count;
    int components;
    int stride;
    void *handle;
    GS_BOOL completed;
} GsVtxLayout;

typedef struct GsCommandListItem {
    void *data;
    int size;
    GsCommandType type;
} GsCommandListItem;

typedef struct GsCommandList {
    GsCommandListItem items[GS_MAX_COMMAND_LIST_ITEMS];
    GsPipeline *pipeline;
    int count;
} GsCommandList;

typedef struct GsPipeline {
    GsVtxLayout *layout;
    GsProgram *program;
} GsPipeline;

typedef struct GsBuffer {
    GsBufferType type;
    GsBufferIntent intent;
    void *handle;
} GsBuffer;

typedef struct GsUnmanagedBufferData {
    void *data;
    int size;
} GsUnmanagedBufferData;

typedef struct GsClearCommand {
    float r;
    float g;
    float b;
    float a;
    GsClearFlags flags;
} GsClearCommand;

typedef struct GsViewportCommand {
    int x;
    int y;
    int width;
    int height;
} GsViewportCommand;

typedef struct GsPipelineCommand {
    GsPipeline *pipeline;
} GsPipelineCommand;

typedef struct GsUseBufferCommand {
    GsBuffer *buffer;
} GsUseBufferCommand;

typedef struct GsDrawArraysCommand {
    int start;
    int count;
} GsDrawArraysCommand;

typedef struct GsDrawIndexedCommand {
    int count;
} GsDrawIndexedCommand;

typedef struct GsScissorCommand {
    int x;
    int y;
    int width;
    int height;
    GS_BOOL enable;
} GsScissorCommand;

typedef struct GsShader {
    GsShaderType type;
    void *handle;
} GsShader;

typedef struct GsProgram {
    GsShader *vertex;
    GsShader *fragment;
    GS_BOOL completed;
    void *handle;
} GsProgram;

// Shaders
GsShader *gs_create_shader(GsShaderType type, const char *source);
void gs_destroy_shader(GsShader *shader);

// Programs
GsProgram *gs_create_program();
void gs_program_attach_shader(GsProgram *program, GsShader *shader);
void gs_program_complete(GsProgram *program);
void gs_destroy_program(GsProgram *program);

// Pipeline
GsPipeline *gs_create_pipeline();
void gs_destroy_pipeline(GsPipeline *pipeline);
void gs_pipeline_set_layout(GsPipeline *pipeline, GsVtxLayout *layout);

// Buffers
GsBuffer *gs_create_buffer(GsBufferType type, GsBufferIntent intent);
void gs_destroy_buffer(GsBuffer *buffer);
void gs_buffer_set_data(GsBuffer *buffer, void *data, int size);
void gs_buffer_set_partial_data(GsBuffer *buffer, void *data, int size, int offset);
GsUnmanagedBufferData *gs_buffer_get_data(GsBuffer *buffer, int offset, int size);
void gs_destroy_unmanaged_buffer_data(GsUnmanagedBufferData *data);

// Command list
GsCommandList *gs_create_command_list();
void gs_command_list_begin(GsCommandList *list);
void gs_command_list_add(GsCommandList *list, GsCommandType type, void *data, int size);
void gs_command_list_clear(GsCommandList *list);
void gs_clear(GsCommandList *list, GsClearFlags flags, float r, float g, float b, float a);
void gs_set_viewport(GsCommandList *list, int x, int y, int width, int height);
void gs_use_pipeline(GsCommandList *list, GsPipeline *pipeline);
void gs_use_buffer(GsCommandList *list, GsBuffer *buffer);
void gs_set_scissor(GsCommandList *list, int x, int y, int width, int height);
void gs_disable_scissor(GsCommandList *list);
void gs_draw_arrays(GsCommandList *list, int start, int count);
void gs_draw_indexed(GsCommandList *list, int count);
void gs_command_list_end(GsCommandList *list);
void gs_command_list_submit(GsCommandList *list);
void gs_destroy_command_list(GsCommandList *list);

// Vertex Layout
GS_BOOL gs_layout_add(GsVtxLayout *layout, int index, GsVtxAttribType type, int count);
GsVtxLayout *gs_create_layout();
void gs_destroy_layout(GsVtxLayout *layout);
void gs_layout_complete(GsVtxLayout *layout);

// Global
GS_BOOL gs_init(GsConfig *config);
void gs_handle_internal_command(GsCommandListItem item);
void gs_shutdown();
void gs_discard_frame();
void gs_frame();

// Config
void gs_destroy_config(GsConfig *config);
GsConfig *gs_create_config();

// Backend
GsBackend *gs_create_backend(GsBackendType type);
void gs_destroy_backend(GsBackend *backend);
GsBackendType gs_get_optimal_backend_type();

// optional mainloop wrapper
void gs_create_mainloop(void (*mainloop)());
void gs_stop_mainloop();

#ifdef __cplusplus
}
#endif

#endif //GENESIS_H
