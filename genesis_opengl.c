#include "genesis_opengl.h"
#include "genesis.h"
#include <stdio.h>
#include <stdlib.h>

#if defined(__EMSCRIPTEN__)
    #define GS_OPENGL_PLATFORM_IMPL
    #include <emscripten.h>
    #include <EGL/egl.h>
    #if defined(GS_EMSCRIPTEN_GLES3)
        #define GS_OPENGL_V320ES
        #include <GLES3/gl3.h>
    #else
        #define GS_OPENGL_V200ES
        #include <GLES2/gl2.h>
    #endif
#endif

#if defined(_WIN32)
    #include <windows.h>
    #define GS_OPENGL_PLATFORM_IMPL
    #define GS_OPENGL_USE_GLAD
    #define GS_OPENGL_V460
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

#if defined(__ANDROID__)
    #include <EGL/egl.h>
    #define GS_OPENGL_PLATFORM_IMPL
    #if defined(GS_ANDROID_GLES2)
        #define GS_OPENGL_V200ES
        #include <GLES2/gl2.h>
    #else
        #define GS_OPENGL_V320ES
        #include <GLES3/gl3.h>
    #endif
#endif

#if defined(__linux__) && !defined(__ANDROID__)
    #define GS_OPENGL_PLATFORM_IMPL
    #define GS_OPENGL_USE_GLAD
    #define GS_OPENGL_V460

    #if defined(GS_WAYLAND)
        #include <wayland-egl.h>
        #include <EGL/egl.h>
        void *gs_opengl_getproc(const char *name) {
            return (void *) eglGetProcAddress(name);
        }
    #else
        #error "X11 is not supported yet"
    #endif
#endif

#ifdef GS_OPENGL_USE_GLAD
#include "glad/include/glad/gl.h"
#endif

#ifndef GS_OPENGL_PLATFORM_IMPL
void *gs_opengl_getproc(const char *name) {
    GS_LOG("gs_opengl_getproc not implemented for platform.\n");
    return NULL;
}
#endif

#define GS_OPENGL_PIPELINE_CAP(want, current, cap) \
    if (want != current) { \
        if (want) { \
            glEnable(cap); \
        } else { \
            glDisable(cap); \
        } \
    }\
    \
    current = want;

#define GS_OPENGL_GLES2_COPY_TEXTURE_FALLBACK(fboName, src, dst) \
    static GLuint fboName = 0; \
    if (fboName == 0) { \
        glGenFramebuffers(1, &fboName); \
    } \
    GLint previous_fbo = 0; \
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previous_fbo); \
    glBindFramebuffer(GL_FRAMEBUFFER, fboName); \
    glFramebufferTexture2D( \
        GL_FRAMEBUFFER, \
        GL_COLOR_ATTACHMENT0, \
        GL_TEXTURE_2D, \
        *(GLuint*)(src)->handle, \
        0 \
    ); \
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER); \
    GS_ASSERT(status == GL_FRAMEBUFFER_COMPLETE); \
    glBindTexture(GL_TEXTURE_2D, *(GLuint*)(dst)->handle); \
    glCopyTexSubImage2D( \
        GL_TEXTURE_2D, \
        0, \
        0, 0, \
        0, 0, \
        (src)->width, \
        (src)->height \
    ); \
    glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)previous_fbo); \

#if defined(GS_OPENGL_V460)
static const int gs_opengl_attrib_types[] = {
    [GS_ATTRIB_TYPE_FLOAT]  = GL_FLOAT,
    [GS_ATTRIB_TYPE_INT16]  = GL_SHORT,
    [GS_ATTRIB_TYPE_UINT8]  = GL_UNSIGNED_BYTE,
    [GS_ATTRIB_TYPE_UINT16] = GL_UNSIGNED_SHORT,
    [GS_ATTRIB_TYPE_UINT32] = GL_UNSIGNED_INT,
    [GS_ATTRIB_TYPE_INT32]  = GL_INT,
    [GS_ATTRIB_TYPE_INT8]   = GL_BYTE,
    [GS_ATTRIB_TYPE_DOUBLE] = GL_DOUBLE,
};
#endif

#if defined(GS_OPENGL_V200ES) || defined(GS_OPENGL_V320ES)
static const int gs_opengl_attrib_types[] = {
    [GS_ATTRIB_TYPE_FLOAT]  = GL_FLOAT,
    [GS_ATTRIB_TYPE_INT16]  = GL_SHORT,
    [GS_ATTRIB_TYPE_UINT8]  = GL_UNSIGNED_BYTE,
    [GS_ATTRIB_TYPE_UINT16] = GL_UNSIGNED_SHORT,
    [GS_ATTRIB_TYPE_UINT32] = GL_UNSIGNED_INT,
    [GS_ATTRIB_TYPE_INT32]  = GL_INT,
    [GS_ATTRIB_TYPE_INT8]   = GL_BYTE,
    [GS_ATTRIB_TYPE_DOUBLE] = -1,
};
#endif

static const int gs_opengl_face_types[] = {
    [GS_CUBEMAP_FACE_UP]    = GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
    [GS_CUBEMAP_FACE_DOWN]  = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
    [GS_CUBEMAP_FACE_LEFT]  = GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
    [GS_CUBEMAP_FACE_RIGHT] = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
    [GS_CUBEMAP_FACE_FRONT] = GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
    [GS_CUBEMAP_FACE_BACK]  = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};

static const int gs_opengl_texture_types[] = {
    [GS_TEXTURE_TYPE_2D]      = GL_TEXTURE_2D,
    [GS_TEXTURE_TYPE_CUBEMAP] = GL_TEXTURE_CUBE_MAP
};

#if defined(GS_OPENGL_V460) || defined(GS_OPENGL_V320ES)
static const int gs_opengl_texture_formats[] = {
    [GS_TEXTURE_FORMAT_RGBA8]         = GL_RGBA,
    [GS_TEXTURE_FORMAT_RGB8]          = GL_RGB,
    [GS_TEXTURE_FORMAT_RGB16F]        = GL_RGB16F,
    [GS_TEXTURE_FORMAT_RGBA16F]       = GL_RGBA16F,
    [GS_TEXTURE_FORMAT_DEPTH24_STENCIL8] = GL_DEPTH24_STENCIL8,
    [GS_TEXTURE_FORMAT_DEPTH32F]      = GL_DEPTH_COMPONENT32F
};
#endif

