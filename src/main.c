#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif
#include <SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include "./colors.c"
#include "./primitives.c"

// Define pi if not defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Screen dimension constants
const int SCREEN_W = 640;
const int SCREEN_H = 480;

// Number of boxes to create
#define NUM_POLYS 5

// Buffer
SDL_Window* window = NULL;
SDL_Renderer* buffer = NULL;

// Keys
Uint8* key = NULL;

// Ellipse position
float ellipse_x, ellipse_y;

// Count for rays
int number_of_rays;

// Quit status
bool quit = false;

// Box instances
PRIM_BOX boxes[NUM_POLYS];

// Random function
int random_int(int min, int max) {
  int random_number = (rand() % (max - min)) + min;
  return random_number;
}

// Make shape
void make_shape(PRIM_POINT* _points, PRIM_LINE* _lines, int _length) {
  for (int i = 0; i < _length; i++) {
    _lines[i].p1.x = _points[i].x;
    _lines[i].p1.y = _points[i].y;

    if (i < _length - 1) {
      _lines[i].p2.x = _points[i + 1].x;
      _lines[i].p2.y = _points[i + 1].y;
    } else {
      _lines[i].p2.x = _points[0].x;
      _lines[i].p2.y = _points[0].y;
    }
  }
}

// Returns distance 2D
int distance_to_2d(int x1, int y1, int x2, int y2) {
  return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

// Returns 1 if the lines intersect, otherwise 0. In addition, if the lines
// intersect the intersection point may be stored in the floats i_x and i_y.
char get_line_intersection(float p0_x,
                           float p0_y,
                           float p1_x,
                           float p1_y,
                           float p2_x,
                           float p2_y,
                           float p3_x,
                           float p3_y,
                           float* i_x,
                           float* i_y) {
  float s1_x, s1_y, s2_x, s2_y;
  s1_x = p1_x - p0_x;
  s1_y = p1_y - p0_y;
  s2_x = p3_x - p2_x;
  s2_y = p3_y - p2_y;

  float s, t;
  s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y)) /
      (-s2_x * s1_y + s1_x * s2_y);
  t = (s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) /
      (-s2_x * s1_y + s1_x * s2_y);

  if (s >= 0 && s <= 1 && t >= 0 && t <= 1) {
    // Collision detected
    if (i_x != NULL)
      *i_x = p0_x + (t * s1_x);
    if (i_y != NULL)
      *i_y = p0_y + (t * s1_y);
    return 1;
  }

  return 0;  // No collision
}

// Gen shapes
void generate_shapes() {
  // Setup all boxes randomly
  for (int i = 0; i < NUM_POLYS; i++) {
    boxes[i].width = random_int(50, 400);
    boxes[i].height = random_int(50, 400);
    boxes[i].x = random_int(0, SCREEN_W - boxes[i].width);
    boxes[i].y = random_int(0, SCREEN_H - boxes[i].height);

    // Sides
    for (int t = 0; t < 4; t++) {
      boxes[i].points[t].x = random_int(0, boxes[i].width);
      boxes[i].points[t].y = random_int(0, boxes[i].height);
    }

    // Make a shape
    make_shape(boxes[i].points, boxes[i].sides, 4);
  }
}

