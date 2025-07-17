#include "genesis.h"
#include <stdlib.h>

static GS_BOOL gs_noop_init(GsBackend *backend, GsConfig *config) {
    backend->capabilities = 0;
    return GS_TRUE;
}

static void gs_noop_shutdown(GsBackend *backend) {}

static void gs_noop_submit(GsBackend *backend, GsCommandList *list) {}

static void gs_noop_create_buffer(GsBuffer *buffer) { buffer->handle = 0; }
static void gs_noop_set_buffer_data(GsBuffer *buffer, void *data, int size) {}
static void gs_noop_set_buffer_partial_data(GsBuffer *buffer, void *data, int size, int offset) {}
static void gs_noop_destroy_buffer(GsBuffer *buffer) { buffer->handle = 0; }

static void gs_noop_create_shader(GsShader *shader, const char *source) { shader->handle = 0; }
static void gs_noop_destroy_shader(GsShader *shader) { shader->handle = 0; }

static void gs_noop_create_program(GsProgram *program) { program->handle = 0; }
static void gs_noop_destroy_program(GsProgram *program) { program->handle = 0; }

static GsUniformLocation gs_noop_get_uniform_location(GsProgram *program, const char *name) { return -1; }

static void gs_noop_create_layout(GsVtxLayout *layout) {}
static void gs_noop_destroy_layout(GsVtxLayout *layout) {}

static void gs_noop_create_texture(GsTexture *texture) { texture->handle = 0; }
static void gs_noop_set_texture_data(GsTexture *texture, GsCubemapFace face, void *data) {}
static void gs_noop_generate_mipmaps(GsTexture *texture) {}
static void gs_noop_clear_texture(GsTexture *texture) {}
static void gs_noop_update_texture_state(GsTexture *texture) {}
static void gs_noop_destroy_texture(GsTexture *texture) { texture->handle = 0; }

static void gs_noop_create_render_pass(GsRenderPass *pass) {}
static void gs_noop_destroy_render_pass(GsRenderPass *pass) {}

static void gs_noop_create_framebuffer(GsFramebuffer *framebuffer) { framebuffer->handle = 0; }
static void gs_noop_destroy_framebuffer(GsFramebuffer *framebuffer) { framebuffer->handle = 0; }
static void gs_noop_framebuffer_attach_texture(GsFramebuffer *framebuffer, GsTexture *texture, GsFramebufferAttachmentType attachment) {}

GsBackend *gs_noop_create() {
    GsBackend *backend = GS_ALLOC(GsBackend);

    backend->type = GS_BACKEND_NOOP;
    backend->init = gs_noop_init;
    backend->shutdown = gs_noop_shutdown;
    backend->submit = gs_noop_submit;

    backend->create_buffer_handle = gs_noop_create_buffer;
    backend->set_buffer_data = gs_noop_set_buffer_data;
    backend->set_buffer_partial_data = gs_noop_set_buffer_partial_data;
    backend->destroy_buffer_handle = gs_noop_destroy_buffer;

    backend->create_shader_handle = gs_noop_create_shader;
    backend->destroy_shader_handle = gs_noop_destroy_shader;

    backend->create_program_handle = gs_noop_create_program;
    backend->destroy_program_handle = gs_noop_destroy_program;

    backend->get_uniform_location = gs_noop_get_uniform_location;

    backend->create_layout_handle = gs_noop_create_layout;
    backend->destroy_layout_handle = gs_noop_destroy_layout;

    backend->create_texture_handle = gs_noop_create_texture;
    backend->set_texture_data = gs_noop_set_texture_data;
    backend->generate_mipmaps = gs_noop_generate_mipmaps;
    backend->destroy_texture_handle = gs_noop_destroy_texture;
    backend->clear_texture = gs_noop_clear_texture;

    backend->create_render_pass_handle = gs_noop_create_render_pass;
    backend->destroy_render_pass_handle = gs_noop_destroy_render_pass;

    backend->create_framebuffer = gs_noop_create_framebuffer;
    backend->destroy_framebuffer = gs_noop_destroy_framebuffer;
    backend->framebuffer_attach_texture = gs_noop_framebuffer_attach_texture;

    return backend;
}