#if defined(GS_OPENGL_V200ES)
static const int gs_opengl_texture_formats[] = {
    [GS_TEXTURE_FORMAT_RGBA8]         = GL_RGBA,
    [GS_TEXTURE_FORMAT_RGB8]          = GL_RGB,
    [GS_TEXTURE_FORMAT_RGB16F]        = -1,
    [GS_TEXTURE_FORMAT_RGBA16F]       = -1,
    [GS_TEXTURE_FORMAT_DEPTH24_STENCIL8] = -1,
    [GS_TEXTURE_FORMAT_DEPTH32F]      = -1,
};
#endif

static const int gs_opengl_texture_wraps[] = {
    [GS_TEXTURE_WRAP_REPEAT] = GL_REPEAT,
    [GS_TEXTURE_WRAP_CLAMP]  = GL_CLAMP_TO_EDGE,
    [GS_TEXTURE_WRAP_MIRROR] = GL_MIRRORED_REPEAT
};

static const int gs_opengl_texture_filters[] = {
    [GS_TEXTURE_FILTER_NEAREST]         = GL_NEAREST,
    [GS_TEXTURE_FILTER_LINEAR]          = GL_LINEAR,
    [GS_TEXTURE_FILTER_MIPMAP_NEAREST]  = GL_NEAREST_MIPMAP_NEAREST,
    [GS_TEXTURE_FILTER_MIPMAP_LINEAR]   = GL_LINEAR_MIPMAP_LINEAR
};

static const int gs_opengl_buffer_types[] = {
    [GS_BUFFER_TYPE_VERTEX] = GL_ARRAY_BUFFER,
    [GS_BUFFER_TYPE_INDEX]  = GL_ELEMENT_ARRAY_BUFFER
};

#if defined(GS_OPENGL_V460) || defined(GS_OPENGL_V320ES)
static const int gs_opengl_buffer_intents[] = {
    [GS_BUFFER_INTENT_DRAW_STATIC]  = GL_STATIC_DRAW,
    [GS_BUFFER_INTENT_DRAW_DYNAMIC] = GL_DYNAMIC_DRAW,
    [GS_BUFFER_INTENT_DRAW_STREAM]  = GL_STREAM_DRAW,
    [GS_BUFFER_INTENT_READ_STATIC]  = GL_STATIC_READ,
    [GS_BUFFER_INTENT_READ_DYNAMIC] = GL_DYNAMIC_READ,
    [GS_BUFFER_INTENT_READ_STREAM]  = GL_STREAM_READ,
    [GS_BUFFER_INTENT_COPY_STATIC]  = GL_STATIC_COPY,
    [GS_BUFFER_INTENT_COPY_DYNAMIC] = GL_DYNAMIC_COPY,
    [GS_BUFFER_INTENT_COPY_STREAM]  = GL_STREAM_COPY
};
#endif

#if defined(GS_OPENGL_V200ES)
static const int gs_opengl_buffer_intents[] = {
    [GS_BUFFER_INTENT_DRAW_STATIC]  = GL_STATIC_DRAW,
    [GS_BUFFER_INTENT_DRAW_DYNAMIC] = GL_DYNAMIC_DRAW,
    [GS_BUFFER_INTENT_DRAW_STREAM]  = GL_STREAM_DRAW,
    [GS_BUFFER_INTENT_READ_STATIC]  = GL_STATIC_DRAW,
    [GS_BUFFER_INTENT_READ_DYNAMIC] = GL_DYNAMIC_DRAW,
    [GS_BUFFER_INTENT_READ_STREAM]  = GL_STREAM_DRAW,
    [GS_BUFFER_INTENT_COPY_STATIC]  = GL_STATIC_DRAW,
    [GS_BUFFER_INTENT_COPY_DYNAMIC] = GL_DYNAMIC_DRAW,
    [GS_BUFFER_INTENT_COPY_STREAM]  = GL_STREAM_DRAW
};
#endif

static const int gs_opengl_blend_factors[] = {
    [GS_BLEND_FACTOR_ZERO]                 = GL_ZERO,
    [GS_BLEND_FACTOR_ONE]                  = GL_ONE,
    [GS_BLEND_FACTOR_SRC_COLOR]            = GL_SRC_COLOR,
    [GS_BLEND_FACTOR_ONE_MINUS_SRC_COLOR]  = GL_ONE_MINUS_SRC_COLOR,
    [GS_BLEND_FACTOR_DST_COLOR]            = GL_DST_COLOR,
    [GS_BLEND_FACTOR_ONE_MINUS_DST_COLOR]  = GL_ONE_MINUS_DST_COLOR,
    [GS_BLEND_FACTOR_SRC_ALPHA]            = GL_SRC_ALPHA,
    [GS_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA]  = GL_ONE_MINUS_SRC_ALPHA,
    [GS_BLEND_FACTOR_DST_ALPHA]            = GL_DST_ALPHA,
    [GS_BLEND_FACTOR_ONE_MINUS_DST_ALPHA]  = GL_ONE_MINUS_DST_ALPHA,
    [GS_BLEND_FACTOR_CONSTANT_COLOR]       = GL_CONSTANT_COLOR,
    [GS_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR] = GL_ONE_MINUS_CONSTANT_COLOR,
    [GS_BLEND_FACTOR_CONSTANT_ALPHA]       = GL_CONSTANT_ALPHA,
    [GS_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA] = GL_ONE_MINUS_CONSTANT_ALPHA,
    [GS_BLEND_FACTOR_SRC_ALPHA_SATURATE]   = GL_SRC_ALPHA_SATURATE
};

#if defined(GS_OPENGL_V460) || defined(GS_OPENGL_V320ES)
static const int gs_opengl_blend_ops[] = {
    [GS_BLEND_OP_ADD]              = GL_FUNC_ADD,
    [GS_BLEND_OP_SUBTRACT]         = GL_FUNC_SUBTRACT,
    [GS_BLEND_OP_REVERSE_SUBTRACT] = GL_FUNC_REVERSE_SUBTRACT,
    [GS_BLEND_OP_MIN]              = GL_MIN,
    [GS_BLEND_OP_MAX]              = GL_MAX
};
#endif

