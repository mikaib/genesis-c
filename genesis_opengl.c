#include "genesis_opengl.h"
#include "genesis.h"
#include <stdio.h>
#include <stdlib.h>

#define GS_OPENGL_BIND_BUFFER(target, requested, bound) \
    if (requested != bound) { \
        if (requested != NULL) { \
            glBindBuffer(target, *(GLuint*)requested->handle); \
        } else { \
            glBindBuffer(target, 0); \
        } \
        bound = requested; \
    }

#if defined(_WIN32)
    #include <windows.h>
    #define GS_OPENGL_PLATFORM_IMPL
    #define GS_OPENGL_USE_GLAD
    #define GS_OPENGL_V460
    // #define GS_OPENGL_DEBUG
    void *gs_opengl_getproc(const char *name) {
        void *p = (void *) wglGetProcAddress(name);
        if (p == 0 ||
            (p == (void *) 0x1) || (p == (void *) 0x2) || (p == (void *) 0x3) ||
            (p == (void *) -1)) {
            const HMODULE module = LoadLibraryA("opengl32.dll");
            p = (void *) GetProcAddress(module, name);
        }

        return p;
    }
#endif

#if defined(__EMSCRIPTEN__)
    #define GS_OPENGL_PLATFORM_IMPL
    #define GS_OPENGL_V200ES
    #include <emscripten.h>
    #include <GLES2/gl2.h>
    #include <EGL/egl.h>
#endif

#ifdef GS_OPENGL_USE_GLAD
#include "glad/include/glad/gl.h"
#endif

#ifndef GS_OPENGL_PLATFORM_IMPL
void *gs_opengl_getproc(const char *name) {
        printf("gs_opengl_getproc not implemented for platform.\n");
        return NULL;
    }
#endif

GsBackend *gs_opengl_create() {
    GsBackend *backend = GS_ALLOC(GsBackend);

    // core
    backend->type = GS_BACKEND_GL460;
    backend->init = gs_opengl_init;
    backend->shutdown = gs_opengl_shutdown;
    backend->submit = gs_opengl_submit;

    // buffer
    backend->create_buffer_handle = gs_opengl_create_buffer;
    backend->set_buffer_data = gs_opengl_set_buffer_data;
    backend->set_buffer_partial_data = gs_opengl_set_buffer_partial_data;
    backend->destroy_buffer_handle = gs_opengl_destroy_buffer;

    // shader
    backend->create_shader_handle = gs_opengl_create_shader;
    backend->destroy_shader_handle = gs_opengl_destroy_shader;

    // program
    backend->create_program_handle = gs_opengl_create_program;
    backend->destroy_program_handle = gs_opengl_destroy_program;

    // layout
    backend->create_layout_handle = gs_opengl_create_layout;
    backend->destroy_layout_handle = gs_opengl_destroy_layout;

    return backend;
}

GLuint* bound_vertex_array = NULL;
GsBuffer* bound_vertex_buffer = NULL;
GsBuffer* bound_index_buffer = NULL;
GsProgram* bound_program = NULL;
GsVtxLayout* bound_layout = NULL;
GsBuffer* requested_vertex_buffer = NULL;
GsBuffer* requested_index_buffer = NULL;
GsProgram* requested_program = NULL;
GsVtxLayout* requested_layout = NULL;

int gs_opengl_get_buffer_type(GsBufferType type) {
    switch (type) {
        case GS_BUFFER_TYPE_VERTEX: return GL_ARRAY_BUFFER;
        case GS_BUFFER_TYPE_INDEX: return GL_ELEMENT_ARRAY_BUFFER;
    }

    return 0;
}

