/**
* Author: [Michael Bian]
* Assignment: Simple 2D Scene
* Date due: 2023-09-20, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

enum AppStatus { RUNNING, TERMINATED };

constexpr int WINDOW_WIDTH  = 1200,
              WINDOW_HEIGHT = 880;

// Set up the background color
constexpr float BG_RED     = 0.197f,
                BG_BLUE    = 0.146f,
                BG_GREEN   = 0.113f,
                BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X      = 0,
              VIEWPORT_Y      = 0,
              VIEWPORT_WIDTH  = WINDOW_WIDTH,
              VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
               F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

// Sources:
// Big Mom: https://images.app.goo.gl/t1UqDwkdMUhBZJnt6
// Zeus: https://images.app.goo.gl/FCLD8DbBthsLYqjk8
// Prometheus: https://images.app.goo.gl/rAWwNgbMjdhihc9J8
// Big Mom Wanted Poster: https://images.app.goo.gl/DiZhe6z4dinURz9v6
constexpr char BIG_MOM_SPRITE_FILEPATH[] = "big_mom.png";
constexpr char ZEUS_SPRITE_FILEPATH[] = "zeus.png";
constexpr char PROMETHEUS_SPRITE_FILEPATH[] = "prometheus.png";
constexpr char BIG_MOM_WANTED_SPRITE_FILEPATH[] = "big_mom_wanted.jpg";
constexpr char WANO_SPRITE_FILEPATH[] = "wano.jpg";

GLuint g_bigmom_texture_id, g_zeus_texture_id, g_prometheus_texture_id, g_bigmom_wanted_texture_id, g_wano_texture_id;

constexpr int TRIANGLE_RED     = 1.0,
              TRIANGLE_BLUE    = 0.4,
              TRIANGLE_GREEN   = 0.4,
              TRIANGLE_OPACITY = 1.0;

AppStatus g_app_status = RUNNING;
SDL_Window* g_display_window;

ShaderProgram g_shader_program = ShaderProgram();

glm::mat4 g_view_matrix,
          g_model_matrix_bigmom,
          g_model_matrix_zeus,
          g_model_matrix_prometheus,
          g_model_matrix_bigmom_wanted,
          g_model_matrix_wano,
          g_projection_matrix;

// ——————————— GLOBAL VARS AND CONSTS FOR OBJECTS' TRANSFORMATIONS ——————————— //
// constexpr int MAX_FRAME = 150; This was for an original triangle movement of big_mom,
// but I choose not to use this version because it is so unstable

// Transformation for ZEUS
constexpr float ZEUS_ORBIT_SPEED = 1.0f;
constexpr float ZEUS_ORBIT_RADIUS = 0.4f;
constexpr float ZEUS_ROT_INCREMENT = 1.0f;
float zeus_rot_angle = 0.0f;


// Transformation for PROMETHEUS
constexpr float PROMETHEUS_PULSE_SPEED = 8.0f, PROMETHEUS_BASE_SCALE = 1.5f, PROMETHEUS_MAX_AMPLITUDE = 0.1f;
float prometheus_pulse_time = 0.0f;

// Transformation for BIG MOM
constexpr float BIG_MOM_MOVE_SPEED = 1.0f;
constexpr float BIG_MOM_MAX_AMPLITUDE = 2.0f;
int  g_frame_counter = 0;
int g_mom_move_direction_counter = 0;
bool g_mom_move_up = true;
float big_mom_move_time = 0.0f;

// Transformation for BIG MOM WANTED POSTER
constexpr float BIGMOM_WANTED_ROT_INCREMENT = 0.3f;

// The movement vectors for all the objects
glm::vec3 g_model_matrix_bigmom_translate = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_model_matrix_bigmom_translate_new = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 g_model_matrix_zeus_translate = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_model_matrix_zeus_rotation = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 zeus_rot_direction = glm::vec3(1.0f, 0.0f, 0.0f);

glm::vec3 g_model_matrix_prometheus_scale = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 g_model_matrix_bigmom_wanted_roation = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 bigmom_wanted_roation_direction = glm::vec3(0.0f, 0.0f, 1.0f);

// Initial position for all the objects
constexpr glm::vec3 BIGMOM_INIT_POS = glm::vec3(0.0f, 0.0f, 0.0f);
constexpr glm::vec3 ZEUS_INIT_POS = glm::vec3(-1.0f, 1.0f, 0.0f);
constexpr glm::vec3 PROMETHEUS_POS = glm::vec3(-0.6f, 0.5f, 0.0f);
constexpr glm::vec3 BIGMOM_WANTED_INIT_POS = glm::vec3(0.0, 0.0f, 0.0f);

// Initial scale for all the objects
constexpr glm::vec3 BIG_MOM_INIT_SCALE = glm::vec3(4.82f, 3.6f, 0.0f);
constexpr glm::vec3 ZEUS_INIT_SCALE = glm::vec3(0.5f, 0.5f, 0.0f);
constexpr glm::vec3 PROMETHEUS_INIT_SCALE = glm::vec3(-0.5f, 0.63f, 0.0f);
constexpr glm::vec3 BIG_MOM_WANTED_INIT_SCALE = glm::vec3(4.2f, 5.96f, 0.0f);
constexpr glm::vec3 WANO_INIT_SCALE = glm::vec3(17.8, 10.0f, 0.0f);

// Delta time global variable for recording the previous frame
float g_previous_ticks = 0.0f;

// Texture required global variables
constexpr int NUMBER_OF_TEXTURES = 1; // to be generated, that is
constexpr GLint LEVEL_OF_DETAIL = 0; // base image level; Level n is the nth mipmap reduction image
constexpr GLint TEXTURE_BORDER = 0; // this value MUST be zero

constexpr float MILLISECONDS_IN_SECOND = 1000.0f;

// —————————————————————————————————————————————————————————————————— //

GLuint load_texture(const char* filepath) {
    // Try to load the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    // Quit if it fails
    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }
    GLuint textureID;                               // declaration
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);  // assignment
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);
        
    return textureID;
}


void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("CS3113_Assignment1_Draw a Simple 2D Scene",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    
    if (g_display_window == nullptr) {
        std::cerr << "ERROR: SDL Window could not be created.\n";
        g_app_status = TERMINATED;
            
        SDL_Quit();
        exit(1);
    }


#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_view_matrix = glm::mat4(1.0f);
    g_model_matrix_bigmom = glm::mat4(1.0f);
    g_model_matrix_zeus = glm::mat4(1.0f);
    g_model_matrix_prometheus = glm::mat4(1.0f);
    g_model_matrix_bigmom_wanted = glm::mat4(1.0f);
    g_model_matrix_wano = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED,
                 BG_BLUE,
                 BG_GREEN,
                 BG_OPACITY);
    
    
    g_bigmom_texture_id = load_texture(BIG_MOM_SPRITE_FILEPATH);
    g_zeus_texture_id = load_texture(ZEUS_SPRITE_FILEPATH);
    g_prometheus_texture_id = load_texture(PROMETHEUS_SPRITE_FILEPATH);
    g_bigmom_wanted_texture_id = load_texture(BIG_MOM_WANTED_SPRITE_FILEPATH);
    g_wano_texture_id = load_texture(WANO_SPRITE_FILEPATH);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
        {
            g_app_status = TERMINATED;
        }
    }
}


void update()
{
    // ——————————— TRANSFORMATION FOR ALL THE OBJECTS ——————————— //
    
    // Settle the delta time for each frame
    float g_ticks = (float) SDL_GetTicks() / 1000.0f;  // get the current number of g_ticks
    float g_delta_time = g_ticks - g_previous_ticks;  // the delta time is the difference from the last frame
    g_previous_ticks = g_ticks; // modify the previous ticks global variable
    
    // Increase the frame counter
    // g_frame_counter += 1;
    
    /*
    if (g_frame_counter >= MAX_FRAME) {
        ++ g_mom_move_direction_counter;
        g_frame_counter = 0;
    }
    */
    
    // g_model_matrix_bigmom_translate.x += ((g_mom_move_direction_counter % 4) == 0 ? BIG_MOM_MOVE_SPEED * g_delta_time : (g_mom_move_direction_counter % 4) == 2 ? -BIG_MOM_MOVE_SPEED * g_delta_time : 0.0f );
    
    // g_model_matrix_bigmom_translate.y += ((g_mom_move_direction_counter % 4) == 1 ? BIG_MOM_MOVE_SPEED * g_delta_time : (g_mom_move_direction_counter % 4) == 3 ? -BIG_MOM_MOVE_SPEED * g_delta_time : 0.0f);  --- All these are for the previous triangle movement of big mom.
    
    // Update big mom's transformation vectors so she can performs the up-down diagonal move.
    big_mom_move_time += BIG_MOM_MOVE_SPEED * g_delta_time;
    g_model_matrix_bigmom_translate_new.x = BIG_MOM_MAX_AMPLITUDE * glm::sin(big_mom_move_time);
    g_model_matrix_bigmom_translate_new.y = BIG_MOM_MAX_AMPLITUDE * glm::sin(big_mom_move_time);
    
    // Update zeus's transformation vectors so he can perfroms the orbiting and rotation.
    zeus_rot_angle += ZEUS_ORBIT_SPEED * g_delta_time;
    g_model_matrix_zeus_translate.x = ZEUS_ORBIT_RADIUS * glm::cos(zeus_rot_angle);
    g_model_matrix_zeus_translate.y = ZEUS_ORBIT_RADIUS * glm::sin(zeus_rot_angle);
    g_model_matrix_zeus_rotation.y += ZEUS_ROT_INCREMENT * g_delta_time;
    
    // Update prometheus' transformation vectors so he can performs the pulsing.
    prometheus_pulse_time += PROMETHEUS_PULSE_SPEED * g_delta_time;
    g_model_matrix_prometheus_scale.x = PROMETHEUS_INIT_SCALE.x + PROMETHEUS_MAX_AMPLITUDE * glm::sin(prometheus_pulse_time);
    g_model_matrix_prometheus_scale.y = PROMETHEUS_INIT_SCALE.y + PROMETHEUS_MAX_AMPLITUDE * glm::sin(prometheus_pulse_time);
    
    g_model_matrix_bigmom_wanted_roation.z += BIGMOM_WANTED_ROT_INCREMENT * g_delta_time;
    
    // Model matrix reset
    g_model_matrix_bigmom = glm::mat4(1.0f);
    g_model_matrix_zeus = glm::mat4(1.0f);
    g_model_matrix_prometheus = glm::mat4(1.0f);
    g_model_matrix_bigmom_wanted = glm::mat4(1.0f);
    g_model_matrix_wano = glm::mat4(1.0f);
    
    // Apply transformations
    g_model_matrix_bigmom = glm::translate(g_model_matrix_bigmom, BIGMOM_INIT_POS);
    g_model_matrix_bigmom = glm::translate(g_model_matrix_bigmom, g_model_matrix_bigmom_translate_new);
    g_model_matrix_bigmom = glm::scale(g_model_matrix_bigmom, BIG_MOM_INIT_SCALE);
    
    g_model_matrix_zeus = glm::translate(g_model_matrix_bigmom, g_model_matrix_zeus_translate);
    g_model_matrix_zeus = glm::rotate(g_model_matrix_zeus, g_model_matrix_zeus_rotation.y, zeus_rot_direction);
    g_model_matrix_zeus = glm::scale(g_model_matrix_zeus, ZEUS_INIT_SCALE);
    
    g_model_matrix_prometheus = glm::translate(g_model_matrix_bigmom, PROMETHEUS_POS);
    g_model_matrix_prometheus = glm::scale(g_model_matrix_prometheus, g_model_matrix_prometheus_scale);
    
    g_model_matrix_bigmom_wanted = glm::translate(g_model_matrix_bigmom_wanted, BIGMOM_WANTED_INIT_POS);
    g_model_matrix_bigmom_wanted = glm::rotate(g_model_matrix_bigmom_wanted, g_model_matrix_bigmom_wanted_roation.z, bigmom_wanted_roation_direction);
    g_model_matrix_bigmom_wanted = glm::scale(g_model_matrix_bigmom_wanted, BIG_MOM_WANTED_INIT_SCALE);
    
    g_model_matrix_wano = glm::scale(g_model_matrix_wano, WANO_INIT_SCALE);
    
    // ————————————————————————————————————————————————————————————————— //
}

void draw_object(glm::mat4 &object_model_matrix, GLuint &object_texture_id)
{
    g_shader_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void render() {
    
    glClear(GL_COLOR_BUFFER_BIT);
    
    float vertices[] = {
            -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
            -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };
    
    // Textures
    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };
    
    glVertexAttribPointer(g_shader_program.get_position_attribute(),
                          2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
 
    // Render for WANO as the background
    draw_object(g_model_matrix_wano, g_wano_texture_id);
    
    // Render for the BIG MOM's Wanted Poster
    draw_object(g_model_matrix_bigmom_wanted, g_bigmom_wanted_texture_id);
    
    // Render for Big Mom
    draw_object(g_model_matrix_bigmom, g_bigmom_texture_id);
    // Render for Zeus
    draw_object(g_model_matrix_zeus, g_zeus_texture_id);
    // Render for Pronetheus
    draw_object(g_model_matrix_prometheus, g_prometheus_texture_id);
    
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    SDL_GL_SwapWindow(g_display_window);
}


void shutdown() { SDL_Quit(); }


int main(int argc, char* argv[])
{
    initialise();

    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}
