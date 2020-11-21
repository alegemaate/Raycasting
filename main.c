#include <allegro.h>
#include <alpng.h>
#include <math.h>

#define M_PI 3.14159265358979323846264338327

// Buffer
BITMAP *buffer;
BITMAP *ray_buffer, *ray_buffer2;

// Ellipse position
float ellipse_x, ellipse_y;

// Collision stuff
float poi_x;
float poi_y;
_Bool intersection_found;

// Count for rays
int number_of_rays;

// The points of intersection
int pois[1000];

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

#define NUM_BOXES 5

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
float distanceTo2D( float x1, float y1, float x2, float y2){
  return sqrt((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1));
}

// Angle between 2 points
float angle2D( float x1, float y1, float x2, float y2){
  return atan2(y1 - y2, x1 - x2);
}

// Sort array
void sortArrayByAngle( int *unsortedArray, int length, float orgin_x, float orgin_y){
  //printf( "len:%i ox:%f oy:%f \n\n", length, orgin_x, orgin_y);

  for( int i = 0 ; i < length - 3; i += 2){
    for( int t = 0; t < length - 3; t += 2){
      // Swap them
      if( angle2D( orgin_x, orgin_y, unsortedArray[t + 0], unsortedArray[t + 1]) >
          angle2D( orgin_x, orgin_y, unsortedArray[t + 2], unsortedArray[t + 3])){
        int temp_x = unsortedArray[t + 2];
        int temp_y = unsortedArray[t + 3];

        unsortedArray[t + 2] = unsortedArray[t + 0];
        unsortedArray[t + 3] = unsortedArray[t + 1];

        unsortedArray[t + 0] = temp_x;
        unsortedArray[t + 1] = temp_y;
      }
    }
  }
}



