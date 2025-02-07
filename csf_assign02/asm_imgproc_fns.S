/*
 * x86-64 assembly language implementations of functions
 */

	.section .text

/* Offsets of struct Image fields */
#define IMAGE_WIDTH_OFFSET   0
#define IMAGE_HEIGHT_OFFSET  4
#define IMAGE_DATA_OFFSET    8

/*
 * Helper functions.
 */

/*
 * Get the red value from pixel (bits 31-24).
 * 
 * Parameters:
 *   %edi - input pixel (32 bits)
 */
  .global get_r
get_r:
  movl %edi, %eax         # Move input pixel to return register
  andl $0xFF000000, %eax  # Extract the red value in-place
  shrl $24, %eax          # Shift right 24 bits to remove trailing 0s
  ret

/*
 * Get the green value from pixel (bits 23-16).
 * 
 * Parameters:
 *   %edi - input pixel (32 bits)
 */
  .global get_g
get_g:
  movl %edi, %eax         # Move input pixel to return register
  andl $0x00FF0000, %eax  # Extract the green value in-place
  shrl $16, %eax          # Shift right 16 bits to remove trailing 0s
  ret

/*
 * Get the blue value from pixel (bits 15-8).
 * 
 * Parameters:
 *   %edi - input pixel (32 bits)
 */
  .global get_b
get_b:
  movl %edi, %eax         # Move input pixel to return register
  andl $0x0000FF00, %eax  # Extract the blue value in-place
  shrl $8, %eax           # Shift right 8 bits to remove trailing 0s
  ret

/*
 * Get the alpha value from pixel (bits 7-0).
 * 
 * Parameters:
 *   %edi - input pixel (32 bits)
 */
  .global get_a
get_a:
  movl %edi, %eax         # Move input pixel to return register
  andl $0x000000FF, %eax  # Extract the alpha value in-place
  ret

/*
 * Make pixel from rgba components.
 * 
 * Parameters:
 *   %edi - red value
 *   %esi - green value
 *   %edx - blue value
 *   %ecx - alpha value
 */
  .global make_pixel
make_pixel:
  movl %edi, %eax   # Move red value to return register
  shll $8, %eax     # Shift left 8 bits to make space for green value
  orl %esi, %eax    # Add green value to return register
  shll $8, %eax     # Shift left 8 bits again to make space for blue value
  orl %edx, %eax    # Add blue value to return register
  shll $8, %eax     # Shift left 8 bits again to make space for alpha value
  orl %ecx, %eax    # Add alpha value to return register
  ret

/*
 * Convert pixel to grayscale.
 * 
 * Parameters:
 *   %edi - input pixel
 */
  .global to_grayscale
to_grayscale:
  pushq %r12        # Preserve r12-15 callee-save registers on stack
  pushq %r13
  pushq %r14
  pushq %r15
  
  movl %edi, %r15d  # Save original pixel

  call get_r        # Call get_r on input pixel
  movl %eax, %r12d  # Store return value in r12

  movl %r15d, %edi  # Restore input pixel from save
  call get_g        # Call get_g on input pixel
  movl %eax, %r13d  # Store return value in r13

  movl %r15d, %edi  # Restore input pixel from save
  call get_b        # Call get_b on input pixel
  movl %eax, %r14d  # Store return value in r14

  movl %r15d, %edi  # Restore input pixel from save 
  call get_a        # Call get_a on input pixel
  movl %eax, %r15d  # Store return value in r15 (override input pixel save)

  imull $49, %r12d  # Multiply red value by 49
  imull $128, %r13d # Multiply green value by 128
  imull $79, %r14d  # Multiply blue value by 79

  movl $0, %r10d    # Init r10 to 0
  addl %r12d, %r10d # Make rgb weighted sum
  addl %r13d, %r10d
  addl %r14d, %r10d
  shrl $8, %r10d    # Shift right 8 bits (divide by 256)

  movl %r10d, %edi  # Put grayscale rgb values in first three params
  movl %r10d, %esi
  movl %r10d, %edx
  movl %r15d, %ecx  # Put original alpha value
  call make_pixel   # Make return pixel

  popq %r12         # Restore r12-15 callee-save registers
  popq %r13
  popq %r14
  popq %r15

  ret

