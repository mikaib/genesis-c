#ifndef GENESIS_NOOP_H
#define GENESIS_NOOP_H

#include "genesis.h"

#ifdef __cplusplus
extern "C" {
#endif

// Creation / destruction
GsBackend *gs_noop_create();
GS_BOOL gs_noop_init(GsBackend *backend, GsConfig *config);
void gs_noop_shutdown(GsBackend *backend);

// Command submission
void gs_noop_submit(GsBackend *backend, GsCommandList *list);

// Buffer
void gs_noop_create_buffer(GsBuffer *buffer);
void gs_noop_set_buffer_data(GsBuffer *buffer, void *data, int size);
void gs_noop_set_buffer_partial_data(GsBuffer *buffer, void *data, int size, int offset);
void gs_noop_destroy_buffer(GsBuffer *buffer);

// Shader
void gs_noop_create_shader(GsShader *shader, const char *source);
void gs_noop_destroy_shader(GsShader *shader);

// Program
void gs_noop_create_program(GsProgram *program);
void gs_noop_destroy_program(GsProgram *program);

// Uniforms
GsUniformLocation gs_noop_get_uniform_location(GsProgram *program, const char *name);

// Layout
void gs_noop_create_layout(GsVtxLayout *layout);
void gs_noop_destroy_layout(GsVtxLayout *layout);

// Texture
void gs_noop_create_texture(GsTexture *texture);
void gs_noop_set_texture_data(GsTexture *texture, GsCubemapFace face, void *data);
void gs_noop_generate_mipmaps(GsTexture *texture);
void gs_noop_clear_texture(GsTexture *texture);
void gs_noop_update_texture_state(GsTexture *texture);
void gs_noop_destroy_texture(GsTexture *texture);

// Render pass
void gs_noop_create_render_pass(GsRenderPass *pass);
void gs_noop_destroy_render_pass(GsRenderPass *pass);

// Framebuffer
void gs_noop_create_framebuffer(GsFramebuffer *framebuffer);
void gs_noop_destroy_framebuffer(GsFramebuffer *framebuffer);
void gs_noop_framebuffer_attach_texture(GsFramebuffer *framebuffer, GsTexture *texture, GsFramebufferAttachmentType attachment);

#ifdef __cplusplus
}
#endif

#endif // GENESIS_NOOP_H