int gs_opengl_get_buffer_intent(GsBufferIntent intent) {
    #ifdef GS_OPENGL_V460
    switch (intent) {
        case GS_BUFFER_INTENT_DRAW_STATIC: return GL_STATIC_DRAW;
        case GS_BUFFER_INTENT_DRAW_DYNAMIC: return GL_DYNAMIC_DRAW;
        case GS_BUFFER_INTENT_DRAW_STREAM: return GL_STREAM_DRAW;
        case GS_BUFFER_INTENT_READ_STATIC: return GL_STATIC_READ;
        case GS_BUFFER_INTENT_READ_DYNAMIC: return GL_DYNAMIC_READ;
        case GS_BUFFER_INTENT_READ_STREAM: return GL_STREAM_READ;
        case GS_BUFFER_INTENT_COPY_STATIC: return GL_STATIC_COPY;
        case GS_BUFFER_INTENT_COPY_DYNAMIC: return GL_DYNAMIC_COPY;
        case GS_BUFFER_INTENT_COPY_STREAM: return GL_STREAM_COPY;
    }
    #endif

    #ifdef GS_OPENGL_V200ES
    switch (intent) {
        case GS_BUFFER_INTENT_DRAW_STATIC: return GL_STATIC_DRAW;
        case GS_BUFFER_INTENT_DRAW_DYNAMIC: return GL_DYNAMIC_DRAW;
        case GS_BUFFER_INTENT_DRAW_STREAM: return GL_STREAM_DRAW;
        case GS_BUFFER_INTENT_READ_STATIC: return GL_STATIC_DRAW;
        case GS_BUFFER_INTENT_READ_DYNAMIC: return GL_DYNAMIC_DRAW;
        case GS_BUFFER_INTENT_READ_STREAM: return GL_STREAM_DRAW;
        case GS_BUFFER_INTENT_COPY_STATIC: return GL_STATIC_DRAW;
        case GS_BUFFER_INTENT_COPY_DYNAMIC: return GL_DYNAMIC_DRAW;
        case GS_BUFFER_INTENT_COPY_STREAM: return GL_STREAM_DRAW;
    }
    #endif

    return 0;
}

void gs_opengl_internal_bind_state() {
    GS_OPENGL_BIND_BUFFER(GL_ARRAY_BUFFER, requested_vertex_buffer, bound_vertex_buffer);
    GS_OPENGL_BIND_BUFFER(GL_ELEMENT_ARRAY_BUFFER, requested_index_buffer, bound_index_buffer);

    if (requested_program != bound_program) {
        GS_ASSERT(requested_program != NULL); // must have a program bound

        glUseProgram(*(GLuint*)requested_program->handle);
        bound_program = requested_program;
    }

    if (requested_layout != bound_layout) {
        gs_opengl_internal_bind_layout_state();
    }
}

void gs_opengl_internal_bind_layout_state() {
    if (requested_layout != NULL) {
        for (int i = 0; i < requested_layout->count; i++) {
            const GsVtxLayoutItem item = requested_layout->items[i];
            glVertexAttribPointer(item.index, item.components, gs_opengl_get_attrib_type(item.type), GL_FALSE, requested_layout->stride, (const void*)(uintptr_t)item.offset);
            glEnableVertexAttribArray(item.index);
        }

        if (bound_layout != NULL && requested_layout->count < bound_layout->count) {
            for (int i = requested_layout->count; i < bound_layout->count; i++) {
                glDisableVertexAttribArray(bound_layout->items[i].index);
            }
        }

        bound_layout = requested_layout;
    } else {
        if (bound_layout != NULL) {
            for (int i = 0; i < bound_layout->count; i++) {
                glDisableVertexAttribArray(bound_layout->items[i].index);
            }
        }

        bound_layout = NULL;
    }
}

void gs_opengl_internal_bind_buffer(GsBuffer *buffer) {
    GS_ASSERT(buffer != NULL);

    switch (buffer->type) {
        case GS_BUFFER_TYPE_VERTEX:
           requested_vertex_buffer = buffer;
        break;
        case GS_BUFFER_TYPE_INDEX:
            requested_index_buffer = buffer;
        break;
    }
}

void gs_opengl_internal_bind_layout(GsVtxLayout *layout) {
    GS_ASSERT(layout != NULL);
    requested_layout = layout;
}

void gs_opengl_internal_unbind_layout() {
    requested_layout = NULL;
}

void gs_opengl_internal_bind_program(GsProgram *program) {
    GS_ASSERT(program != NULL);
    requested_program = program;
}

void gs_opengl_internal_unbind_program() {
    requested_program = NULL;
}

void gs_opengl_internal_unbind_buffer(GsBufferType type) {
    switch (type) {
        case GS_BUFFER_TYPE_VERTEX:
            requested_vertex_buffer = NULL;
        break;
        case GS_BUFFER_TYPE_INDEX:
            requested_index_buffer = NULL;
        break;
    }
}

void gs_opengl_create_buffer(GsBuffer *buffer) {
    GS_ASSERT(buffer != NULL);

    GLuint* buffers = GS_ALLOC_MULTIPLE(GLuint, 1);
    glGenBuffers(1, buffers);

    buffer->handle = &buffers[0];
}

