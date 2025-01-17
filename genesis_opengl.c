#include "genesis_opengl.h"
#include "genesis.h"
#include <stdio.h>
#include <stdlib.h>

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

#if defined(__ANDROID__)
    #define GS_OPENGL_PLATFORM_IMPL
    #define GS_OPENGL_V200ES
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

GsBuffer* bound_vertex_buffer = NULL;
GsBuffer* bound_index_buffer = NULL;
GsProgram* bound_program = NULL;
GsVtxLayout* bound_layout = NULL;
GsTexture** bound_textures = NULL;
GsBuffer* requested_vertex_buffer = NULL;
GsBuffer* requested_index_buffer = NULL;
GsProgram* requested_program = NULL;
GsVtxLayout* requested_layout = NULL;
GsTexture** requested_textures = NULL;

GsBackend *gs_opengl_create() {
    GsBackend *backend = GS_ALLOC(GsBackend);

    // core
    backend->type = GS_BACKEND_OPENGL;
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

    // uniforms
    backend->get_uniform_location = gs_opengl_get_uniform_location;

    // layout
    backend->create_layout_handle = gs_opengl_create_layout;
    backend->destroy_layout_handle = gs_opengl_destroy_layout;

    // texture
    backend->create_texture_handle = gs_opengl_create_texture;
    backend->set_texture_data = gs_opengl_set_texture_data;
    backend->generate_mipmaps = gs_opengl_generate_mipmaps;
    backend->destroy_texture_handle = gs_opengl_destroy_texture;

    // init state
    bound_textures = GS_ALLOC_MULTIPLE(GsTexture*, GS_MAX_TEXTURE_SLOTS);
    requested_textures = GS_ALLOC_MULTIPLE(GsTexture*, GS_MAX_TEXTURE_SLOTS);

    for (int i = 0; i < GS_MAX_TEXTURE_SLOTS; i++) {
        bound_textures[i] = NULL;
        requested_textures[i] = NULL;
    }

    return backend;
}

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
    if (requested_vertex_buffer != bound_vertex_buffer) {
        if (requested_vertex_buffer != NULL) {
            GsOpenGLBufferHandle *handle = (GsOpenGLBufferHandle*)requested_vertex_buffer->handle;
            glBindVertexArray(handle->vaoHandle);

            bound_index_buffer = handle->lastIndexBuffer;
            bound_layout = handle->lastLayout;
        } else {
            glBindVertexArray(0);
        }

        bound_vertex_buffer = requested_vertex_buffer;
    }

    if (requested_index_buffer != bound_index_buffer) {
        if (requested_index_buffer != NULL) {
            GsOpenGLBufferHandle *handle = (GsOpenGLBufferHandle*)requested_index_buffer->handle;
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle->handle);
        } else {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }

        if (bound_vertex_buffer != NULL) {
            GsOpenGLBufferHandle *vertexHandle = (GsOpenGLBufferHandle*)bound_vertex_buffer->handle;
            vertexHandle->lastIndexBuffer = requested_index_buffer;
        }

        bound_index_buffer = requested_index_buffer;
    }

    if (requested_program != bound_program) {
        GS_ASSERT(requested_program != NULL); // must have a program bound

        glUseProgram(*(GLuint*)requested_program->handle);
        bound_program = requested_program;
    }

    if (requested_layout != bound_layout) {
        gs_opengl_internal_bind_layout_state();
    }

    for (int i = 0; i < GS_MAX_TEXTURE_SLOTS; i++) {
        if (requested_textures[i] != bound_textures[i]) {
            if (requested_textures[i] != NULL) {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, *(GLuint*)requested_textures[i]->handle);
            } else {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            bound_textures[i] = requested_textures[i];
        }
    }
}

void gs_opengl_cmd_set_uniform_int(const GsCommandListItem item) {
    const GsUniformIntCommand *cmd = (GsUniformIntCommand *) item.data;

    gs_opengl_internal_bind_state();
    glUniform1i(cmd->location, cmd->value);
}

void gs_opengl_cmd_set_uniform_float(const GsCommandListItem item) {
    const GsUniformFloatCommand *cmd = (GsUniformFloatCommand *) item.data;

    gs_opengl_internal_bind_state();
    glUniform1f(cmd->location, cmd->value);
}

void gs_opengl_cmd_set_uniform_vec2(const GsCommandListItem item) {
    const GsUniformVec2Command *cmd = (GsUniformVec2Command *) item.data;

    gs_opengl_internal_bind_state();
    glUniform2f(cmd->location, cmd->x, cmd->y);
}

void gs_opengl_cmd_set_uniform_vec3(const GsCommandListItem item) {
    const GsUniformVec3Command *cmd = (GsUniformVec3Command *) item.data;

    gs_opengl_internal_bind_state();
    glUniform3f(cmd->location, cmd->x, cmd->y, cmd->z);
}

