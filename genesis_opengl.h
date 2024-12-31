#ifndef GENESIS_GL460_H
#define GENESIS_GL460_H

#include "genesis.h"

#ifdef __cplusplus
extern "C"
{
#endif

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

// layout
void gs_opengl_create_layout(GsVtxLayout *layout);
void gs_opengl_destroy_layout(GsVtxLayout *layout);

// commands
void gs_opengl_cmd_clear(const GsCommandListItem item);
void gs_opengl_cmd_set_viewport(const GsCommandListItem item);
void gs_opengl_cmd_use_pipeline(const GsCommandListItem item);
void gs_opengl_cmd_use_buffer(const GsCommandListItem item);
void gs_opengl_cmd_draw_arrays(const GsCommandListItem item);
void gs_opengl_cmd_draw_indexed(const GsCommandListItem item);
void gs_opengl_cmd_set_scissor(const GsCommandListItem item);
void gs_opengl_submit(GsBackend *backend, GsCommandList *list);

// bind resources
void gs_opengl_internal_bind_buffer(GsBuffer *buffer);
void gs_opengl_internal_unbind_buffer(GsBufferType type);
void gs_opengl_internal_bind_program(GsProgram *program);
void gs_opengl_internal_unbind_program();
void gs_opengl_internal_bind_layout(GsVtxLayout *layout);
void gs_opengl_internal_unbind_layout();
void gs_opengl_internal_bind_state();

// buffer
void gs_opengl_create_buffer(GsBuffer *buffer);
void gs_opengl_set_buffer_data(GsBuffer *buffer, void *data, int size);
void gs_opengl_set_buffer_partial_data(GsBuffer *buffer, void *data, int size, int offset);
void gs_opengl_destroy_buffer(GsBuffer *buffer);

// type conversions
int gs_opengl_get_buffer_type(GsBufferType type);
int gs_opengl_get_buffer_intent(GsBufferIntent intent);
int gs_opengl_get_attrib_type(GsVtxAttribType type);

// platform
void *gs_opengl_getproc(const char *name);

#ifdef __cplusplus
}
#endif

#endif //GENESIS_GL460_H