/*
 * Compute the gradient needed for pixel at index.
 * 
 * Parameters:
 *   %rdi - index to calculate
 *   %rsi - max index
 */
  .global gradient
gradient:
  /* TODO: implement */
  ret

/*
 * Compute the 1-dimensional index from column and row indices.
 * 
 * Parameters:
 *   %rdi - pointer to the input Image
 *   %rsi - column index
 *   %rdx - row index
 */
  .global compute_index
compute_index:
  movq IMAGE_WIDTH_OFFSET(%rdi), %r10  # Store image width in temporary register
  movq $0, %rax                        # Init return register to 0
  addq %rdx, %rax                      # Add row index to return register
  imulq %r10, %rax                     # Multiply row index with image width
  addq %rsi, %rax                      # Add column index to return register
  ret

/*
 * Implementations of API functions
 */

/*
 * Render an output image containing 4 copies of the original image,
 * refered to as A, B, C, and D in the following diagram:
 *
 *   +---+---+
 *   | A | B |
 *   +---+---+
 *   | C | D |
 *   +---+---+
 *
 * The width and height of the output image are (respectively) twice
 * the width and height of the input image.
 *
 * A is an exact copy of the original input image. B has only the
 * red color component values of the input image, C has only the
 * green color component values, and D has only the blue color component
 * values.
 *
 * Each of the copies (A-D) should use the same alpha values as the
 * original image.
 *
 * This transformation always succeeds.
 *
 * Parameters:
 *   %rdi - pointer to the input Image
 *   %rsi - pointer to the output Image (which will have
 *          width and height twice the width/height of the
 *          input image)
 */
	.globl imgproc_rgb
imgproc_rgb:
	/* TODO: implement */
	ret

/*
 * void imgproc_grayscale( struct Image *input_img, struct Image *output_img );
 *
 * Transform image by converting each pixel to grayscale.
 * This transformation always succeeds.
 *
 * Parameters:
 *   %rdi - pointer to original struct Image
 *   %rsi - pointer to output struct Image
 */
	.globl imgproc_grayscale
imgproc_grayscale:
	/* TODO: implement */
	ret

/*
 * Render a "faded" version of the input image.
 *
 * See the assignment description for an explanation of how this transformation
 * should work.
 *
 * This transformation always succeeds.
 *
 * Parameters:
 *   %rdi - pointer to the input Image
 *   %rsi - pointer to the output Image
 */
	.globl imgproc_fade
imgproc_fade:
	/* TODO: implement */
	ret

/*
 * Render a "kaleidoscope" transformation of input_img in output_img.
 * The input_img must be square, i.e., the width and height must be
 * the same. Assume that the input image is divided into 8 "wedges"
 * like this:
 *
 *    +----+----+
 *    |\ A |   /|
 *    | \  |  / |
 *    |  \ | /  |
 *    | B \|/   |
 *    +----+----+
 *    |   /|\   |
 *    |  / | \  |
 *    | /  |  \ |
 *    |/   |   \|
 *    +----+----+
 *
 * The wedge labeled "A" is the one that will be replicated 8 times
 * in the output image. In the output image,
 *
 *    Wedge A will have exactly the same pixels as wedge A in
 *    the input image, in the same positions.
 *
 *    Wedge B will have exactly the same pixels as wedge A, but the
 *    column and row are transposed. (I.e., it is a reflection across
 *    the diagonal.)
 *
 *    All other wedges are mirrored versions of A such that the overall
 *    image is symmetrical across all four lines dividing the image
 *    (vertical split, horizontal split, and the two diagonal splits.)
 *
 * Parameters:
 *   input_img  - pointer to the input Image
 *   output_img - pointer to the output Image (in which the transformed
 *                pixels should be stored)
 *
 * Returns:
 *   1 if successful, 0 if the transformation fails because the
 *   width and height of input_img are not the same.
 */
	.globl imgproc_kaleidoscope
imgproc_kaleidoscope:
	/* TODO: implement */
	ret

	/* This avoids linker warning about executable stack */
.section .note.GNU-stack,"",@progbits

/*
vim:ft=gas:
*/