void gs_opengl_set_buffer_data(GsBuffer *buffer, void *data, int size) {
    GS_ASSERT(buffer != NULL);
    GS_ASSERT(data != NULL);
    GS_ASSERT(size > 0);

    gs_opengl_internal_bind_buffer(buffer);
    gs_opengl_internal_bind_state();
    glBufferData(gs_opengl_get_buffer_type(buffer->type), size, data, gs_opengl_get_buffer_intent(buffer->intent));
}

void gs_opengl_set_buffer_partial_data(GsBuffer *buffer, void *data, int size, int offset) {
    GS_ASSERT(buffer != NULL);
    GS_ASSERT(data != NULL);
    GS_ASSERT(size > 0);
    GS_ASSERT(offset >= 0);

    gs_opengl_internal_bind_buffer(buffer);
    gs_opengl_internal_bind_state();
    glBufferSubData(gs_opengl_get_buffer_type(buffer->type), offset, size, data);
}

void gs_opengl_destroy_buffer(GsBuffer *buffer) {
    GS_ASSERT(buffer != NULL);

    if (bound_vertex_buffer == buffer) {
        gs_opengl_internal_unbind_buffer(GS_BUFFER_TYPE_VERTEX);
        gs_opengl_internal_bind_state();
    }

    if (bound_index_buffer == buffer) {
        gs_opengl_internal_unbind_buffer(GS_BUFFER_TYPE_INDEX);
        gs_opengl_internal_bind_state();
    }

    glDeleteBuffers(1, (GLuint*)buffer->handle);

    GS_FREE(buffer->handle);
    buffer->handle = NULL;
}

#ifdef GS_OPENGL_DEBUG
void gs_opengl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
        return;
    }

    printf("OpenGL Debug: %s\n", message);
}
#endif

GS_BOOL gs_opengl_init(GsBackend *backend, GsConfig *config) {
    // GS_ASSERT(config->window != NULL);
    GS_ASSERT(backend != NULL);

    #ifdef GS_OPENGL_USE_GLAD
        const int res = gladLoadGL((GLADloadfunc) gs_opengl_getproc);

        if (!res) {
            printf("Failed to load OpenGL\n");
            return GS_FALSE;
        }
    #endif

    #ifdef GS_OPENGL_DEBUG
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback((GLDEBUGPROC) gs_opengl_debug_callback, NULL);
    #endif

    #ifdef GS_OPENGL_V460
        GLuint* vao = GS_ALLOC_MULTIPLE(GLuint, 1);
        glGenVertexArrays(1, vao);
        glBindVertexArray(*vao);
        bound_vertex_array = vao;
    #endif

    return GS_TRUE;
}

void gs_opengl_shutdown(GsBackend *backend) {
    GS_ASSERT(backend != NULL);

    #ifdef GS_OPENGL_V460
    glDeleteVertexArrays(1, bound_vertex_array);
    GS_FREE(bound_vertex_array);
    #endif
}