void gs_opengl_cmd_set_uniform_vec4(const GsCommandListItem item) {
    const GsUniformVec4Command *cmd = (GsUniformVec4Command *) item.data;

    gs_opengl_internal_bind_state();
    glUniform4f(cmd->location, cmd->x, cmd->y, cmd->z, cmd->w);
}

void gs_opengl_cmd_set_uniform_mat4(const GsCommandListItem item) {
    const GsUniformMat4Command *cmd = (GsUniformMat4Command *) item.data;

    float mat[16] = {
        cmd->m00, cmd->m01, cmd->m02, cmd->m03,
        cmd->m10, cmd->m11, cmd->m12, cmd->m13,
        cmd->m20, cmd->m21, cmd->m22, cmd->m23,
        cmd->m30, cmd->m31, cmd->m32, cmd->m33
    };

    gs_opengl_internal_bind_state();
    glUniformMatrix4fv(cmd->location, 1, GL_FALSE, mat);
}

void gs_opengl_internal_bind_layout_state() {
    if (bound_vertex_buffer == NULL) {
        bound_layout = requested_layout;
        return;
    }

    GsOpenGLBufferHandle *handle = (GsOpenGLBufferHandle*)bound_vertex_buffer->handle;
    handle->lastLayout = requested_layout;

    glBindBuffer(GL_ARRAY_BUFFER, handle->handle);

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

void gs_opengl_internal_bind_texture(GsTexture *texture, int slot) {
    GS_ASSERT(texture != NULL);
    GS_ASSERT(slot >= 0 && slot < GS_MAX_TEXTURE_SLOTS);

    requested_textures[slot] = texture;
}

void gs_opengl_internal_unbind_texture(int slot) {
    GS_ASSERT(slot >= 0 && slot < GS_MAX_TEXTURE_SLOTS);

    requested_textures[slot] = NULL;
}

void gs_opengl_create_buffer(GsBuffer *buffer) {
    GS_ASSERT(buffer != NULL);

    GLuint vbo;
    glGenBuffers(1, &vbo);

    GsOpenGLBufferHandle *handle = GS_ALLOC(GsOpenGLBufferHandle);
    handle->handle = vbo;
    handle->vaoHandle = 0xFF; // invalid
    handle->lastLayout = NULL;
    handle->lastIndexBuffer = NULL;

    if (buffer->type == GS_BUFFER_TYPE_VERTEX) {
        GLuint vao;
        glGenVertexArrays(1, &vao);
        handle->vaoHandle = vao;

        gs_opengl_internal_unbind_layout();
        gs_opengl_internal_unbind_buffer(GS_BUFFER_TYPE_VERTEX);
        gs_opengl_internal_unbind_buffer(GS_BUFFER_TYPE_INDEX);
        gs_opengl_internal_bind_state();

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        gs_opengl_internal_bind_buffer(buffer);
    }

    buffer->handle = (void*)handle;
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

    GsOpenGLBufferHandle *handle = (GsOpenGLBufferHandle*)buffer->handle;
    glDeleteBuffers(1, &handle->handle);

    if (buffer->type == GS_BUFFER_TYPE_VERTEX) {
        glDeleteVertexArrays(1, &handle->vaoHandle);
    }

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

    // caps
    backend->capabilities = 0;
    backend->capabilities |= GS_CAPABILITY_RENDERER;

    return GS_TRUE;
}

void gs_opengl_shutdown(GsBackend *backend) {
    GS_ASSERT(backend != NULL);
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

void gs_opengl_cmd_use_texture(const GsCommandListItem item) {
    const GsTextureCommand *cmd = (GsTextureCommand *) item.data;
    gs_opengl_internal_bind_texture(cmd->texture, cmd->slot);
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
            case GS_COMMAND_USE_TEXTURE:
                gs_opengl_cmd_use_texture(item);
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
            case GS_COMMAND_SET_UNIFORM_INT:
                gs_opengl_cmd_set_uniform_int(item);
                break;
            case GS_COMMAND_SET_UNIFORM_FLOAT:
                gs_opengl_cmd_set_uniform_float(item);
                break;
            case GS_COMMAND_SET_UNIFORM_VEC2:
                gs_opengl_cmd_set_uniform_vec2(item);
                break;
            case GS_COMMAND_SET_UNIFORM_VEC3:
                gs_opengl_cmd_set_uniform_vec3(item);
                break;
            case GS_COMMAND_SET_UNIFORM_VEC4:
                gs_opengl_cmd_set_uniform_vec4(item);
                break;
            case GS_COMMAND_SET_UNIFORM_MAT4:
                gs_opengl_cmd_set_uniform_mat4(item);
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

int gs_opengl_get_face_type(GsCubemapFace face) {
    switch (face) {
        case GS_CUBEMAP_FACE_UP: return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
        case GS_CUBEMAP_FACE_DOWN: return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
        case GS_CUBEMAP_FACE_LEFT: return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
        case GS_CUBEMAP_FACE_RIGHT: return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
        case GS_CUBEMAP_FACE_FRONT: return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
        case GS_CUBEMAP_FACE_BACK: return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
        default: return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
    }
}

int gs_opengl_get_texture_type(GsTextureType type) {
    switch (type) {
        case GS_TEXTURE_TYPE_2D: return GL_TEXTURE_2D;
        case GS_TEXTURE_TYPE_CUBEMAP: return GL_TEXTURE_CUBE_MAP;
        default: return GL_TEXTURE_2D;
    }
}

int gs_opengl_get_texture_format(GsTextureFormat format) {
    switch (format) {
        case GS_TEXTURE_FORMAT_RGBA8: return GL_RGBA;
        case GS_TEXTURE_FORMAT_RGB8: return GL_RGB;
        case GS_TEXTURE_FORMAT_RGB16F: return GL_RGB16F;
        case GS_TEXTURE_FORMAT_RGBA16F: return GL_RGBA16F;
        case GS_TEXTURE_FORMAT_DEPTH24_STENCIL8: return GL_DEPTH24_STENCIL8;
        case GS_TEXTURE_FORMAT_DEPTH32F: return GL_DEPTH_COMPONENT32F;
        default: return GL_RGBA;
    }
}

int gs_opengl_get_texture_wrap(GsTextureWrap wrap) {
    switch (wrap) {
        case GS_TEXTURE_WRAP_REPEAT: return GL_REPEAT;
        case GS_TEXTURE_WRAP_CLAMP: return GL_CLAMP_TO_EDGE;
        case GS_TEXTURE_WRAP_MIRROR: return GL_MIRRORED_REPEAT;
        default: return GL_REPEAT;
    }
}

int gs_opengl_get_texture_filter(GsTextureFilter filter) {
    switch (filter) {
        case GS_TEXTURE_FILTER_NEAREST: return GL_NEAREST;
        case GS_TEXTURE_FILTER_LINEAR: return GL_LINEAR;
        case GS_TEXTURE_FILTER_MIPMAP_NEAREST: return GL_NEAREST_MIPMAP_NEAREST;
        case GS_TEXTURE_FILTER_MIPMAP_LINEAR: return GL_LINEAR_MIPMAP_LINEAR;
        default: return GL_NEAREST;
    }
}

void gs_opengl_create_texture(GsTexture *texture) {
    GS_ASSERT(texture != NULL);

    GLuint* handle = GS_ALLOC(GLuint);
    glGenTextures(1, handle);

    texture->handle = handle;
}

void gs_opengl_set_texture_data(GsTexture *texture, GsCubemapFace face, void *data) {
    GS_ASSERT(texture != NULL);
    GS_ASSERT(data != NULL);

    gs_opengl_internal_bind_texture(texture, 0);
    gs_opengl_internal_bind_state();

    glActiveTexture(GL_TEXTURE0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gs_opengl_get_texture_wrap(texture->wrap_s));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gs_opengl_get_texture_wrap(texture->wrap_t));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gs_opengl_get_texture_filter(texture->min));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gs_opengl_get_texture_filter(texture->mag));

    switch (texture->type) {
        case GS_TEXTURE_TYPE_2D:
            glTexImage2D(GL_TEXTURE_2D, 0, gs_opengl_get_texture_format(texture->format), texture->width, texture->height, 0, gs_opengl_get_texture_format(texture->format), GL_UNSIGNED_BYTE, data);
        case GS_TEXTURE_TYPE_CUBEMAP:
            glTexImage2D(gs_opengl_get_face_type(face), 0, gs_opengl_get_texture_format(texture->format), texture->width, texture->height, 0, gs_opengl_get_texture_format(texture->format), GL_UNSIGNED_BYTE, data);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, gs_opengl_get_texture_wrap(texture->wrap_r));
    }
}

GsUniformLocation gs_opengl_get_uniform_location(GsProgram *program, const char *name) {
    GS_ASSERT(program != NULL);
    GS_ASSERT(name != NULL);

    GLuint loc = glGetUniformLocation(*(GLuint*)program->handle, name);

    return loc; // -1 is an invalid location
}

void gs_opengl_generate_mipmaps(GsTexture *texture) {
    GS_ASSERT(texture != NULL);

    gs_opengl_internal_bind_texture(texture, 0);
    gs_opengl_internal_bind_state();

    glActiveTexture(GL_TEXTURE0);
    glGenerateMipmap(GL_TEXTURE_2D);
}

void gs_opengl_destroy_texture(GsTexture *texture) {
    GS_ASSERT(texture != NULL);

    glDeleteTextures(1, (GLuint*)texture->handle);

    GS_FREE(texture->handle);
    texture->handle = NULL;
}

void gs_opengl_create_layout(GsVtxLayout *layout) {
    // NOTE: not relevant for the GL460 backend.
}

void gs_opengl_destroy_layout(GsVtxLayout *layout) {
    // NOTE: not relevant for the GL460 backend.
}