#if defined(GS_OPENGL_V200ES)
static const int gs_opengl_blend_ops[] = {
    [GS_BLEND_OP_ADD]              = GL_FUNC_ADD,
    [GS_BLEND_OP_SUBTRACT]         = GL_FUNC_SUBTRACT,
    [GS_BLEND_OP_REVERSE_SUBTRACT] = GL_FUNC_REVERSE_SUBTRACT,
    [GS_BLEND_OP_MIN]              = -1,
    [GS_BLEND_OP_MAX]              = -1
};
#endif

static const int gs_opengl_depth_func[] = {
    [GS_DEPTH_FUNC_NEVER]         = GL_NEVER,
    [GS_DEPTH_FUNC_LESS]         = GL_LESS,
    [GS_DEPTH_FUNC_EQUAL]        = GL_EQUAL,
    [GS_DEPTH_FUNC_LESS_EQUAL]   = GL_LEQUAL,
    [GS_DEPTH_FUNC_GREATER]      = GL_GREATER,
    [GS_DEPTH_FUNC_NOT_EQUAL]    = GL_NOTEQUAL,
    [GS_DEPTH_FUNC_GREATER_EQUAL] = GL_GEQUAL,
    [GS_DEPTH_FUNC_ALWAYS]       = GL_ALWAYS
};

static const GsCommandHandler gs_opengl_commands [] = {
    [GS_COMMAND_CLEAR]                = gs_opengl_cmd_clear,
    [GS_COMMAND_SET_VIEWPORT]         = gs_opengl_cmd_set_viewport,
    [GS_COMMAND_USE_PIPELINE]         = gs_opengl_cmd_use_pipeline,
    [GS_COMMAND_USE_BUFFER]           = gs_opengl_cmd_use_buffer,
    [GS_COMMAND_USE_TEXTURE]          = gs_opengl_cmd_use_texture,
    [GS_COMMAND_BEGIN_PASS]           = gs_opengl_cmd_begin_render_pass,
    [GS_COMMAND_END_PASS]             = gs_opengl_cmd_end_render_pass,
    [GS_COMMAND_DRAW_ARRAYS]          = gs_opengl_cmd_draw_arrays,
    [GS_COMMAND_DRAW_INDEXED]         = gs_opengl_cmd_draw_indexed,
    [GS_COMMAND_SET_SCISSOR]          = gs_opengl_cmd_set_scissor,
    [GS_COMMAND_SET_UNIFORM_INT]      = gs_opengl_cmd_set_uniform_int,
    [GS_COMMAND_SET_UNIFORM_FLOAT]    = gs_opengl_cmd_set_uniform_float,
    [GS_COMMAND_SET_UNIFORM_VEC2]     = gs_opengl_cmd_set_uniform_vec2,
    [GS_COMMAND_SET_UNIFORM_VEC3]     = gs_opengl_cmd_set_uniform_vec3,
    [GS_COMMAND_SET_UNIFORM_VEC4]     = gs_opengl_cmd_set_uniform_vec4,
    [GS_COMMAND_SET_UNIFORM_MAT4]     = gs_opengl_cmd_set_uniform_mat4,
    [GS_COMMAND_COPY_TEXTURE]         = gs_opengl_cmd_copy_texture,
    [GS_COMMAND_RESOLVE_TEXTURE]      = gs_opengl_cmd_resolve_texture,
    [GS_COMMAND_GEN_MIPMAPS]          = gs_opengl_cmd_generate_mipmaps,
    [GS_COMMAND_COPY_TEXTURE_PARTIAL] = gs_opengl_cmd_copy_texture_partial,
};

// State
GsBuffer* bound_vertex_buffer = NULL;
GsBuffer* bound_index_buffer = NULL;
GsProgram* bound_program = NULL;
GsVtxLayout* bound_layout = NULL;
GsFramebuffer* bound_framebuffer = NULL;
GsTexture** bound_textures = NULL;
GsOpenGLViewport bound_viewport = { -1, -1, -1, -1 };
GsOpenGLColor clear_color = { 0.0f, 0.0f, 0.0f,  -1.0f };
int bound_texture_slot = 0;
GsPipeline* bound_pipeline = NULL;
GsBuffer* requested_vertex_buffer = NULL;
GsBuffer* requested_index_buffer = NULL;
GsProgram* requested_program = NULL;
GsVtxLayout* requested_layout = NULL;
GsFramebuffer* requested_framebuffer = NULL;
GsTexture** requested_textures = NULL;
GsOpenGLViewport requested_viewport = { 0, 0, 0, 0 };
GsBlendFactor blend_src = -1;
GsBlendFactor blend_dst = -1;
GsBlendOp blend_op = -1;
GsBlendFactor blend_src_alpha = -1;
GsBlendFactor blend_dst_alpha = -1;
GsBlendOp blend_op_alpha = -1;
GS_BOOL blend_enabled = -1;
GsDepthFunc depth_func = -1;
GS_BOOL depth_write_enabled = -1;
GS_BOOL depth_test_enabled = -1;
GS_BOOL stencil_test_enabled = -1;
GS_BOOL msaa_enabled = -1;

GS_BOOL gs_opengl_cmp_viewport(const GsOpenGLViewport a, const GsOpenGLViewport b) {
    return a.x == b.x && a.y == b.y && a.width == b.width && a.height == b.height;
}

static GsOpenGLStateStack state_stack[GS_OPENGL_MAX_STATE_STACK];
static int state_stack_index = 0;

void gs_opengl_push_state() {
    GS_ASSERT(state_stack_index < GS_OPENGL_MAX_STATE_STACK);

    GsTexture** requested_textures_copy = GS_ALLOC_MULTIPLE(GsTexture*, GS_MAX_TEXTURE_SLOTS);
    for (int i = 0; i < GS_MAX_TEXTURE_SLOTS; i++) {
        requested_textures_copy[i] = requested_textures[i];
    }

    GsOpenGLStateStack state = {
        .vertex_buffer = requested_vertex_buffer,
        .index_buffer = requested_index_buffer,
        .pipeline = bound_pipeline,
        .framebuffer = requested_framebuffer,
        .textures = requested_textures_copy,
        .viewport = { requested_viewport.x, requested_viewport.y, requested_viewport.width, requested_viewport.height },
    };

    state_stack[state_stack_index] = state;
    state_stack_index++;
}