// Initilize game
int init() {
  printf("Setting up\n");

  // Setup SDL
  if (SDL_Init(SDL_INIT_VIDEO)) {
    printf("Could not init sdl: %s\n", SDL_GetError());
    return 1;
  }

  // Create window
  window = SDL_CreateWindow("Raycaster", SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED, SCREEN_W, SCREEN_H,
                            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

  if (window == NULL) {
    printf("Could not create window: %s\n", SDL_GetError());
    return 1;
  }

  // Create buffer
  buffer = SDL_CreateRenderer(window, -1, 0);

  // Set ellipse position
  ellipse_x = SCREEN_W / 2;
  ellipse_y = SCREEN_H / 2;

  // Generate shapes
  generate_shapes();

  // Ray count
  number_of_rays = 200;

  // Init keyboard state
  key = SDL_GetKeyboardState(NULL);

  printf("Ready!\n");

  // Success
  return 0;
}

// Update game
void update() {
  // Update keyboard state
  SDL_PumpEvents();
  key = SDL_GetKeyboardState(NULL);

  // Move our little friend
  if (key[SDL_SCANCODE_UP])
    ellipse_y -= 2;
  if (key[SDL_SCANCODE_DOWN])
    ellipse_y += 2;
  if (key[SDL_SCANCODE_LEFT])
    ellipse_x -= 2;
  if (key[SDL_SCANCODE_RIGHT])
    ellipse_x += 2;

  // Click move
  int m_x, m_y;
  SDL_PumpEvents();
  int m_b = SDL_GetMouseState(&m_x, &m_y);
  if (m_b & SDL_BUTTON(SDL_BUTTON_LEFT)) {
    ellipse_x = m_x;
    ellipse_y = m_y;
  }

  // Respawn shapes
  if (key[SDL_SCANCODE_R]) {
    generate_shapes();
  }

  // Change number of rays
  if (key[SDL_SCANCODE_KP_PLUS] || key[SDL_SCANCODE_EQUALS]) {
    number_of_rays++;
  } else if (key[SDL_SCANCODE_MINUS] || key[SDL_SCANCODE_KP_MINUS]) {
    if (number_of_rays > 0) {
      number_of_rays--;
    }
  }
}

// Draw to screen
void draw() {
  // Get mouse position
  int m_x, m_y;
  SDL_GetMouseState(&m_x, &m_y);

  // Clear screen
  SDL_SetRenderDrawColor(buffer, C_BLACK);
  SDL_RenderClear(buffer);

  // Boxes
  if (key[SDL_SCANCODE_SPACE]) {
    for (int i = 0; i < NUM_POLYS; i++) {
      for (int t = 0; t < 4; t++) {
        SDL_SetRenderDrawColor(buffer, C_WHITE);
        SDL_RenderDrawLine(buffer, boxes[i].sides[t].p1.x + boxes[i].x,
                           boxes[i].sides[t].p1.y + boxes[i].y,
                           boxes[i].sides[t].p2.x + boxes[i].x,
                           boxes[i].sides[t].p2.y + boxes[i].y);
      }
    }
  }

  // Check collision with all boxes!
  for (double q = 0; q < 2 * M_PI; q += ((2 * M_PI) / number_of_rays)) {
    float point_x = (SCREEN_W * 2) * cos(q) + ellipse_x;
    float point_y = (SCREEN_W * 2) * sin(q) + ellipse_y;

    float poi_x = point_x;
    float poi_y = point_y;

    bool intersection_found = false;

    for (int i = 0; i < NUM_POLYS; i++) {
      for (int t = 0; t < 4; t++) {
        float temp_poi_x = -1;
        float temp_poi_y = -1;

        // Check if ray and side intersect
        if (get_line_intersection(ellipse_x, ellipse_y, point_x, point_y,
                                  boxes[i].sides[t].p1.x + boxes[i].x,
                                  boxes[i].sides[t].p1.y + boxes[i].y,
                                  boxes[i].sides[t].p2.x + boxes[i].x,
                                  boxes[i].sides[t].p2.y + boxes[i].y,
                                  &temp_poi_x, &temp_poi_y)) {
          // Check if closer match found
          if (!intersection_found ||
              distance_to_2d(temp_poi_x, temp_poi_y, ellipse_x, ellipse_y) <
                  distance_to_2d(poi_x, poi_y, ellipse_x, ellipse_y)) {
            poi_x = temp_poi_x;
            poi_y = temp_poi_y;
            intersection_found = true;
          }
        }
      }
    }

    // Draw line to closest collision
    SDL_SetRenderDrawColor(buffer, C_WHITE);
    SDL_RenderDrawLine(buffer, ellipse_x, ellipse_y, poi_x, poi_y);

    // Draw intersection if there is one
    if (intersection_found && key[SDL_SCANCODE_SPACE]) {
      SDL_SetRenderDrawColor(buffer, C_RED);
      SDL_Rect collision = {.x = poi_x, .y = poi_y, .w = 5, .h = 5};
      SDL_RenderFillRect(buffer, &collision);
    }
  }

  // Player
  SDL_SetRenderDrawColor(buffer, C_WHITE);
  SDL_Rect player = {.x = ellipse_x, .y = ellipse_y, .w = 10, .h = 10};
  SDL_RenderFillRect(buffer, &player);

  // Mouse (makes a cursor from primatives)
  SDL_SetRenderDrawColor(buffer, C_WHITE);
  SDL_RenderDrawLine(buffer, m_x, m_y, m_x + 20, m_y + 10);
  SDL_RenderDrawLine(buffer, m_x, m_y, m_x + 10, m_y + 20);
  SDL_RenderDrawLine(buffer, m_x + 10, m_y + 20, m_x + 12, m_y + 12);
  SDL_RenderDrawLine(buffer, m_x + 20, m_y + 10, m_x + 12, m_y + 12);

  // Text telling help
  // textprintf_ex(buffer, font, 20, SCREEN_H - 100, C_WHITE, C_WHITE,
  //               "[R] Respawn shapes");
  // textprintf_ex(buffer, font, 20, SCREEN_H - 80, C_WHITE, C_WHITE,
  //               "[SPACE] Show shapes and colliders");
  // textprintf_ex(buffer, font, 20, SCREEN_H - 60, C_WHITE, C_WHITE,
  //               "[MOUSE CLICK/ARROW KEYS] Move ellipse (%.0f,%.0f)",
  //               ellipse_x, ellipse_y);
  // textprintf_ex(buffer, font, 20, SCREEN_H - 40, C_WHITE, C_WHITE,
  //               "[+/-] Number of rays (%i)", number_of_rays);

  // Draw buffer
  SDL_RenderPresent(buffer);
}

void loop() {
  SDL_Event e;

  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT) {
      quit = true;
    } else {
      update();
      draw();
    }
  }
}

// Main
int main(int argc, char* argv[]) {
  // Initilize game
  if (init()) {
    return 1;
  }

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(loop, 60, true);
#else
  while (!quit) {
    loop();
  }
#endif

  // Exit
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
