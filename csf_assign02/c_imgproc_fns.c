// C implementations of image processing functions

#include <stdlib.h>
#include <assert.h>
#include "imgproc.h"
#include <stdio.h>

// Get the red value from pixel
uint32_t get_r( uint32_t pixel ) { return (pixel & 0xFF000000) >> 24; }

// Get the green value from pixel
uint32_t get_g( uint32_t pixel ) { return (pixel & 0x00FF0000) >> 16; }

// Get the blue value from pixel
uint32_t get_b( uint32_t pixel ) { return (pixel & 0x0000FF00) >> 8; }

// Get the alpha value from pixel
uint32_t get_a( uint32_t pixel ) { return pixel & 0x000000FF; }

// Make pixel from rgba components
uint32_t make_pixel( uint32_t r, uint32_t g, uint32_t b, uint32_t a ) {
  return (r << 24) | (g << 16) | (b << 8) | a;
}

// Convert pixel to grayscale
uint32_t to_grayscale( uint32_t pixel ) {
  uint32_t r = get_r(pixel);
  uint32_t g = get_g(pixel);
  uint32_t b = get_b(pixel);
  uint32_t y = ( 79 * r + 128 * g + 49 * b ) / 256;
  return make_pixel( y, y, y, get_a(pixel) );
}

// Compute the gradient needed for pixel at index `x`
int64_t gradient( int64_t x, int64_t max ) {
  int64_t square = ((2000000000LL * x) / (1000000LL * max) - 1000);
  return 1000000LL - square * square;
}

// Compute the 1-dimensional index from column and row indices
int32_t compute_index( struct Image *img, int32_t col, int32_t row ) {
  return row * img->width + col;
}

// Convert input pixels to grayscale.
// This transformation always succeeds.
//
// Parameters:
//   input_img  - pointer to the input Image
//   output_img - pointer to the output Image (in which the transformed
//                pixels should be stored)
void imgproc_grayscale( struct Image *input_img, struct Image *output_img ) {
  for (int i=0;i<input_img->height;i++)
    for (int j=0;j<input_img->width;j++) {
      int input_index = compute_index(input_img, j, i);
      uint32_t pixel = input_img->data[input_index];
      int output_index = compute_index(output_img, j, i);
      output_img->data[output_index] = to_grayscale(pixel);
    }
}

// Render an output image containing 4 replicas of the original image,
// refered to as A, B, C, and D in the following diagram:
//
//   +---+---+
//   | A | B |
//   +---+---+
//   | C | D |
//   +---+---+
//
// The width and height of the output image are (respectively) twice
// the width and height of the input image.
//
// A is an exact copy of the original input image. B has only the
// red color component values of the input image, C has only the
// green color component values, and D has only the blue color component
// values.
//
// Each of the copies (A-D) should use the same alpha values as the
// original image.
//
// This transformation always succeeds.
//
// Parameters:
//   input_img - pointer to the input Image
//   output_img - pointer to the output Image (which will have
//                width and height twice the width/height of the
//                input image)
void imgproc_rgb( struct Image *input_img, struct Image *output_img ) {
  for (int i=0;i<input_img->height;i++)
    for (int j=0;j<input_img->width;j++) {
      int input_index = compute_index(input_img, j, i);
      uint32_t pixel = input_img->data[input_index];

      int copy_index = compute_index( output_img, j,                      i                       );
      int red_index  = compute_index( output_img, (input_img->width + j), i                       );
      int green_index= compute_index( output_img, j,                      (input_img->height + i) );
      int blue_index = compute_index( output_img, (input_img->width + j), (input_img->height + i) );
      
      output_img->data[copy_index] =  pixel;
      output_img->data[red_index]  =  make_pixel( get_r(pixel), 0, 0, get_a(pixel) );
      output_img->data[green_index]=  make_pixel( 0, get_g(pixel), 0, get_a(pixel) );
      output_img->data[blue_index] =  make_pixel( 0, 0, get_b(pixel), get_a(pixel) );
    }
}