void gs_opengl_pop_state() {
    state_stack_index--;
    GS_ASSERT(state_stack_index >= 0);

    if (state_stack[state_stack_index].vertex_buffer != NULL) {
        requested_vertex_buffer = state_stack[state_stack_index].vertex_buffer;
    }

    if (state_stack[state_stack_index].index_buffer != NULL) {
        requested_index_buffer = state_stack[state_stack_index].index_buffer;
    }

    requested_framebuffer = state_stack[state_stack_index].framebuffer; // framebuffer may be null
    requested_viewport = state_stack[state_stack_index].viewport; // always set

    gs_opengl_bind_viewport();

    for (int i = 0; i < GS_MAX_TEXTURE_SLOTS; i++) {
        requested_textures[i] = state_stack[state_stack_index].textures[i];
    }

    if (bound_pipeline != state_stack[state_stack_index].pipeline && state_stack[state_stack_index].pipeline != NULL) {
        gs_opengl_internal_bind_pipeline(state_stack[state_stack_index].pipeline);
    }

    GS_FREE(state_stack[state_stack_index].textures);
}

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
    backend->clear_texture = gs_opengl_clear_texture;

    // render pass
    backend->create_render_pass_handle = gs_opengl_create_render_pass;
    backend->destroy_render_pass_handle = gs_opengl_destroy_render_pass;

    // framebuffer
    backend->create_framebuffer = gs_opengl_create_framebuffer;
    backend->destroy_framebuffer = gs_opengl_destroy_framebuffer;
    backend->framebuffer_attach_texture = gs_opengl_framebuffer_attach_texture;

    // init state
    bound_textures = GS_ALLOC_MULTIPLE(GsTexture*, GS_MAX_TEXTURE_SLOTS);
    requested_textures = GS_ALLOC_MULTIPLE(GsTexture*, GS_MAX_TEXTURE_SLOTS);

    GS_LOG("OpenGL backend initialized.\n");
    const char *glVersion = (const char *) glGetString(GL_VERSION);
    if (glVersion != NULL) {
        GS_LOG("OpenGL version: %s\n", glVersion);
    } else {
        GS_LOG("OpenGL version not available.\n");
    }
    const char *glVendor = (const char *) glGetString(GL_VENDOR);
    if (glVendor != NULL) {
        GS_LOG("OpenGL vendor: %s\n", glVendor);
    } else {
        GS_LOG("OpenGL vendor not available.\n");
    }
    const char *glRenderer = (const char *) glGetString(GL_RENDERER);
    if (glRenderer != NULL) {
        GS_LOG("OpenGL renderer: %s\n", glRenderer);
    } else {
        GS_LOG("OpenGL renderer not available.\n");
    }
    const char *glShadingLanguageVersion = (const char *) glGetString(GL_SHADING_LANGUAGE_VERSION);
    if (glShadingLanguageVersion != NULL) {
        GS_LOG("OpenGL shading language version: %s\n", glShadingLanguageVersion);
    } else {
        GS_LOG("OpenGL shading language version not available.\n");
    }

    for (int i = 0; i < GS_MAX_TEXTURE_SLOTS; i++) {
        bound_textures[i] = NULL;
        requested_textures[i] = NULL;
    }

    return backend;
}

void gs_opengl_internal_active_texture(int slot) {
    if (slot != bound_texture_slot) {
        glActiveTexture(GL_TEXTURE0 + slot);
        bound_texture_slot = slot;
    }
}

