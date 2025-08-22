#include "wasm_display.h"
#include "texture.h" // For TEXTURE_TYPE_* definitions
#include "renderer.h" // Include renderer.h to get the renderer_t definition
#include <SDL2/SDL.h>
#include <GLES3/gl3.h>
#include <stdio.h>
#include "world.h"

// Forward declarations for our WASM-specific renderer functions
static void WASM_FreeGPUTexture(texture_t* texture);
static void WASM_UpLoadTextureToGpu(texture_t* texture);
static void WASM_RenderEntities(void); // Forward declaration
// Add stubs for other functions to avoid null pointers
static void WASM_Stub_Void(void) {}
static void WASM_Stub_SetTexture(unsigned int id) {}
static void WASM_Stub_RenderString(xf_colorless_sprite_t* vertices,ushort* indices, uint numIndices) {}
static void WASM_Stub_GetColorBuffer(uchar* data) {}
static void WASM_Stub_UpLoadEntityToGPU(entity_t* entity) {}
static uint WASM_Stub_UploadVerticesToGPU(void* vertices, uint mem_size) { return 0; }
static void WASM_Stub_FreeGPUBuffer(uint buffer) {}
static void WASM_Stub_RenderColorlessSprites(xf_colorless_sprite_t* vertices, ushort numIndices, ushort* indices) {}
static void WASM_Stub_FadeScreen(float alpha) {}
static void WASM_Stub_SetMaterialTextureBlending(char modulate) {}
static void WASM_Stub_SetTransparency(float alpha) {}
static int WASM_Stub_IsTextureCompressionSupported(int type) { return 0; }
static void WASM_Stub_RefreshViewPort(void) {}


static SDL_Window* window = NULL;
static SDL_GLContext gl_context = NULL;
static int current_width = 800;
static int current_height = 600;

// --- Shader management structures and constants from renderer_progr.c ---
#include "log.h"
#include "filesystem.h"

typedef struct simple_shader_t {
	GLuint prog;
	GLuint vars[32];
} shader_prog_t;

#define SHADER_MVT_MATRIX               0
#define SHADER_ATT_VERTEX               1
#define SHADER_ATT_NORMAL               2
#define SHADER_ATT_UV                   3
#define SHADER_TEXT_COLOR_SAMPLER       5
#define SHADER_UNI_LIGHT_POS            7
#define SHADER_UNI_LIGHT_COL_AMBIENT    8
#define SHADER_UNI_LIGHT_COL_DIFFUSE    9

static shader_prog_t simpleShader;

// --- Shader Loading Functions (adapted from renderer_progr.c) ---

