#ifndef GENESIS_H
#define GENESIS_H

#ifdef __cplusplus
extern "C"
{
#endif

#define GS_VERSION_MAJOR 0
#define GS_VERSION_MINOR 1
#define GS_VERSION_PATCH 0

#define GS_MAX_VERTEX_LAYOUT_ITEMS 128
#define GS_MAX_TEXTURE_SLOTS 16
#define GS_MAX_COMMAND_LIST_ITEMS 4096
#define GS_MAX_COMMAND_SUBMISSIONS 128

#define GS_MALLOC(size) malloc(size)
#define GS_ALLOC_MULTIPLE(obj, count) (obj*)malloc(sizeof(obj) * count)
#define GS_ASSERT(cond) if(!(cond)) { printf("Assertion failed: %s, file: %s, line: %d\n", #cond, __FILE__, __LINE__); exit(1); }
#define GS_ASSERT_WARN(cond, message) if(!(cond)) { printf("Warning: %s, file: %s, line: %d\n", message, __FILE__, __LINE__); }
#define GS_MEMSET(ptr, value, size) memset(ptr, value, size)

#define GS_ALLOC(obj) GS_ALLOC_MULTIPLE(obj, 1)
#define GS_FREE(obj) free(obj)

#define GS_TABLE_SIZE(arr) (int)(sizeof(arr) / sizeof((arr)[0]))
#define GS_BOOL int
#define GS_TRUE 1
#define GS_FALSE 0

typedef enum {
    GS_BACKEND_OPENGL
} GsBackendType;

typedef enum {
    GS_CAPABILITY_RENDERER = 1 << 0,
} GsCapability;

typedef enum {
    GS_ATTRIB_TYPE_UINT8,
    GS_ATTRIB_TYPE_INT16,
    GS_ATTRIB_TYPE_FLOAT,
    GS_ATTRIB_TYPE_UINT16,
    GS_ATTRIB_TYPE_UINT32,
    GS_ATTRIB_TYPE_INT32,
    GS_ATTRIB_TYPE_INT8,
    GS_ATTRIB_TYPE_DOUBLE,
} GsVtxAttribType;

typedef enum {
    GS_COMMAND_NONE,
    GS_COMMAND_CLEAR,
    GS_COMMAND_SET_VIEWPORT,
    GS_COMMAND_USE_PIPELINE,
    GS_COMMAND_USE_BUFFER,
    GS_COMMAND_USE_TEXTURE,
    GS_COMMAND_DRAW_ARRAYS,
    GS_COMMAND_DRAW_INDEXED,
    GS_COMMAND_SET_SCISSOR,
    GS_COMMAND_SET_UNIFORM_INT,
    GS_COMMAND_SET_UNIFORM_FLOAT,
    GS_COMMAND_SET_UNIFORM_VEC2,
    GS_COMMAND_SET_UNIFORM_VEC3,
    GS_COMMAND_SET_UNIFORM_VEC4,
    GS_COMMAND_SET_UNIFORM_MAT4,
    GS_COMMAND_COPY_TEXTURE,
    GS_COMMAND_COPY_TEXTURE_PARTIAL,
    GS_COMMAND_RESOLVE_TEXTURE,
    GS_COMMAND_GEN_MIPMAPS,
    GS_COMMAND_BEGIN_PASS,
    GS_COMMAND_END_PASS,
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

typedef enum {
    GS_TEXTURE_FORMAT_RGB8,
    GS_TEXTURE_FORMAT_RGBA8,
    GS_TEXTURE_FORMAT_RGB16F,
    GS_TEXTURE_FORMAT_RGBA16F,
    GS_TEXTURE_FORMAT_DEPTH24_STENCIL8,
    GS_TEXTURE_FORMAT_DEPTH32F
} GsTextureFormat;

typedef enum {
    GS_TEXTURE_WRAP_REPEAT,
    GS_TEXTURE_WRAP_CLAMP,
    GS_TEXTURE_WRAP_MIRROR
} GsTextureWrap;

typedef enum {
    GS_TEXTURE_FILTER_NEAREST,
    GS_TEXTURE_FILTER_LINEAR,
    GS_TEXTURE_FILTER_MIPMAP_NEAREST,
    GS_TEXTURE_FILTER_MIPMAP_LINEAR
} GsTextureFilter;

typedef enum {
    GS_TEXTURE_TYPE_2D,
    GS_TEXTURE_TYPE_CUBEMAP
} GsTextureType;

typedef enum {
    GS_CUBEMAP_FACE_UP,
    GS_CUBEMAP_FACE_DOWN,
    GS_CUBEMAP_FACE_LEFT,
    GS_CUBEMAP_FACE_RIGHT,
    GS_CUBEMAP_FACE_FRONT,
    GS_CUBEMAP_FACE_BACK,
    GS_CUBEMAP_FACE_NONE
} GsCubemapFace;

typedef enum {
    GS_BLEND_FACTOR_ZERO,
    GS_BLEND_FACTOR_ONE,
    GS_BLEND_FACTOR_SRC_COLOR,
    GS_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
    GS_BLEND_FACTOR_DST_COLOR,
    GS_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
    GS_BLEND_FACTOR_SRC_ALPHA,
    GS_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    GS_BLEND_FACTOR_DST_ALPHA,
    GS_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
    GS_BLEND_FACTOR_CONSTANT_COLOR,
    GS_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
    GS_BLEND_FACTOR_CONSTANT_ALPHA,
    GS_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,
    GS_BLEND_FACTOR_SRC_ALPHA_SATURATE
} GsBlendFactor;

typedef enum {
    GS_BLEND_OP_ADD,
    GS_BLEND_OP_SUBTRACT,
    GS_BLEND_OP_REVERSE_SUBTRACT,
    GS_BLEND_OP_MIN,
    GS_BLEND_OP_MAX
} GsBlendOp;

typedef enum {
    GS_DEPTH_FUNC_NEVER,
    GS_DEPTH_FUNC_LESS,
    GS_DEPTH_FUNC_EQUAL,
    GS_DEPTH_FUNC_LESS_EQUAL,
    GS_DEPTH_FUNC_GREATER,
    GS_DEPTH_FUNC_NOT_EQUAL,
    GS_DEPTH_FUNC_GREATER_EQUAL,
    GS_DEPTH_FUNC_ALWAYS
} GsDepthFunc;

typedef enum {
    GS_FRAMEBUFFER_ATTACHMENT_COLOR,
    GS_FRAMEBUFFER_ATTACHMENT_DEPTH,
    GS_FRAMEBUFFER_ATTACHMENT_STENCIL,
    GS_FRAMEBUFFER_ATTACHMENT_DEPTH_STENCIL
} GsFramebufferAttachmentType;

typedef int GsUniformLocation;
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
typedef struct GsTexture GsTexture;
typedef struct GsFramebuffer GsFramebuffer;
typedef struct GsUnmanagedBufferData GsUnmanagedBufferData;
typedef struct GsClearCommand GsClearCommand;
typedef struct GsViewportCommand GsViewportCommand;
typedef struct GsPipelineCommand GsPipelineCommand;
typedef struct GsTextureCommand GsTextureCommand;
typedef struct GsUseBufferCommand GsUseBufferCommand;
typedef struct GsDrawArraysCommand GsDrawArraysCommand;
typedef struct GsDrawIndexedCommand GsDrawIndexedCommand;
typedef struct GsScissorCommand GsScissorCommand;
typedef struct GsUniformIntCommand GsUniformIntCommand;
typedef struct GsUniformFloatCommand GsUniformFloatCommand;
typedef struct GsUniformVec2Command GsUniformVec2Command;
typedef struct GsUniformVec3Command GsUniformVec3Command;
typedef struct GsUniformVec4Command GsUniformVec4Command;
typedef struct GsUniformMat4Command GsUniformMat4Command;
typedef struct GsCopyTextureCommand GsCopyTextureCommand;
typedef struct GsCopyTexturePartialCommand GsCopyTexturePartialCommand;
typedef struct GsResolveTextureCommand GsResolveTextureCommand;
typedef struct GsGenMipmapsCommand GsGenMipmapsCommand;
typedef struct GsBeginRenderPassCommand GsBeginRenderPassCommand;
typedef struct GsEndRenderPassCommand GsEndRenderPassCommand;

typedef struct GsConfig {
    // config
    GsBackend *backend;
    void *window;

    // state
    GsCommandList *command_lists[GS_MAX_COMMAND_SUBMISSIONS];
    int command_list_count;
} GsConfig;

typedef struct GsRenderPass {
    GsFramebuffer *framebuffer;
    void *handle;
} GsRenderPass;

typedef struct GsBackend {
    GsBackendType type;
    GsCapability capabilities;

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
    GsUniformLocation (*get_uniform_location)(GsProgram *program, const char *name);
    void (*destroy_program_handle)(GsProgram *program);

    // layout
    void (*create_layout_handle)(GsVtxLayout *layout);
    void (*destroy_layout_handle)(GsVtxLayout *layout);

    // texture
    void (*create_texture_handle)(GsTexture *texture);
    void (*set_texture_data)(GsTexture *texture, GsCubemapFace face, void *data);
    void (*generate_mipmaps)(GsTexture *texture);
    void (*clear_texture)(GsTexture *texture);
    void (*destroy_texture_handle)(GsTexture *texture);

    // render pass
    void (*create_render_pass_handle)(GsRenderPass *pass);
    void (*destroy_render_pass_handle)(GsRenderPass *pass);

    // framebuffer
    void (*create_framebuffer)(GsFramebuffer *framebuffer);
    void (*destroy_framebuffer)(GsFramebuffer *framebuffer);
    void (*framebuffer_attach_texture)(GsFramebuffer *framebuffer, GsTexture *texture, GsFramebufferAttachmentType attachment);
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
    // general state
    GsVtxLayout *layout;
    GsProgram *program;
    int msaa_samples;

    // blend
    GsBlendOp blend_op;
    GsBlendFactor blend_src;
    GsBlendFactor blend_dst;
    GsBlendOp blend_op_alpha;
    GsBlendFactor blend_src_alpha;
    GsBlendFactor blend_dst_alpha;
    GS_BOOL blend_enabled;

    // stencil
    GS_BOOL stencil_test;

    // depth
    GsDepthFunc depth_func;
    GS_BOOL depth_write;
    GS_BOOL depth_test;
} GsPipeline;

typedef struct GsBuffer {
    GsBufferType type;
    GsBufferIntent intent;
    void *handle;
} GsBuffer;

typedef struct GsFramebuffer {
    void *handle;
    int width;
    int height;
} GsFramebuffer;

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

typedef struct GsUniformIntCommand {
    GsUniformLocation location;
    int value;
} GsUniformIntCommand;

typedef struct GsUniformFloatCommand {
    GsUniformLocation location;
    float value;
} GsUniformFloatCommand;

typedef struct GsUniformVec2Command {
    GsUniformLocation location;
    float x;
    float y;
} GsUniformVec2Command;

typedef struct GsUniformVec3Command {
    GsUniformLocation location;
    float x;
    float y;
    float z;
} GsUniformVec3Command;

typedef struct GsUniformVec4Command {
    GsUniformLocation location;
    float x;
    float y;
    float z;
    float w;
} GsUniformVec4Command;

typedef struct GsUniformMat4Command {
    GsUniformLocation location;
    float m00;
    float m01;
    float m02;
    float m03;
    float m10;
    float m11;
    float m12;
    float m13;
    float m20;
    float m21;
    float m22;
    float m23;
    float m30;
    float m31;
    float m32;
    float m33;
} GsUniformMat4Command;

typedef struct GsPipelineCommand {
    GsPipeline *pipeline;
} GsPipelineCommand;

typedef struct GsTextureCommand {
    GsTexture *texture;
    int slot;
} GsTextureCommand;

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

typedef struct GsTexture {
    int width;
    int height;
    float lodBias;
    GsTextureFormat format;
    GsTextureWrap wrap_s;
    GsTextureWrap wrap_t;
    GsTextureWrap wrap_r; // cubemap
    GsTextureFilter min;
    GsTextureFilter mag;
    GsTextureType type;
    void *handle;
} GsTexture;

typedef struct GsCopyTextureCommand {
    GsTexture *src;
    GsTexture *dst;
} GsCopyTextureCommand;

typedef struct GsCopyTexturePartialCommand {
    GsTexture *src;
    GsTexture *dst;
    int src_x;
    int src_y;
    int dst_x;
    int dst_y;
    int width;
    int height;
} GsCopyTexturePartialCommand;

typedef struct GsResolveTextureCommand {
    GsTexture *src;
    GsTexture *dst;
} GsResolveTextureCommand;

typedef struct GsGenMipmapsCommand {
    GsTexture *texture;
} GsGenMipmapsCommand;

typedef struct GsBeginRenderPassCommand {
    GsRenderPass *pass;
} GsBeginRenderPassCommand;

typedef struct GsEndRenderPassCommand {
    int dummy;
} GsEndRenderPassCommand;

// Textures
GsTexture *gs_create_texture(int width, int height, GsTextureFormat format, GsTextureWrap wrap_s, GsTextureWrap wrap_t, GsTextureFilter min, GsTextureFilter mag);
GsTexture *gs_create_cubemap(int width, int height, GsTextureFormat format, GsTextureWrap wrap_s, GsTextureWrap wrap_t, GsTextureWrap wrap_r, GsTextureFilter min, GsTextureFilter mag);
void gs_texture_set_data(GsTexture *texture, void *data);
void gs_texture_set_face_data(GsTexture *texture, GsCubemapFace face, void *data);
void gs_texture_generate_mipmaps(GsTexture *texture);
void gs_texture_clear(GsTexture *texture);
void gs_destroy_texture(GsTexture *texture);

// Render Pass
GsRenderPass *gs_create_render_pass(GsFramebuffer *framebuffer);
void gs_destroy_render_pass(GsRenderPass *pass);

// Shaders
GsShader *gs_create_shader(GsShaderType type, const char *source);
void gs_destroy_shader(GsShader *shader);

// Programs
GsProgram *gs_create_program();
void gs_program_attach_shader(GsProgram *program, GsShader *shader);
void gs_program_build(GsProgram *program);
GsUniformLocation gs_get_uniform_location(GsProgram *program, const char *name);
void gs_destroy_program(GsProgram *program);

// Pipeline
GsPipeline *gs_create_pipeline();
void gs_destroy_pipeline(GsPipeline *pipeline);
void gs_pipeline_set_layout(GsPipeline *pipeline, GsVtxLayout *layout);

// Framebuffers
GsFramebuffer *gs_create_framebuffer(int width, int height);
void gs_destroy_framebuffer(GsFramebuffer *framebuffer);
void gs_framebuffer_attach_texture(GsFramebuffer *framebuffer,  GsTexture *texture, GsFramebufferAttachmentType type);

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
void gs_use_texture(GsCommandList *list, GsTexture *texture, int slot);
void gs_set_scissor(GsCommandList *list, int x, int y, int width, int height);
void gs_disable_scissor(GsCommandList *list);
void gs_draw_arrays(GsCommandList *list, int start, int count);
void gs_draw_indexed(GsCommandList *list, int count);
void gs_uniform_set_int(GsCommandList *list, GsUniformLocation location, int value);
void gs_uniform_set_float(GsCommandList *list, GsUniformLocation location, float value);
void gs_uniform_set_vec2(GsCommandList *list, GsUniformLocation location, float x, float y);
void gs_uniform_set_vec3(GsCommandList *list, GsUniformLocation location, float x, float y, float z);
void gs_uniform_set_vec4(GsCommandList *list, GsUniformLocation location, float x, float y, float z, float w);
void gs_uniform_set_mat4(GsCommandList *list, GsUniformLocation location, float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33);
void gs_copy_texture(GsCommandList *list, GsTexture *src, GsTexture *dst);
void gs_resolve_texture(GsCommandList *list, GsTexture *src, GsTexture *dst);
void gs_copy_texture_partial(GsCommandList *list, GsTexture *src, GsTexture *dst, int src_x, int src_y, int dst_x, int dst_y, int width, int height);
void gs_generate_mipmaps(GsCommandList *list, GsTexture *texture);
void gs_begin_render_pass(GsCommandList *list, GsRenderPass *pass);
void gs_end_render_pass(GsCommandList *list);
void gs_command_list_end(GsCommandList *list);
void gs_command_list_submit(GsCommandList *list);
void gs_destroy_command_list(GsCommandList *list);

// Vertex Layout
GS_BOOL gs_layout_add(GsVtxLayout *layout, int index, GsVtxAttribType type, int count);
GsVtxLayout *gs_create_layout();
void gs_destroy_layout(GsVtxLayout *layout);
void gs_layout_build(GsVtxLayout *layout);

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

// caps
GS_BOOL gs_has_capability(GsCapability capability);

// optional mainloop wrapper
void gs_create_mainloop(void (*mainloop)());
void gs_stop_mainloop();

#ifdef __cplusplus
}
#endif

#endif //GENESIS_H