static void gs_opengl_bind_vertex_buffer() {
    #if defined(GS_OPENGL_V460) || defined(GS_OPENGL_V320ES)
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
    #endif

    #if defined(GS_OPENGL_V200ES)
    if (requested_vertex_buffer != bound_vertex_buffer) {
        if (requested_vertex_buffer != NULL) {
            GsOpenGLBufferHandle *handle = (GsOpenGLBufferHandle*)requested_vertex_buffer->handle;
            glBindBuffer(GL_ARRAY_BUFFER, handle->handle);

            gs_opengl_internal_bind_layout_state();
        } else {
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        bound_vertex_buffer = requested_vertex_buffer;
    }
    #endif
}

static void gs_opengl_bind_index_buffer() {
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
}

static void gs_opengl_bind_program() {
    if (requested_program != bound_program) {
        GS_ASSERT(requested_program != NULL);
        glUseProgram(*(GLuint*)requested_program->handle);
        bound_program = requested_program;
    }
}

static void gs_opengl_bind_layout() {
    if (requested_layout != bound_layout) {
        gs_opengl_internal_bind_layout_state();
    }
}

static void gs_opengl_bind_textures() {
    for (int i = 0; i < GS_MAX_TEXTURE_SLOTS; i++) {
        if (requested_textures[i] != bound_textures[i]) {
            gs_opengl_internal_active_texture(i);

            if (requested_textures[i] != NULL) {
                glBindTexture(GL_TEXTURE_2D, *(GLuint*)requested_textures[i]->handle);
            } else {
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            bound_textures[i] = requested_textures[i];
        }
    }
}

static void gs_opengl_bind_framebuffer() {
    if (bound_framebuffer != requested_framebuffer) {
        if (requested_framebuffer != NULL) {
            GLuint* handle = (GLuint*)requested_framebuffer->handle;
            glBindFramebuffer(GL_FRAMEBUFFER, *handle);
        } else {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        bound_framebuffer = requested_framebuffer;
    }
}

void gs_opengl_internal_bind_state() {
    gs_opengl_bind_vertex_buffer();
    gs_opengl_bind_index_buffer();
    gs_opengl_bind_program();
    gs_opengl_bind_layout();
    gs_opengl_bind_textures();
    gs_opengl_bind_framebuffer();
    gs_opengl_bind_viewport();
}

void gs_opengl_bind_viewport() {
    if (!gs_opengl_cmp_viewport(requested_viewport, bound_viewport)) {
        glViewport(requested_viewport.x, requested_viewport.y, requested_viewport.width, requested_viewport.height);
        bound_viewport = requested_viewport;
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

void gs_opengl_internal_bind_framebuffer(GsFramebuffer *framebuffer) {
    // NOTE: Framebuffer may be null
    requested_framebuffer = framebuffer;
}

void gs_opengl_internal_unbind_framebuffer() {
    requested_framebuffer = NULL;
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
    #if defined(GS_OPENGL_V460)
        glCreateBuffers(1, &vbo);
    #endif

    #if defined(GS_OPENGL_V200ES) || defined(GS_OPENGL_V320ES)
        glGenBuffers(1, &vbo);
    #endif

    GsOpenGLBufferHandle *handle = GS_ALLOC(GsOpenGLBufferHandle);
    handle->handle = vbo;
    handle->vaoHandle = 0xFF; // invalid
    handle->lastLayout = NULL;
    handle->lastIndexBuffer = NULL;

    #if defined(GS_OPENGL_V460) || defined(GS_OPENGL_V320ES)
        if (buffer->type == GS_BUFFER_TYPE_VERTEX) {
            GLuint vao;
            glGenVertexArrays(1, &vao);
            handle->vaoHandle = vao;
        }
    #endif

    buffer->handle = (void*)handle;
}

void gs_opengl_internal_restore_buffer(GsBuffer* temp_buffer) {
    if (temp_buffer->type == GS_BUFFER_TYPE_VERTEX) {
        if (bound_vertex_buffer != NULL) {
            GsOpenGLBufferHandle *bound_handle = (GsOpenGLBufferHandle*)bound_vertex_buffer->handle;
            glBindBuffer(GL_ARRAY_BUFFER, bound_handle->handle);
        } else {
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
    } else if (temp_buffer->type == GS_BUFFER_TYPE_INDEX) {
        if (bound_index_buffer != NULL) {
            GsOpenGLBufferHandle *bound_handle = (GsOpenGLBufferHandle*)bound_index_buffer->handle;
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bound_handle->handle);
        } else {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }
    }
}

void gs_opengl_set_buffer_data(GsBuffer *buffer, void *data, int size) {
    GS_ASSERT(buffer != NULL);
    GS_ASSERT(data != NULL);
    GS_ASSERT(size > 0);

    #if defined(GS_OPENGL_V460)
        glNamedBufferData(((GsOpenGLBufferHandle*)buffer->handle)->handle, size, data, gs_opengl_get_buffer_intent(buffer->intent));
    #endif

    #if defined(GS_OPENGL_V320ES) || defined(GS_OPENGL_V200ES)
        GsOpenGLBufferHandle *handle = (GsOpenGLBufferHandle*)buffer->handle;
        glBindBuffer(gs_opengl_get_buffer_type(buffer->type), handle->handle);
        glBufferData(gs_opengl_get_buffer_type(buffer->type), size, data, gs_opengl_get_buffer_intent(buffer->intent));

        gs_opengl_internal_restore_buffer(buffer);
    #endif
}

void gs_opengl_set_buffer_partial_data(GsBuffer *buffer, void *data, int size, int offset) {
    GS_ASSERT(buffer != NULL);
    GS_ASSERT(data != NULL);
    GS_ASSERT(size > 0);
    GS_ASSERT(offset >= 0);

    #if defined(GS_OPENGL_V460)
        glNamedBufferSubData(((GsOpenGLBufferHandle*)buffer->handle)->handle, offset, size, data);
    #endif

    #if defined(GS_OPENGL_V320ES) || defined(GS_OPENGL_V200ES)
        GsOpenGLBufferHandle *handle = (GsOpenGLBufferHandle*)buffer->handle;
        glBindBuffer(gs_opengl_get_buffer_type(buffer->type), handle->handle);
        glBufferSubData(gs_opengl_get_buffer_type(buffer->type), offset, size, data);

        gs_opengl_internal_restore_buffer(buffer);
    #endif
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

    #if defined(GS_OPENGL_V460) || defined(GS_OPENGL_V320ES)
        if (buffer->type == GS_BUFFER_TYPE_VERTEX) {
            glDeleteVertexArrays(1, &handle->vaoHandle);
        }
    #endif

    GS_FREE(buffer->handle);
    buffer->handle = NULL;
}

#ifdef GS_OPENGL_DEBUG
void gs_opengl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
        return;
    }

    GS_LOG("OpenGL Debug: %s\n", message);
}
#endif

GS_BOOL gs_opengl_init(GsBackend *backend, GsConfig *config) {
    // GS_ASSERT(config->window != NULL);
    GS_ASSERT(backend != NULL);

    #ifdef GS_OPENGL_USE_GLAD
        const int res = gladLoadGL((GLADloadfunc) gs_opengl_getproc);

        if (!res) {
            GS_LOG("Failed to load OpenGL\n");
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

    if (clear_color.r != cmd->r || clear_color.g != cmd->g || clear_color.b != cmd->b || clear_color.a != cmd->a) {
        glClearColor(cmd->r, cmd->g, cmd->b, cmd->a);
        clear_color.r = cmd->r;
        clear_color.g = cmd->g;
        clear_color.b = cmd->b;
        clear_color.a = cmd->a;
    }

    glClear(flags);
}

void gs_opengl_cmd_set_viewport(const GsCommandListItem item) {
    const GsViewportCommand *cmd = (GsViewportCommand *) item.data;
    if (cmd->width <= 0 || cmd->height <= 0) {
        return;
    }

    if (bound_framebuffer != requested_framebuffer) {
        if (requested_framebuffer != NULL) {
            GLuint* handle = (GLuint*)requested_framebuffer->handle;
            glBindFramebuffer(GL_FRAMEBUFFER, *handle);
        } else {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        bound_framebuffer = requested_framebuffer;
    }

    requested_viewport.x = cmd->x;
    requested_viewport.y = cmd->y;
    requested_viewport.width = cmd->width;
    requested_viewport.height = cmd->height;
    gs_opengl_bind_viewport();
}

void gs_opengl_internal_bind_pipeline(GsPipeline *pipeline) {
    gs_opengl_internal_bind_program(pipeline->program);
    gs_opengl_internal_bind_layout(pipeline->layout);

    GS_OPENGL_PIPELINE_CAP(pipeline->blend_enabled, blend_enabled, GL_BLEND);
    GS_OPENGL_PIPELINE_CAP(pipeline->depth_test, depth_test_enabled, GL_DEPTH_TEST);
    GS_OPENGL_PIPELINE_CAP(pipeline->stencil_test, stencil_test_enabled, GL_STENCIL_TEST);

    if (
        pipeline->blend_dst != blend_dst || pipeline->blend_dst_alpha != blend_dst_alpha ||
        pipeline->blend_src != blend_src || pipeline->blend_src_alpha != blend_src_alpha ||
        pipeline->blend_op != blend_op || pipeline->blend_op_alpha != blend_op_alpha
    ) {
        glBlendFuncSeparate(
            gs_opengl_get_blend_factor(pipeline->blend_src),
            gs_opengl_get_blend_factor(pipeline->blend_dst),
            gs_opengl_get_blend_factor(pipeline->blend_src_alpha),
            gs_opengl_get_blend_factor(pipeline->blend_dst_alpha)
        );
        glBlendEquationSeparate(
            gs_opengl_get_blend_op(pipeline->blend_op),
            gs_opengl_get_blend_op(pipeline->blend_op_alpha)
        );

        blend_src = pipeline->blend_src;
        blend_dst = pipeline->blend_dst;
        blend_src_alpha = pipeline->blend_src_alpha;
        blend_dst_alpha = pipeline->blend_dst_alpha;
        blend_op = pipeline->blend_op;
        blend_op_alpha = pipeline->blend_op_alpha;
    }

    if (pipeline->msaa_samples > 0 && !msaa_enabled) {
        #if defined(GS_OPENGL_V460)
            glEnable(GL_MULTISAMPLE);
        #endif
        msaa_enabled = GS_TRUE;
    }

    if (pipeline->msaa_samples == 0 && msaa_enabled) {
        #if defined(GS_OPENGL_V460)
            glDisable(GL_MULTISAMPLE);
        #endif
        msaa_enabled = GS_FALSE;
    }

    if (pipeline->depth_write != depth_write_enabled) {
        if (pipeline->depth_write) {
            glDepthMask(GL_TRUE);
        } else {
            glDepthMask(GL_FALSE);
        }

        depth_write_enabled = pipeline->depth_write;
    }

    if (pipeline->depth_func != depth_func) {
        glDepthFunc(gs_opengl_get_depth_func(pipeline->depth_func));
        depth_func = pipeline->depth_func;
    }

    bound_pipeline = pipeline;
}

void gs_opengl_cmd_use_pipeline(GsCommandListItem item) {
    const GsPipelineCommand *cmd = (GsPipelineCommand *) item.data;
    GsPipeline *pipeline = cmd->pipeline;

    gs_opengl_internal_bind_pipeline(pipeline);
}

void gs_opengl_cmd_use_texture(const GsCommandListItem item) {
    const GsTextureCommand *cmd = (GsTextureCommand *) item.data;
    gs_opengl_internal_bind_texture(cmd->texture, cmd->slot);
}

void gs_opengl_cmd_begin_render_pass(const GsCommandListItem item) {
    const GsBeginRenderPassCommand *cmd = (GsBeginRenderPassCommand *) item.data;
    GS_ASSERT(cmd->pass != NULL);

    gs_opengl_push_state();
    gs_opengl_internal_bind_framebuffer(cmd->pass->framebuffer);
}

void gs_opengl_cmd_end_render_pass(const GsCommandListItem item) {
    const GsEndRenderPassCommand *cmd = (GsEndRenderPassCommand *) item.data;

    gs_opengl_pop_state();
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

void gs_opengl_cmd_copy_texture(const GsCommandListItem item) {
    const GsCopyTextureCommand *cmd = (GsCopyTextureCommand *) item.data;

    #if defined(GS_OPENGL_V460)
        glCopyImageSubData(*(GLuint*)cmd->src->handle, GL_TEXTURE_2D, 0, 0, 0, 0, *(GLuint*)cmd->dst->handle, GL_TEXTURE_2D, 0, 0, 0, 0, cmd->src->width, cmd->src->height, 1);
    #endif

    #if defined(GS_OPENGL_V200ES) || defined(GS_OPENGL_V320ES)
        GS_OPENGL_GLES2_COPY_TEXTURE_FALLBACK(copy_fbo, cmd->src, cmd->dst);
    #endif
}

void gs_opengl_cmd_resolve_texture(const GsCommandListItem item) {
    const GsResolveTextureCommand *cmd = (GsResolveTextureCommand *) item.data;

    gs_opengl_internal_bind_texture(cmd->src, 0);
    gs_opengl_internal_bind_texture(cmd->dst, 1);

    #if defined(GS_OPENGL_V460) || defined(GS_OPENGL_V320ES)
        glBlitFramebuffer(0, 0, cmd->src->width, cmd->src->height, 0, 0, cmd->dst->width, cmd->dst->height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    #endif

    #if defined(GS_OPENGL_V200ES)
        GS_OPENGL_GLES2_COPY_TEXTURE_FALLBACK(resolve_fbo, cmd->src, cmd->dst);
    #endif
}

void gs_opengl_cmd_generate_mipmaps(const GsCommandListItem item) {
    const GsGenMipmapsCommand *cmd = (GsGenMipmapsCommand *) item.data;

    gs_opengl_internal_bind_texture(cmd->texture, 0);
    glGenerateMipmap(GL_TEXTURE_2D);
}

void gs_opengl_cmd_copy_texture_partial(const GsCommandListItem item) {
    const GsCopyTexturePartialCommand *cmd = (GsCopyTexturePartialCommand *) item.data;

    gs_opengl_internal_bind_texture(cmd->src, 0);
    gs_opengl_internal_bind_texture(cmd->dst, 1);

    #if defined(GS_OPENGL_V460)
        glCopyImageSubData(*(GLuint*)cmd->src->handle, GL_TEXTURE_2D, 0, cmd->src_x, cmd->src_y, 0, *(GLuint*)cmd->dst->handle, GL_TEXTURE_2D, 0, cmd->dst_x, cmd->dst_y, 0, cmd->width, cmd->height, 1);
    #endif

    #if defined(GS_OPENGL_V200ES) || defined(GS_OPENGL_V320ES)
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, cmd->dst_x, cmd->dst_y, cmd->src_x, cmd->src_y, cmd->width, cmd->height);
    #endif
}

const char *gs_opengl_command_names[] = {
    "GS_COMMAND_CLEAR",
    "GS_COMMAND_SET_VIEWPORT",
    "GS_COMMAND_USE_PIPELINE",
    "GS_COMMAND_USE_TEXTURE",
    "GS_COMMAND_USE_BUFFER",
    "GS_COMMAND_BEGIN_PASS",
    "GS_COMMAND_END_PASS",
    "GS_COMMAND_DRAW_ARRAYS",
    "GS_COMMAND_DRAW_INDEXED",
    "GS_COMMAND_SET_SCISSOR",
    "GS_COMMAND_SET_UNIFORM_INT",
    "GS_COMMAND_SET_UNIFORM_FLOAT",
    "GS_COMMAND_SET_UNIFORM_VEC2",
    "GS_COMMAND_SET_UNIFORM_VEC3",
    "GS_COMMAND_SET_UNIFORM_VEC4",
    "GS_COMMAND_SET_UNIFORM_MAT4",
    "GS_COMMAND_COPY_TEXTURE",
    "GS_COMMAND_RESOLVE_TEXTURE",
    "GS_COMMAND_GEN_MIPMAPS",
    "GS_COMMAND_COPY_TEXTURE_PARTIAL"
};

void gs_opengl_submit(GsBackend *backend, GsCommandList *list) {
    GS_ASSERT(backend != NULL);
    GS_ASSERT(list != NULL);

    for (int i = 0; i < list->count; i++) {
        const GsCommandListItem item = list->items[i];
        GS_ASSERT(item.type >= 0);
        GS_ASSERT(item.type < GS_TABLE_SIZE(gs_opengl_commands));

        const GsCommandHandler handler = gs_opengl_commands[item.type];
        handler(item);
    }

    #if defined(GS_OPENGL_LOG_ERRORS)
        GLenum error = glGetError();
        while (error != GL_NO_ERROR) {
            GS_LOG("OpenGL Error: %d\n", error);
            error = glGetError();
        }
    #endif
}

void gs_opengl_create_render_pass(GsRenderPass *pass) {
    // GL backend does not need to create a render pass
}

void gs_opengl_destroy_render_pass(GsRenderPass *pass) {
    // GL backend does not need to destroy a render pass
}

void gs_opengl_create_shader(GsShader *shader, const char *source) {
    GS_ASSERT(shader != NULL);

    GLuint* handle = GS_ALLOC(GLuint);
    *handle = glCreateShader(shader->type == GS_SHADER_TYPE_VERTEX ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);

    glShaderSource(*handle, 1, &source, NULL);
    glCompileShader(*handle);

    GLint compiled = 0;
    glGetShaderiv(*handle, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint logLength = 0;
        glGetShaderiv(*handle, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 1) {
            char *log = (char *)GS_ALLOC_MULTIPLE(char, logLength);
            glGetShaderInfoLog(*handle, logLength, NULL, log);
            GS_LOG("Shader compile error: %s\n", log);
            GS_FREE(log);
        } else {
            GS_LOG("Shader compile failed with no log.\n");
        }
    }

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

int gs_opengl_get_buffer_type(GsBufferType type) {
    GS_ASSERT(type >= 0);
    GS_ASSERT(type < GS_TABLE_SIZE(gs_opengl_buffer_types));

    return gs_opengl_buffer_types[type];
}

int gs_opengl_get_buffer_intent(GsBufferIntent intent) {
    GS_ASSERT(intent >= 0);
    GS_ASSERT(intent < GS_TABLE_SIZE(gs_opengl_buffer_intents));

    return gs_opengl_buffer_intents[intent];
}

int gs_opengl_get_blend_factor(GsBlendFactor factor) {
    GS_ASSERT(factor >= 0);
    GS_ASSERT(factor < GS_TABLE_SIZE(gs_opengl_blend_factors));

    return gs_opengl_blend_factors[factor];
}

int gs_opengl_get_blend_op(GsBlendOp op) {
    GS_ASSERT(op >= 0);
    GS_ASSERT(op < GS_TABLE_SIZE(gs_opengl_blend_ops));

    int res = gs_opengl_blend_ops[op];
    GS_ASSERT(res != -1);

    return res;
}

int gs_opengl_get_attrib_type(GsVtxAttribType type) {
    GS_ASSERT(type >= 0);
    GS_ASSERT(type < GS_TABLE_SIZE(gs_opengl_attrib_types));
    int res = gs_opengl_attrib_types[type];

    GS_ASSERT(res != -1);
    return res;
}

int gs_opengl_get_face_type(GsCubemapFace face) {
    GS_ASSERT(face >= 0);
    GS_ASSERT(face < GS_TABLE_SIZE(gs_opengl_face_types));

    return gs_opengl_face_types[face];
}

int gs_opengl_get_texture_type(GsTextureType type) {
    GS_ASSERT(type >= 0);
    GS_ASSERT(type < GS_TABLE_SIZE(gs_opengl_texture_types));

    return gs_opengl_texture_types[type];
}

int gs_opengl_get_texture_format(GsTextureFormat format) {
    GS_ASSERT(format >= 0);
    GS_ASSERT(format < GS_TABLE_SIZE(gs_opengl_texture_formats));

    int res = gs_opengl_texture_formats[format];
    GS_ASSERT(res != -1);

    return res;
}

int gs_opengl_get_texture_wrap(GsTextureWrap wrap) {
    GS_ASSERT(wrap >= 0);
    GS_ASSERT(wrap < GS_TABLE_SIZE(gs_opengl_texture_wraps));

    return gs_opengl_texture_wraps[wrap];
}

int gs_opengl_get_texture_filter(GsTextureFilter filter) {
    GS_ASSERT(filter >= 0);
    GS_ASSERT(filter < GS_TABLE_SIZE(gs_opengl_texture_filters));

    return gs_opengl_texture_filters[filter];
}

int gs_opengl_get_depth_func(GsDepthFunc func) {
    GS_ASSERT(func >= 0);
    GS_ASSERT(func < GS_TABLE_SIZE(gs_opengl_depth_func));

    return gs_opengl_depth_func[func];
}

void gs_opengl_create_texture(GsTexture *texture) {
    GS_ASSERT(texture != NULL);

    GLuint* handle = GS_ALLOC(GLuint);
    glGenTextures(1, handle);

    texture->handle = handle;
}

void gs_opengl_clear_texture(GsTexture *texture) {
    GS_ASSERT(texture != NULL);

    if (bound_textures[0] != texture) {
        gs_opengl_internal_active_texture(0);
        glBindTexture(gs_opengl_get_texture_type(texture->type), *(GLuint*)texture->handle);
        bound_textures[0] = texture;
    }

    gs_opengl_update_texture_state(texture);

    switch (texture->type) {
        case GS_TEXTURE_TYPE_2D:
            glTexImage2D(GL_TEXTURE_2D, 0, gs_opengl_get_texture_format(texture->format), texture->width, texture->height, 0, gs_opengl_get_texture_format(texture->format), GL_UNSIGNED_BYTE, NULL);
            break;
        case GS_TEXTURE_TYPE_CUBEMAP:
            for (int i = 0; i < 6; i++) {
                glTexImage2D(gs_opengl_get_face_type(i), 0, gs_opengl_get_texture_format(texture->format), texture->width, texture->height, 0, gs_opengl_get_texture_format(texture->format), GL_UNSIGNED_BYTE, NULL);
            }
            break;
    }
}

void gs_opengl_update_texture_state(GsTexture* texture) {
    GS_ASSERT(texture != NULL);

    if (bound_textures[0] != texture) {
        gs_opengl_internal_active_texture(0);
        glBindTexture(gs_opengl_get_texture_type(texture->type), *(GLuint*)texture->handle);
        bound_textures[0] = texture;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gs_opengl_get_texture_wrap(texture->wrap_s));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gs_opengl_get_texture_wrap(texture->wrap_t));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gs_opengl_get_texture_filter(texture->min));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gs_opengl_get_texture_filter(texture->mag));

    #if defined(GS_OPENGL_V460)
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, texture->lodBias);

        if (texture->type == GS_TEXTURE_TYPE_CUBEMAP) {
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, gs_opengl_get_texture_wrap(texture->wrap_r));
        }
    #endif
}

void gs_opengl_set_texture_data(GsTexture *texture, GsCubemapFace face, void *data) {
    GS_ASSERT(texture != NULL);
    GS_ASSERT(data != NULL);

    if (bound_textures[0] != texture) {
        gs_opengl_internal_active_texture(0);
        glBindTexture(gs_opengl_get_texture_type(texture->type), *(GLuint*)texture->handle);
        bound_textures[0] = texture;
    }

    gs_opengl_update_texture_state(texture);

    switch (texture->type) {
        case GS_TEXTURE_TYPE_2D:
            glTexImage2D(GL_TEXTURE_2D, 0, gs_opengl_get_texture_format(texture->format), texture->width, texture->height, 0, gs_opengl_get_texture_format(texture->format), GL_UNSIGNED_BYTE, data);
            break;
        case GS_TEXTURE_TYPE_CUBEMAP:
            glTexImage2D(gs_opengl_get_face_type(face), 0, gs_opengl_get_texture_format(texture->format), texture->width, texture->height, 0, gs_opengl_get_texture_format(texture->format), GL_UNSIGNED_BYTE, data);
            break;
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

    gs_opengl_internal_active_texture(0);
    glBindTexture(gs_opengl_get_texture_type(texture->type), *(GLuint*)texture->handle);
    bound_textures[0] = texture;

    glGenerateMipmap(GL_TEXTURE_2D);
}

void gs_opengl_create_framebuffer(GsFramebuffer *framebuffer) {
    GS_ASSERT(framebuffer != NULL);

    GLuint* handle = GS_ALLOC(GLuint);
    glGenFramebuffers(1, handle);

    framebuffer->handle = handle;
}

void gs_opengl_framebuffer_attach_texture(GsFramebuffer *framebuffer, GsTexture *texture, GsFramebufferAttachmentType attachment) {
    GS_ASSERT(framebuffer != NULL);
    GS_ASSERT(texture != NULL);

    if (bound_framebuffer != framebuffer) {
        glBindFramebuffer(GL_FRAMEBUFFER, *(GLuint*)framebuffer->handle);
        bound_framebuffer = framebuffer;
    }

    switch (attachment) {
        case GS_FRAMEBUFFER_ATTACHMENT_COLOR:
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, gs_opengl_get_texture_type(texture->type), *(GLuint*)texture->handle, 0);
            break;
        case GS_FRAMEBUFFER_ATTACHMENT_DEPTH:
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, gs_opengl_get_texture_type(texture->type), *(GLuint*)texture->handle, 0);
            break;
        case GS_FRAMEBUFFER_ATTACHMENT_STENCIL:
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, gs_opengl_get_texture_type(texture->type), *(GLuint*)texture->handle, 0);
            break;
        case GS_FRAMEBUFFER_ATTACHMENT_DEPTH_STENCIL:
            #if defined(GS_OPENGL_V460) || defined(GS_OPENGL_V320ES)
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, gs_opengl_get_texture_type(texture->type), *(GLuint*)texture->handle, 0);
            #endif

            #if defined(GS_OPENGL_V200ES)
                // always fail because it is not supported
                GS_ASSERT(0);
            #endif
            break;
    }
}

void gs_opengl_destroy_framebuffer(GsFramebuffer *framebuffer) {
    GS_ASSERT(framebuffer != NULL);

    glDeleteFramebuffers(1, (GLuint*)framebuffer->handle);

    GS_FREE(framebuffer->handle);
    framebuffer->handle = NULL;
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


