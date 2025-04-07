#ifndef GENESIS_GL460_H
#define GENESIS_GL460_H

#include "genesis.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef void (*GsCommandHandler)(const GsCommandListItem);
typedef struct GsOpenGLBufferHandle {
    unsigned int handle;
    unsigned int vaoHandle; // used if the buffer type is a vertex buffer
    GsBuffer* lastIndexBuffer; // to remove unnecessary calls to glBindBuffer
    GsVtxLayout* lastLayout; // be able to tell if layout has changed
} GsOpenGLBufferHandle;

// creation / destruction
GsBackend *gs_opengl_create();
GS_BOOL gs_opengl_init(GsBackend *backend, GsConfig *config);
void gs_opengl_shutdown(GsBackend *backend);

// shaders
void gs_opengl_create_shader(GsShader *shader, const char *source);
void gs_opengl_destroy_shader(GsShader *shader);

// programs
void gs_opengl_create_program(GsProgram *program);
void gs_opengl_destroy_program(GsProgram *program);

// uniforms
GsUniformLocation gs_opengl_get_uniform_location(GsProgram *program, const char *name);
void gs_opengl_cmd_set_uniform_int(const GsCommandListItem item);
void gs_opengl_cmd_set_uniform_float(const GsCommandListItem item);
void gs_opengl_cmd_set_uniform_vec2(const GsCommandListItem item);
void gs_opengl_cmd_set_uniform_vec3(const GsCommandListItem item);
void gs_opengl_cmd_set_uniform_vec4(const GsCommandListItem item);
void gs_opengl_cmd_set_uniform_mat4(const GsCommandListItem item);

// layout
void gs_opengl_create_layout(GsVtxLayout *layout);
void gs_opengl_destroy_layout(GsVtxLayout *layout);

// commands
void gs_opengl_cmd_clear(const GsCommandListItem item);
void gs_opengl_cmd_set_viewport(const GsCommandListItem item);
void gs_opengl_cmd_use_pipeline(const GsCommandListItem item);
void gs_opengl_cmd_use_buffer(const GsCommandListItem item);
void gs_opengl_cmd_use_texture(const GsCommandListItem item);
void gs_opengl_cmd_use_framebuffer(const GsCommandListItem item);
void gs_opengl_cmd_draw_arrays(const GsCommandListItem item);
void gs_opengl_cmd_draw_indexed(const GsCommandListItem item);
void gs_opengl_cmd_set_scissor(const GsCommandListItem item);
void gs_opengl_cmd_copy_texture(const GsCommandListItem item);
void gs_opengl_cmd_resolve_texture(const GsCommandListItem item);
void gs_opengl_cmd_generate_mipmaps(const GsCommandListItem item);
void gs_opengl_cmd_copy_texture_partial(const GsCommandListItem item);
void gs_opengl_submit(GsBackend *backend, GsCommandList *list);

// bind resources
void gs_opengl_internal_bind_buffer(GsBuffer *buffer);
void gs_opengl_internal_unbind_buffer(GsBufferType type);
void gs_opengl_internal_bind_program(GsProgram *program);
void gs_opengl_internal_unbind_program();
void gs_opengl_internal_bind_layout(GsVtxLayout *layout);
void gs_opengl_internal_unbind_layout();
void gs_opengl_internal_bind_state();
void gs_opengl_internal_bind_layout_state();
void gs_opengl_internal_bind_texture(GsTexture *texture, int slot);
void gs_opengl_internal_unbind_texture(int slot);
void gs_opengl_internal_bind_framebuffer(GsFramebuffer *framebuffer);
void gs_opengl_internal_unbind_framebuffer();

// textures
void gs_opengl_create_texture(GsTexture *texture);
void gs_opengl_set_texture_data(GsTexture *texture, GsCubemapFace face, void *data);
void gs_opengl_generate_mipmaps(GsTexture *texture);
void gs_opengl_clear_texture(GsTexture *texture);
void gs_opengl_update_texture_state(GsTexture *texture);
void gs_opengl_destroy_texture(GsTexture *texture);

// buffer
void gs_opengl_create_buffer(GsBuffer *buffer);
void gs_opengl_set_buffer_data(GsBuffer *buffer, void *data, int size);
void gs_opengl_set_buffer_partial_data(GsBuffer *buffer, void *data, int size, int offset);
void gs_opengl_destroy_buffer(GsBuffer *buffer);

// type conversions
int gs_opengl_get_buffer_type(GsBufferType type);
int gs_opengl_get_buffer_intent(GsBufferIntent intent);
int gs_opengl_get_attrib_type(GsVtxAttribType type);
int gs_opengl_get_face_type(GsCubemapFace face);
int gs_opengl_get_texture_format(GsTextureFormat format);
int gs_opengl_get_texture_type(GsTextureType type);
int gs_opengl_get_texture_wrap(GsTextureWrap wrap);
int gs_opengl_get_texture_filter(GsTextureFilter filter);
int gs_opengl_get_depth_func(GsDepthFunc func);
int gs_opengl_get_blend_factor(GsBlendFactor factor);
int gs_opengl_get_blend_op(GsBlendOp op);
int gs_opengl_get_texture_format(GsTextureFormat format);

// framebuffer
void gs_opengl_create_framebuffer(GsFramebuffer *framebuffer);
void gs_opengl_destroy_framebuffer(GsFramebuffer *framebuffer);
void gs_opengl_framebuffer_attach_texture(GsFramebuffer *framebuffer, GsTexture *texture, GsFramebufferAttachmentType attachment);

// platform
void *gs_opengl_getproc(const char *name);

#ifdef __cplusplus
}
#endif

#endif //GENESIS_GL460_H