void gs_opengl_cmd_clear(const GsCommandListItem item) {
    const GsClearCommand *cmd = (GsClearCommand *) item.data;
    int flags = 0;

    if (cmd->flags & GS_CLEAR_COLOR) {
        flags |= GL_COLOR_BUFFER_BIT;
    }

    if (cmd->flags & GS_CLEAR_DEPTH) {
        flags |= GL_DEPTH_BUFFER_BIT;
    }

    if (cmd->flags & GS_CLEAR_STENCIL) {
        flags |= GL_STENCIL_BUFFER_BIT;
    }

    glClearColor(cmd->r, cmd->g, cmd->b, cmd->a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void gs_opengl_cmd_set_viewport(const GsCommandListItem item) {
    const GsViewportCommand *cmd = (GsViewportCommand *) item.data;
    glViewport(cmd->x, cmd->y, cmd->width, cmd->height);
}

void gs_opengl_cmd_use_pipeline(const GsCommandListItem item) {
    const GsPipelineCommand *cmd = (GsPipelineCommand *) item.data;
    const GsPipeline *pipeline = cmd->pipeline;

    gs_opengl_internal_bind_program(pipeline->program);
    gs_opengl_internal_bind_layout(pipeline->layout);
}

void gs_opengl_cmd_use_buffer(const GsCommandListItem item) {
    const GsUseBufferCommand *cmd = (GsUseBufferCommand *) item.data;
    gs_opengl_internal_bind_buffer(cmd->buffer);
}

void gs_opengl_cmd_draw_arrays(const GsCommandListItem item) {
    const GsDrawArraysCommand *cmd = (GsDrawArraysCommand *) item.data;

    gs_opengl_internal_bind_state();
    glDrawArrays(GL_TRIANGLES, cmd->start, cmd->count);
}

void gs_opengl_cmd_draw_indexed(const GsCommandListItem item) {
    const GsDrawIndexedCommand *cmd = (GsDrawIndexedCommand *) item.data;

    gs_opengl_internal_bind_state();

    glDrawElements(GL_TRIANGLES, cmd->count, GL_UNSIGNED_INT, 0);
}

void gs_opengl_cmd_set_scissor(const GsCommandListItem item) {
    const GsScissorCommand *cmd = (GsScissorCommand *) item.data;

    if (cmd->enable == GS_TRUE) {
        glScissor(cmd->x, cmd->y, cmd->width, cmd->height);
        glEnable(GL_SCISSOR_TEST);
    } else {
        glDisable(GL_SCISSOR_TEST);
    }
}

void gs_opengl_submit(GsBackend *backend, GsCommandList *list) {
    for (int i = 0; i < list->count; i++) {
        const GsCommandListItem item = list->items[i];

        switch (item.type) {
            case GS_COMMAND_CLEAR:
                gs_opengl_cmd_clear(item);
                break;
            case GS_COMMAND_SET_VIEWPORT:
                gs_opengl_cmd_set_viewport(item);
                break;
            case GS_COMMAND_USE_PIPELINE:
                gs_opengl_cmd_use_pipeline(item);
                break;
            case GS_COMMAND_USE_BUFFER:
                gs_opengl_cmd_use_buffer(item);
                break;
            case GS_COMMAND_DRAW_ARRAYS:
                gs_opengl_cmd_draw_arrays(item);
                break;
            case GS_COMMAND_DRAW_INDEXED:
                gs_opengl_cmd_draw_indexed(item);
                break;
            case GS_COMMAND_SET_SCISSOR:
                gs_opengl_cmd_set_scissor(item);
                break;
            default:
                gs_handle_internal_command(item);
                break;
        }
    }
}

void gs_opengl_create_shader(GsShader *shader, const char *source) {
    GS_ASSERT(shader != NULL);

    GLuint* handle = GS_ALLOC(GLuint);
    *handle = glCreateShader(shader->type == GS_SHADER_TYPE_VERTEX ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);

    glShaderSource(*handle, 1, &source, NULL);
    glCompileShader(*handle);

    shader->handle = handle;
}

void gs_opengl_destroy_shader(GsShader *shader) {
    GS_ASSERT(shader != NULL);

    glDeleteShader(*(GLuint*)shader->handle);

    GS_FREE(shader->handle);
    shader->handle = NULL;
}

void gs_opengl_create_program(GsProgram *program) {
    GS_ASSERT(program != NULL);

    GLuint* handle = GS_ALLOC(GLuint);
    *handle = glCreateProgram();

    program->handle = handle;

    if (program->vertex != NULL) {
        glAttachShader(*(GLuint*)program->handle, *(GLuint*)program->vertex->handle);
    }

    if (program->fragment != NULL) {
        glAttachShader(*(GLuint*)program->handle, *(GLuint*)program->fragment->handle);
    }

    glLinkProgram(*(GLuint*)program->handle);
}

void gs_opengl_destroy_program(GsProgram *program) {
    GS_ASSERT(program != NULL);

    glDeleteProgram(*(GLuint*)program->handle);

    GS_FREE(program->handle);
    program->handle = NULL;
}

int gs_opengl_get_attrib_type(GsVtxAttribType type) {
    switch (type) {
        case GS_ATTRIB_TYPE_FLOAT: return GL_FLOAT;
        case GS_ATTRIB_TYPE_INT16: return GL_SHORT;
        case GS_ATTRIB_TYPE_UINT8: return GL_UNSIGNED_BYTE;
        default: return GL_FLOAT;
    }
}

void gs_opengl_create_layout(GsVtxLayout *layout) {
    // NOTE: not relevant for the GL460 backend.
}

void gs_opengl_destroy_layout(GsVtxLayout *layout) {
    // NOTE: not relevant for the GL460 backend.
}