// Returns 1 if the lines intersect, otherwise 0. In addition, if the lines
// intersect the intersection point may be stored in the floats i_x and i_y.
char get_line_intersection(float p0_x, float p0_y, float p1_x, float p1_y,
                           float p2_x, float p2_y, float p3_x, float p3_y,
                           float *i_x, float *i_y){
    // Slopes
    float s1_x, s1_y, s2_x, s2_y;
    s1_x = p1_x - p0_x;     s1_y = p1_y - p0_y;
    s2_x = p3_x - p2_x;     s2_y = p3_y - p2_y;

    // Check for poi
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

// Gen shapes
void generate_shapes(){
  // Setup all boxes randomly
  for( int i = 0; i < NUM_BOXES; i++){
    // Box 1 is outer wall
    if( i == 0){
      boxes[i].width = SCREEN_W;
      boxes[i].height = SCREEN_H;
      boxes[i].x = 0;
      boxes[i].y = 0;

      // Sides
      for( int t = 0; t < 4; t++){
        boxes[i].points[t].x = (((int)ceil((double)t/2)) % 2) * SCREEN_W;
        boxes[i].points[t].y = (int)floor(t/2) * SCREEN_H;
      }

    }
    else{
      boxes[i].width = random( 50, 400);
      boxes[i].height = random( 50, 400);
      boxes[i].x = random( 0, SCREEN_W - boxes[i].width);
      boxes[i].y = random( 0, SCREEN_H - boxes[i].height);

      // Sides
      for( int t = 0; t < 4; t++){
        boxes[i].points[t].x = random( 0, boxes[i].width);
        boxes[i].points[t].y = random( 0, boxes[i].height);
      }
    }

    // Make a shape
    make_shape( boxes[i].points, boxes[i].sides, 4);
  }
}

// Initilize game
void init(){
  allegro_init();
  alpng_init();
  install_keyboard();
  install_mouse();

  install_sound(DIGI_AUTODETECT,MIDI_AUTODETECT,".");
  set_color_depth( 32);
  set_window_title("Raycaster");
  set_gfx_mode( GFX_AUTODETECT_WINDOWED, 1280, 960, 0, 0);

  // Create buffer
  buffer = create_bitmap( SCREEN_W, SCREEN_H);
  ray_buffer = create_bitmap( SCREEN_W, SCREEN_H);

  // Set ellipse position
  ellipse_x = SCREEN_W / 2;
  ellipse_y = SCREEN_H / 2;

  // Generate shapes
  generate_shapes();

  // Collision stuff
  poi_x = 0;
  poi_y = 0;
  intersection_found = FALSE;

  // Ray count
  number_of_rays = 200;
}

// Update game
void update(){
  // Make a ray to each point
  int point_count = 0;
  for( int i = 0; i < NUM_BOXES; i++){
    for( int t = 0; t < 4; t++){
      // + or - amount
      for( float q = -2; q < 3; q++){
        float point_x = boxes[i].points[t].x + boxes[i].x;
        float point_y = boxes[i].points[t].y + boxes[i].y;

        // New point with slightly adjusted angle
        float point_angle = angle2D( ellipse_x, ellipse_y, boxes[i].points[t].x + boxes[i].x, boxes[i].points[t].y + boxes[i].y);

        // Extend line
        point_x = point_x + cos(point_angle + q/100) * -1000;
        point_y = point_y + sin(point_angle + q/100) * -1000;

        poi_x = point_x;
        poi_y = point_y;

        // Check if there is a closer collision
        for( int j = 0; j < NUM_BOXES; j++){
          for( int k = 0; k < 4; k++){
            float temp_poi_x = -1;
            float temp_poi_y = -1;

            // Check if ray and side intersect
            if( get_line_intersection( ellipse_x, ellipse_y, point_x, point_y,
                                       boxes[j].sides[k].p1.x + boxes[j].x, boxes[j].sides[k].p1.y + boxes[j].y,
                                       boxes[j].sides[k].p2.x + boxes[j].x, boxes[j].sides[k].p2.y + boxes[j].y,
                                       &temp_poi_x, &temp_poi_y)){
              // Check if closer match found
              if( distanceTo2D( temp_poi_x, temp_poi_y, ellipse_x, ellipse_y) < distanceTo2D( poi_x, poi_y, ellipse_x, ellipse_y)){
                poi_x = temp_poi_x;
                poi_y = temp_poi_y;
                intersection_found = TRUE;
              }
            }
          }
        }

        if( intersection_found){
          // Add to vertex buffer
          pois[point_count] = poi_x;
          pois[point_count + 1] = poi_y;
          point_count += 2;

          // Draw line to closest collision
          line( ray_buffer, ellipse_x, ellipse_y, poi_x, poi_y, makecol( 255, 255, 255));

          // Draw intersection if there is one
          if( intersection_found && key[KEY_SPACE]){
            ellipse( ray_buffer, poi_x, poi_y, 5, 5, makecol( 255, 0, 0));
          }
        }
      }
    }
  }


  // Check collision with all boxes!
  /*int point_count = 0;
  for( double q = 0; q < 2 * M_PI; q += ((2 * M_PI) / number_of_rays)){
    float point_x = (SCREEN_W * 2) * cos(q) + ellipse_x;
    float point_y = (SCREEN_W * 2) * sin(q) + ellipse_y;

    poi_x = point_x;
    poi_y = point_y;

    intersection_found = FALSE;

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
            intersection_found = TRUE;
          }
        }
      }
    }

    // Add to vertex buffer
    pois[point_count] = poi_x;
    pois[point_count + 1] = poi_y;
    point_count += 2;

    // Draw line to closest collision
    line( ray_buffer, ellipse_x, ellipse_y, poi_x, poi_y, makecol( 255, 255, 255));

    // Draw intersection if there is one
    if( intersection_found && key[KEY_SPACE]){
      ellipse( ray_buffer, poi_x, poi_y, 5, 5, makecol( 255, 0, 0));
    }
  }*/

  // Sort poi array
  sortArrayByAngle( pois, point_count, ellipse_x, ellipse_y);

  // Draw a large polygon surrounding the pois
  polygon( ray_buffer, (point_count / 2), pois, makecol( 255, 0, 0));

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

  // Respawn shapes
  if( key[KEY_R]){
    generate_shapes();
  }

  // Change number of rays
  if( key[KEY_PLUS_PAD])
    number_of_rays ++;
  else if( key[KEY_MINUS_PAD]){
    if( number_of_rays > 0)
      number_of_rays --;
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

  // Text telling help
  textprintf_ex( buffer, font, 20, SCREEN_H - 100, makecol( 255, 255, 255), makecol( 0, 0, 0), "[R] Respawn shapes");
  textprintf_ex( buffer, font, 20, SCREEN_H - 80, makecol( 255, 255, 255), makecol( 0, 0, 0), "[SPACE] Show shapes and colliders");
  textprintf_ex( buffer, font, 20, SCREEN_H - 60, makecol( 255, 255, 255), makecol( 0, 0, 0), "[MOUSE CLICK/ARROW KEYS] Move ellipse (%.0f,%.0f)", ellipse_x, ellipse_y);
  textprintf_ex( buffer, font, 20, SCREEN_H - 40, makecol( 255, 255, 255), makecol( 0, 0, 0), "[+/-] Number of rays (%i)", number_of_rays);

  // Draw buffer
  draw_sprite( screen, buffer, 0, 0);
}

// Main
int main(){
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
