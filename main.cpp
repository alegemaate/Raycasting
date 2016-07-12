#include <allegro.h>
#include <alpng.h>
#include <math.h>

// Buffer
BITMAP *buffer;
BITMAP *ray_buffer;

// Ellipse position
float ellipse_x, ellipse_y;

// Collision stuff
float poi_x;
float poi_y;
bool intersection_found;

// Line
typedef struct{
  int x1;
  int y1;

  int x2;
  int y2;
} prim_line;

// Boxes
typedef struct{
  int x;
  int y;
  int width;
  int height;

  prim_line sides[4];
} box;

#define NUM_BOXES 20

box boxes[NUM_BOXES];

// Random function
int random( int min, int max){
  int random_number = (rand() % (max - min)) + min;
  return random_number;
}

// Init lines
void set_line( int _x1, int _y1, int _x2, int _y2, prim_line *_line){
  _line -> x1 = _x1;
  _line -> y1 = _y1;
  _line -> x2 = _x2;
  _line -> y2 = _y2;
}

// Returns distance 2D
int distanceTo2D(int x1, int y1, int x2, int y2){
  return sqrt((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1));
}

// Returns 1 if the lines intersect, otherwise 0. In addition, if the lines
// intersect the intersection point may be stored in the floats i_x and i_y.
char get_line_intersection(float p0_x, float p0_y, float p1_x, float p1_y,
                           float p2_x, float p2_y, float p3_x, float p3_y,
                           float *i_x, float *i_y){
    float s1_x, s1_y, s2_x, s2_y;
    s1_x = p1_x - p0_x;     s1_y = p1_y - p0_y;
    s2_x = p3_x - p2_x;     s2_y = p3_y - p2_y;

    float s, t;
    s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y)) / (-s2_x * s1_y + s1_x * s2_y);
    t = ( s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) / (-s2_x * s1_y + s1_x * s2_y);

    if (s >= 0 && s <= 1 && t >= 0 && t <= 1){
        // Collision detected
        if (i_x != NULL)
          *i_x = p0_x + (t * s1_x);
        if (i_y != NULL)
          *i_y = p0_y + (t * s1_y);
        return 1;
    }

    return 0; // No collision
}

// Initilize game
void init(){
  allegro_init();
  alpng_init();
  install_keyboard();
  install_mouse();

  install_sound(DIGI_AUTODETECT,MIDI_AUTODETECT,".");
  set_color_depth( 32);
  set_window_title("Forager");
  set_gfx_mode( GFX_AUTODETECT_WINDOWED, 1280, 960, 0, 0);

  // Create buffer
  buffer = create_bitmap( SCREEN_W, SCREEN_H);
  ray_buffer = create_bitmap( SCREEN_W, SCREEN_H);

  // Set ellipse position
  ellipse_x = SCREEN_W / 2;
  ellipse_y = SCREEN_H / 2;

  // Setup all boxes randomly
  for( int i = 0; i < NUM_BOXES; i++){
    boxes[i].x = random( 0, SCREEN_W);
    boxes[i].y = random( 0, SCREEN_H);
    boxes[i].width = random( 10, 100);
    boxes[i].height = random( 10, 100);

    // Sides
    // Top
    set_line( 0, 0, boxes[i].width, 0, &boxes[i].sides[0]);
    set_line( 0, boxes[i].height, boxes[i].width, boxes[i].height, &boxes[i].sides[1]);
    set_line( 0, 0, 0, boxes[i].height, &boxes[i].sides[2]);
    set_line( boxes[i].width, 0, boxes[i].width, boxes[i].height, &boxes[i].sides[3]);
  }

  // Collision stuff
  poi_x = 0;
  poi_y = 0;
  intersection_found = false;
}