// Render a "faded" version of the input image.
//
// See the assignment description for an explanation of how this transformation
// should work.
//
// This transformation always succeeds.
//
// Parameters:
//   input_img - pointer to the input Image
//   output_img - pointer to the output Image
void imgproc_fade( struct Image *input_img, struct Image *output_img ) {
  int64_t width = input_img->width;
  int64_t height = input_img->height;

  for (int64_t row = 0; row < height; row++){
    int64_t row_fade = gradient(row, height);

    for (int64_t col = 0; col < width; col++){
      int64_t col_fade = gradient(col, width);

      int32_t idx = compute_index(input_img, col, row);
      uint32_t pixel = input_img->data[idx];

      uint32_t r = get_r(pixel);
      uint32_t g = get_g(pixel);
      uint32_t b = get_b(pixel);
      uint32_t a = get_a(pixel);

      uint32_t r_faded = (row_fade * col_fade * r)/1000000000000LL;
      uint32_t g_faded = (row_fade * col_fade * g)/1000000000000LL;
      uint32_t b_faded = (row_fade * col_fade * b)/1000000000000LL;
      uint32_t a_faded = a;

      if (r_faded > 255) r_faded = 255;
      if (g_faded > 255) g_faded = 255;
      if (b_faded > 255) b_faded = 255;
      output_img->data[idx] = make_pixel(r_faded, g_faded, b_faded, a_faded);
  
    }
  }

  
}

// Render a "kaleidoscope" transformation of input_img in output_img.
// The input_img must be square, i.e., the width and height must be
// the same. Assume that the input image is divided into 8 "wedges"
// like this:
//
//    +----+----+
//    |\ A |   /|
//    | \  |  / |
//    |  \ | /  |
//    | B \|/   |
//    +----+----+
//    |   /|\   |
//    |  / | \  |
//    | /  |  \ |
//    |/   |   \|
//    +----+----+
//
// The wedge labeled "A" is the one that will be replicated 8 times
// in the output image. In the output image,
//
//    Wedge A will have exactly the same pixels as wedge A in
//    the input image, in the same positions.
//
//    Wedge B will have exactly the same pixels as wedge A, but the
//    column and row are transposed. (I.e., it is a reflection across
//    the diagonal.)
//
//    All other wedges are mirrored versions of A such that the overall
//    image is symmetrical across all four lines dividing the image
//    (vertical split, horizontal split, and the two diagonal splits.)
//
// Parameters:
//   input_img  - pointer to the input Image
//   output_img - pointer to the output Image (in which the transformed
//                pixels should be stored)
//
// Returns:
//   1 if successful, 0 if the transformation fails because the
//   width and height of input_img are not the same.
int imgproc_kaleidoscope( struct Image *input_img, struct Image *output_img ) {
  int32_t width = input_img->width;
  int32_t height = input_img->height;
  if (width != height) return 0;

  int32_t half = ((width + 1) >> 1);

  // top left  
  for (int32_t i = 0; i < half; i++){
    for (int32_t j = i; j < half; j++){
      int32_t idx_original  = compute_index(input_img, j, i);
      int32_t idx_copy      = compute_index(output_img, j, i);
      int32_t idx_reflect   = compute_index(output_img, i, j);
      output_img->data[idx_copy]    = input_img->data[idx_original];
      output_img->data[idx_reflect] = input_img->data[idx_original]; 
    }
  }

  for (int32_t i = 0; i < half; i++){
    for (int32_t j = 0; j < half; j++){
      int32_t idx_original      = compute_index(output_img, j, i);
      int32_t idx_top_right     = compute_index(output_img, (width - 1 - j), i);
      int32_t idx_bottom_left   = compute_index(output_img, j, (height - 1 - i));
      int32_t idx_bottom_right  = compute_index(output_img, (width - 1 - j), (height - 1 - i));

      output_img->data[idx_top_right] = output_img->data[idx_original];
      output_img->data[idx_bottom_left] = output_img->data[idx_original];
      output_img->data[idx_bottom_right] = output_img->data[idx_original];
    }
  }

  return 1;
}
