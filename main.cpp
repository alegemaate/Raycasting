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

// Point
typedef struct{
  int x;
  int y;
} prim_point;

// Line
typedef struct{
  prim_point p1;
  prim_point p2;
} prim_line;

// Boxes
typedef struct{
  int x;
  int y;
  int width;
  int height;

  prim_line sides[4];
  prim_point points[4];
} box;

#define NUM_BOXES 20

box boxes[NUM_BOXES];

// Random function
int random( int min, int max){
  int random_number = (rand() % (max - min)) + min;
  return random_number;
}

// Make shape
void make_shape( prim_point *_points, prim_line *_lines, int _length){
  for( int i = 0; i < _length; i++){
    _lines[i].p1.x = _points[i].x;
    _lines[i].p1.y = _points[i].y;

    if( i < _length - 1){
      _lines[i].p2.x = _points[i + 1].x;
      _lines[i].p2.y = _points[i + 1].y;
    }
    else{
      _lines[i].p2.x = _points[0].x;
      _lines[i].p2.y = _points[0].y;
    }
  }
}

// Init lines
void set_line( int _x1, int _y1, int _x2, int _y2, prim_line *_line){
  _line -> p1.x = _x1;
  _line -> p1.y = _y1;
  _line -> p2.x = _x2;
  _line -> p2.y = _y2;
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
    boxes[i].width = random( 50, 400);
    boxes[i].height = random( 50, 400);
    boxes[i].x = random( 0, SCREEN_W - boxes[i].width);
    boxes[i].y = random( 0, SCREEN_H - boxes[i].height);

    // Sides
    for( int t = 0; t < 4; t++){
      boxes[i].points[t].x = random( 0, boxes[i].width);
      boxes[i].points[t].y = random( 0, boxes[i].height);
    }

    // Make a shape
    make_shape( boxes[i].points, boxes[i].sides, 4);
  }

  // Collision stuff
  poi_x = 0;
  poi_y = 0;
  intersection_found = false;
}

// Update game
void update(){
  // Check collision with all boxes!
  for( double q = 0; q < 2 * M_PI; q += 0.01){
    float point_x = (SCREEN_W * 2) * cos(q) + ellipse_x;
    float point_y = (SCREEN_W * 2) * sin(q) + ellipse_y;

    poi_x = point_x;
    poi_y = point_y;

    intersection_found = false;

    for( int i = 0; i < NUM_BOXES; i++){
      for( int t = 0; t < 4; t++){
        float temp_poi_x = -1;
        float temp_poi_y = -1;

        // Check if ray and side intersect
        if( get_line_intersection( ellipse_x, ellipse_y, point_x, point_y,
                                   boxes[i].sides[t].p1.x + boxes[i].x, boxes[i].sides[t].p1.y + boxes[i].y,
                                   boxes[i].sides[t].p2.x + boxes[i].x, boxes[i].sides[t].p2.y + boxes[i].y,
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
    line( ray_buffer, ellipse_x, ellipse_y, poi_x, poi_y, makecol( 255, 255, 255));

    // Draw intersection if there is one
    if( intersection_found && key[KEY_SPACE]){
      ellipse( ray_buffer, poi_x, poi_y, 5, 5, makecol( 255, 0, 0));
    }
  }

  // Move our little friend
  if( key[KEY_UP]){
    ellipse_y -= 2;
  }
  if( key[KEY_DOWN]){
    ellipse_y += 2;
  }
  if( key[KEY_LEFT]){
    ellipse_x -= 2;
  }
  if( key[KEY_RIGHT]){
    ellipse_x += 2;
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
  rectfill( buffer, 0, 0, SCREEN_W, SCREEN_H, makecol( 0, 0, 0));

  // Boxes
  if( key[KEY_SPACE]){
    for( int i = 0; i < NUM_BOXES; i++){
      for( int t = 0; t < 4; t++){
        line( buffer, boxes[i].sides[t].p1.x + boxes[i].x, boxes[i].sides[t].p1.y + boxes[i].y,
                      boxes[i].sides[t].p2.x + boxes[i].x, boxes[i].sides[t].p2.y + boxes[i].y, makecol( 255, 255, 255));
      }
    }
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