// Update game
void update(){
  // Reset collision vars
  poi_x = mouse_x;
  poi_y = mouse_y;
  intersection_found = false;

  // Check collision with all boxes!
  for( double q = 0; q < 2 * M_PI; q += 0.1){
    float point_x = 400 * cos(q) + ellipse_x;
    float point_y = 400 * sin(q) + ellipse_y;

    poi_x = point_x;
    poi_y = point_y;

    for( int i = 0; i < NUM_BOXES; i++){
      for( int t = 0; t < 4; t++){
        float temp_poi_x = -1;
        float temp_poi_y = -1;

        // Check if ray and side intersect
        if( get_line_intersection( ellipse_x, ellipse_y, point_x, point_y,
                                   boxes[i].sides[t].x1 + boxes[i].x, boxes[i].sides[t].y1 + boxes[i].y,
                                   boxes[i].sides[t].x2 + boxes[i].x, boxes[i].sides[t].y2 + boxes[i].y,
                                   &temp_poi_x, &temp_poi_y)){
          // Check if closer match found
          if( !intersection_found || distanceTo2D( temp_poi_x, temp_poi_y, ellipse_x, ellipse_y) < distanceTo2D( poi_x, poi_y, ellipse_x, ellipse_y)){
            poi_x = temp_poi_x;
            poi_y = temp_poi_y;
            intersection_found = true;
          }
        }
      }
    }

    // Draw line to closest collision
    line( ray_buffer, ellipse_x, ellipse_y, poi_x, poi_y, makecol( 0, 0, 0));
  }

  // Move our little friend
  if( key[KEY_UP]){
    ellipse_y -= 0.5;
  }
  if( key[KEY_DOWN]){
    ellipse_y += 0.5;
  }
  if( key[KEY_LEFT]){
    ellipse_x -= 0.5;
  }
  if( key[KEY_RIGHT]){
    ellipse_x += 0.5;
  }

  // Click move
  if( mouse_b & 1){
    ellipse_x = mouse_x;
    ellipse_y = mouse_y;
  }
}

// Draw to screen
void draw(){
  // Clear screen
  rectfill( buffer, 0, 0, SCREEN_W, SCREEN_H, makecol( 255, 255, 255));

  // Boxes
  for( int i = 0; i < NUM_BOXES; i++){
    rect( buffer, boxes[i].x, boxes[i].y, boxes[i].x + boxes[i].width, boxes[i].y + boxes[i].height, makecol( 0, 0, 0));
  }

  // Ray buffer
  draw_sprite( buffer, ray_buffer, 0, 0);
  rectfill( ray_buffer, 0, 0, SCREEN_W, SCREEN_H, makecol( 255, 0, 255));

  // Player
  ellipse( buffer, ellipse_x, ellipse_y, 10, 10, makecol( 0, 0, 0));

  // Mouse ( makes a cursor from primatives
  line( buffer, mouse_x, mouse_y, mouse_x + 20, mouse_y + 10, makecol( 0, 0, 0));
  line( buffer, mouse_x, mouse_y, mouse_x + 10, mouse_y + 20, makecol( 0, 0, 0));
  line( buffer, mouse_x + 10, mouse_y + 20, mouse_x + 12, mouse_y + 12, makecol( 0, 0, 0));
  line( buffer, mouse_x + 20, mouse_y + 10, mouse_x + 12, mouse_y + 12, makecol( 0, 0, 0));

  // Draw intersection ( if one found)
  /*if( intersection_found){
    ellipse( buffer, poi_x, poi_y, 5, 5, makecol( 255, 0, 0));
  }

  line( buffer, ellipse_x, ellipse_y, poi_x, poi_y, makecol( 0, 0, 0));*/

  // Draw buffer
  draw_sprite( screen, buffer, 0, 0);
}

// Main
int main( int argc, char* args[]){
  // Initilize game
  init();

  // Loop until exit
  while( !key[KEY_ESC]){
    update();
    draw();
  }

  // Exit
  allegro_exit();
  return 0;
}
END_OF_MAIN();