static GLuint WASM_LoadShader(const char *shaderSrcPath, GLenum type)
{
	GLuint shader;
	GLint compiled;
    filehandle_t* shaderFile;

    shaderFile = FS_OpenFile(shaderSrcPath, "rt");
    if (!shaderFile) {
        Log_Printf("[WASM] Could not open shader file: %s", shaderSrcPath);
        return 0;
    }
    FS_UploadToRAM(shaderFile);

	shader = glCreateShader(type);
	if(shader == 0) {
		Log_Printf("[WASM] Failed to create GL shader for '%s'\n", shaderSrcPath);
        FS_CloseFile(shaderFile);
		return 0;
	}

    const GLchar* source = (const GLchar*)shaderFile->ptrStart;
	glShaderSource(shader, 1, &source, NULL);
    FS_CloseFile(shaderFile); // File content is copied by glShaderSource

	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

	if(!compiled) {
		GLint infoLen = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
		if(infoLen > 1) {
			char* infoLog = malloc(sizeof(char) * infoLen);
			glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
			Log_Printf("[WASM] Error compiling shader '%s':\n%s\n", shaderSrcPath, infoLog);
			free(infoLog);
		}
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

static void WASM_LoadProgram(shader_prog_t* shaderProg, const char* vertexShaderPath, const char* fragmentShaderPath)
{
	GLuint vertexShader;
	GLuint fragmentShader;
	GLint linked;

	vertexShader = WASM_LoadShader(vertexShaderPath, GL_VERTEX_SHADER);
	fragmentShader = WASM_LoadShader(fragmentShaderPath, GL_FRAGMENT_SHADER);

	if (vertexShader == 0 || fragmentShader == 0) {
        return;
    }

	shaderProg->prog = glCreateProgram();
	if(shaderProg->prog == 0) {
		Log_Printf("[WASM] Could not create GL program.");
		return;
	}

	glAttachShader(shaderProg->prog, vertexShader);
	glAttachShader(shaderProg->prog, fragmentShader);

	glLinkProgram(shaderProg->prog);
	glGetProgramiv(shaderProg->prog, GL_LINK_STATUS, &linked);

	if(!linked) {
		GLint infoLen = 0;
		glGetProgramiv(shaderProg->prog, GL_INFO_LOG_LENGTH, &infoLen);
		if(infoLen > 1) {
			char* infoLog = malloc(sizeof(char) * infoLen);
			glGetProgramInfoLog(shaderProg->prog, infoLen, NULL, infoLog);
			Log_Printf("[WASM] Error linking program:\n%s\n", infoLog);
			free(infoLog);
		}
		glDeleteProgram(shaderProg->prog);
        shaderProg->prog = 0;
	}
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

static void WASM_LoadSimpleShader(void)
{
    Log_Printf("[WASM] Loading simple shader program...\n");
    WASM_LoadProgram(&simpleShader, "data/shaders/v_simple.glsl", "data/shaders/f_simple.glsl");

    if (simpleShader.prog == 0) {
        Log_Printf("[WASM] FAILED to load simple shader program.\n");
        return;
    }

    // Get all uniforms and attributes
    simpleShader.vars[SHADER_MVT_MATRIX]             = glGetUniformLocation(simpleShader.prog, "modelViewProjectionMatrix");
    simpleShader.vars[SHADER_TEXT_COLOR_SAMPLER]     = glGetUniformLocation(simpleShader.prog, "s_baseMap");
    simpleShader.vars[SHADER_UNI_LIGHT_POS]          = glGetUniformLocation(simpleShader.prog, "lightPosition");
    simpleShader.vars[SHADER_UNI_LIGHT_COL_AMBIENT]  = glGetUniformLocation(simpleShader.prog, "lightColorAmbient");
    simpleShader.vars[SHADER_UNI_LIGHT_COL_DIFFUSE]  = glGetUniformLocation(simpleShader.prog, "lightColorDiffuse");
    simpleShader.vars[SHADER_ATT_VERTEX]             = glGetAttribLocation(simpleShader.prog, "a_vertex");
    simpleShader.vars[SHADER_ATT_NORMAL]             = glGetAttribLocation(simpleShader.prog, "a_normal");
    simpleShader.vars[SHADER_ATT_UV]                 = glGetAttribLocation(simpleShader.prog, "a_texcoord0");

    Log_Printf("[WASM] Simple shader loaded successfully. Program ID: %d\n", simpleShader.prog);
}


int wasm_display_init(int width, int height) {
    current_width = width;
    current_height = height;

    printf("[WASM Display] Initializing display system...\n");

    // Set SDL OpenGL attributes for WebGL compatibility
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    // Force RGBA color format
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

    // Create SDL window with OpenGL context
    window = SDL_CreateWindow("Shmup3D",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              width, height,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    if (!window) {
        printf("[WASM Display] ERROR: Failed to create SDL window: %s\n", SDL_GetError());
        return -1;
    }

    // Create OpenGL context
    gl_context = SDL_GL_CreateContext(window);
    if (!gl_context) {
        printf("[WASM Display] ERROR: Failed to create OpenGL context: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        window = NULL;
        return -1;
    }

    // Make context current
    if (SDL_GL_MakeCurrent(window, gl_context) != 0) {
        printf("[WASM Display] ERROR: Failed to make OpenGL context current: %s\n", SDL_GetError());
        SDL_GL_DeleteContext(gl_context);
        SDL_DestroyWindow(window);
        gl_context = NULL;
        window = NULL;
        return -1;
    }

    printf("[WASM Display] Display system initialized successfully\n");
    return 0;
}

void wasm_display_bind_renderer_methods(renderer_t* renderer) {
    printf("[WASM Display] Binding renderer methods...\n");

    if (!glGenTextures) {
        printf("[WASM Display] ERROR: WebGL context not available! glGenTextures is undefined.\n");
        return;
    }

    renderer->FreeGPUTexture = WASM_FreeGPUTexture;
    renderer->UpLoadTextureToGpu = WASM_UpLoadTextureToGpu;
    renderer->RenderEntities = WASM_RenderEntities;
    
    // Bind stubs for everything else to prevent null pointer calls
    renderer->Set3D = WASM_Stub_Void;
    renderer->StopRendition = WASM_Stub_Void;
    renderer->SetTexture = WASM_Stub_SetTexture;
    renderer->Set2D = WASM_Stub_Void;
    renderer->RenderPlayersBullets = WASM_Stub_Void;
    renderer->RenderFXSprites = WASM_Stub_Void;
    renderer->RenderString = WASM_Stub_RenderString;
    renderer->GetColorBuffer = WASM_Stub_GetColorBuffer;
    renderer->UpLoadEntityToGPU = WASM_Stub_UpLoadEntityToGPU;
    renderer->UploadVerticesToGPU = WASM_Stub_UploadVerticesToGPU;
    renderer->FreeGPUBuffer = WASM_Stub_FreeGPUBuffer;
    renderer->DrawControls = WASM_Stub_Void;
    renderer->StartCleanFrame = WASM_Stub_Void;
    renderer->RenderColorlessSprites = WASM_Stub_RenderColorlessSprites;
    renderer->FadeScreen = WASM_Stub_FadeScreen;
    renderer->SetMaterialTextureBlending = WASM_Stub_SetMaterialTextureBlending;
    renderer->SetTransparency = WASM_Stub_SetTransparency;
    renderer->IsTextureCompressionSupported = WASM_Stub_IsTextureCompressionSupported;
    renderer->RefreshViewPort = WASM_Stub_RefreshViewPort;

    // Load, compile, and link our default shader
    WASM_LoadSimpleShader();

    printf("[WASM Display] Renderer methods bound.\n");
}


static void WASM_FreeGPUTexture(texture_t* texture) {
    if (!texture) {
        printf("WASM_FreeGPUTexture: ERROR - Null texture pointer\n");
        return;
    }

    if (texture->textureId <= 0) {
        printf("WASM_FreeGPUTexture: Texture ID %d is invalid\n", texture->textureId);
        return;
    }

    // Check if WebGL context is available
    if (!glDeleteTextures) {
        printf("WASM_FreeGPUTexture: ERROR - WebGL context not available! glDeleteTextures is undefined.\n");
        return;
    }

    glDeleteTextures(1, &texture->textureId);
    printf("WASM_FreeGPUTexture: Freed texture ID %d\n", texture->textureId);
    texture->textureId = 0;
}

static void WASM_UpLoadTextureToGpu(texture_t* texture) {
    if (!texture || !texture->data) {
        return;
    }

    // Check if WebGL context is available
    if (!glGenTextures) {
        printf("WASM_UpLoadTextureToGpu: ERROR - WebGL context not available!\n");
        return;
    }

    if (texture->textureId == 0) {
        glGenTextures(1, &texture->textureId);
        if (texture->textureId == 0) {
            printf("WASM_UpLoadTextureToGpu: ERROR - Failed to generate texture ID!\n");
            return;
        }
    }
    
    glBindTexture(GL_TEXTURE_2D, texture->textureId);
    
    // Basic texture filtering. The original engine might have more complex logic.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLenum format = GL_RGBA;
    // Corrected to use the macros defined in texture.h
    // Note: The original engine seems to not have a dedicated ALPHA format constant.
    // We assume RGBA for textures that are not explicitly RGB.
    if (texture->format == TEXTURE_GL_RGB) format = GL_RGB;
    else format = GL_RGBA; // Default to RGBA for other formats like RGBA or ALPHA

    glTexImage2D(GL_TEXTURE_2D, 0, format, texture->width, texture->height, 0, format, GL_UNSIGNED_BYTE, texture->data);

    // Free CPU-side texture data after uploading to GPU
    free(texture->data);
    texture->data = NULL;
    
    printf("WASM_UpLoadTextureToGpu: Uploaded texture to ID %d\n", texture->textureId);
}


void wasm_display_shutdown(void) {
    if (gl_context) {
        SDL_GL_DeleteContext(gl_context);
        gl_context = NULL;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = NULL;
    }
}

void wasm_display_resize(int width, int height) {
    current_width = width;
    current_height = height;
    
    if (window) {
        SDL_SetWindowSize(window, width, height);
    }
    
    glViewport(0, 0, width, height);
}

void wasm_display_get_size(int* width, int* height) {
    if (width) *width = current_width;
    if (height) *height = current_height;
}

void wasm_display_swap_buffers(void) {
    if (window && gl_context) {
        SDL_GL_SwapWindow(window);
    }
}

void wasm_display_set_fullscreen(int fullscreen) {
    if (window) {
        SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
    }
}

// This function is not very useful in our context, as init does the check.
int wasm_display_check_webgl2_support(void) {
    return 1; // Assume support since we require ES 3.0
}

// Forward declarations from renderer_progr.c that we will need
matrix_t projectionMatrix;
matrix_t modelViewMatrix;
matrix_t modelViewProjectionMatrix;

#include "world.h"
#include "camera.h"
#include "enemy.h"
#include "player.h"

// We need the definition of vertex_t to correctly set attribute pointers
#include "md5.h"


static void SetupCamera(void)
{
    vec3_t vLookat;
    vectorAdd(camera.position, camera.forward, vLookat);
    gluLookAt(camera.position, vLookat, camera.up, modelViewMatrix);
    gluPerspective(camera.fov, camera.aspect, camera.zNear, camera.zFar, projectionMatrix);
}


static void WASM_RenderEntities(void) {
    if (simpleShader.prog == 0) {
        // Shader failed to load, clear to black and do nothing.
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        return;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDisable(GL_BLEND);

    glUseProgram(simpleShader.prog);

    SetupCamera();

    // Simplified light setup
    glUniform3fv(simpleShader.vars[SHADER_UNI_LIGHT_COL_AMBIENT], 1, light.ambient);
    glUniform3fv(simpleShader.vars[SHADER_UNI_LIGHT_COL_DIFFUSE], 1, light.diffuse);
    glUniform3fv(simpleShader.vars[SHADER_UNI_LIGHT_POS], 1, light.position);

    // Enable vertex attributes
    glEnableVertexAttribArray(simpleShader.vars[SHADER_ATT_VERTEX]);
    glEnableVertexAttribArray(simpleShader.vars[SHADER_ATT_NORMAL]);
    glEnableVertexAttribArray(simpleShader.vars[SHADER_ATT_UV]);

    // --- Render Map ---
    entity_t* entity = map;
    for (int i = 0; i < num_map_entities; i++, entity++) {
        if (entity->numIndices == 0 || !entity->model) continue;

        matrix_t tmp;
        matrix_multiply(modelViewMatrix, entity->matrix, tmp);
        matrix_multiply(projectionMatrix, tmp, modelViewProjectionMatrix);
        glUniformMatrix4fv(simpleShader.vars[SHADER_MVT_MATRIX], 1, GL_FALSE, modelViewProjectionMatrix);

        // Set Texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, entity->material->textures[TEXTURE_DIFFUSE].textureId);
        glUniform1i(simpleShader.vars[SHADER_TEXT_COLOR_SAMPLER], 0);

        // Set Vertex Buffers
        glBindBuffer(GL_ARRAY_BUFFER, entity->model->vboId);
        glVertexAttribPointer(simpleShader.vars[SHADER_ATT_VERTEX], 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, pos));
        glVertexAttribPointer(simpleShader.vars[SHADER_ATT_NORMAL], 3, GL_SHORT, GL_TRUE, sizeof(vertex_t), (void*)offsetof(vertex_t, normal));
        glVertexAttribPointer(simpleShader.vars[SHADER_ATT_UV], 2, GL_SHORT, GL_TRUE, sizeof(vertex_t), (void*)offsetof(vertex_t, text));

        glDrawElements(GL_TRIANGLES, entity->numIndices, GL_UNSIGNED_SHORT, entity->model->indices);
    }
    
    // --- Render Player ---
    for (int i = 0; i < numPlayers; i++) {
        entity = &players[i].entity;
        if (entity->numIndices == 0 || !entity->model) continue;
        
        matrix_t tmp;
        matrix_multiply(modelViewMatrix, entity->matrix, tmp);
        matrix_multiply(projectionMatrix, tmp, modelViewProjectionMatrix);
        glUniformMatrix4fv(simpleShader.vars[SHADER_MVT_MATRIX], 1, GL_FALSE, modelViewProjectionMatrix);

        // Set Texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, entity->material->textures[TEXTURE_DIFFUSE].textureId);
        glUniform1i(simpleShader.vars[SHADER_TEXT_COLOR_SAMPLER], 0);
        
        // Set Vertex Buffers
        glBindBuffer(GL_ARRAY_BUFFER, entity->model->vboId);
        glVertexAttribPointer(simpleShader.vars[SHADER_ATT_VERTEX], 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, pos));
        glVertexAttribPointer(simpleShader.vars[SHADER_ATT_NORMAL], 3, GL_SHORT, GL_TRUE, sizeof(vertex_t), (void*)offsetof(vertex_t, normal));
        glVertexAttribPointer(simpleShader.vars[SHADER_ATT_UV], 2, GL_SHORT, GL_TRUE, sizeof(vertex_t), (void*)offsetof(vertex_t, text));

        glDrawElements(GL_TRIANGLES, entity->model->numIndices, GL_UNSIGNED_SHORT, entity->model->indices);
    }

    // Disable vertex attributes
    glDisableVertexAttribArray(simpleShader.vars[SHADER_ATT_VERTEX]);
    glDisableVertexAttribArray(simpleShader.vars[SHADER_ATT_NORMAL]);
    glDisableVertexAttribArray(simpleShader.vars[SHADER_ATT_UV]);
}