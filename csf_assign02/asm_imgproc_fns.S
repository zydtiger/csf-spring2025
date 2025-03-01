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
  .globl get_r
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
  .globl get_g
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
  .globl get_b
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
  .globl get_a
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
  .globl make_pixel
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
  .globl to_grayscale
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

  imull $79, %r12d  # Multiply red value by 79
  imull $128, %r13d # Multiply green value by 128
  imull $49, %r14d  # Multiply blue value by 49

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

  popq %r15         # Restore r12-15 callee-save registers
  popq %r14
  popq %r13
  popq %r12

  ret

/*
 * Compute the gradient needed for pixel at index.
 * 
 * Parameters:
 *   %rdi - index to calculate
 *   %rsi - max index
 */
  .globl gradient
gradient:
  pushq   %rbx                      # Save callee‑saved RBX
  # Compute numerator = 2000000000 * x (x is in RDI)
  imulq   $2000000000, %rdi, %rax   # RAX = 2000000000 * x
  cqo                             # Sign‑extend RAX into RDX for division
  # Compute denominator = 1000000 * max (max is in RSI)
  imulq   $1000000, %rsi, %rcx      # RCX = 1000000 * max
  idivq   %rcx                    # RAX = (2000000000*x) / (1000000*max)

  # Compute square = (result) - 1000
  subq    $1000, %rax             # RAX = temp – 1000
  movq    %rax, %rbx              # Save square in RBX

  # Compute square * square
  movq    %rbx, %rax              # RAX = square
  imulq   %rbx, %rax              # RAX = square * square

  # Compute final result: 1000000 – (square * square)
  movq    $1000000, %r8           # R8 = 1000000
  subq    %rax, %r8             # R8 = 1000000 – square^2
  movq    %r8, %rax             # Move result into RAX

  popq    %rbx                  # Restore RBX
  ret

/*
 * Compute the 1-dimensional index from column and row indices.
 * 
 * Parameters:
 *   %rdi - pointer to the input Image
 *   %esi - column index
 *   %edx - row index
 */
  .globl compute_index
compute_index:
  movl IMAGE_WIDTH_OFFSET(%rdi), %r10d  # Store image width in temporary register
  movq $0, %rax                         # Init return register to 0
  addl %edx, %eax                       # Add row index to return register
  imull %r10d, %eax                     # Multiply row index with image width
  addl %esi, %eax                       # Add column index to return register
  ret

/*
 * fade_pixel - Compute a faded version of an input pixel.
 *
 * This function computes a faded pixel value using two gradient factors.
 * The fade effect is achieved by multiplying the red, green, and blue components
 * of the input pixel by the product of gradrow and gradcol, then scaling the result
 * by dividing by 1,000,000,000,000. The alpha channel is preserved.
 *
 * Parameters:
 *   %rdi - gradrow (int64_t): vertical gradient factor.
 *   %rsi - gradcol (int64_t): horizontal gradient factor.
 *   %rdx - pixel (uint32_t): the input pixel in RGBA format.
 *
 * Returns:
 *   A new faded pixel value as a uint32_t in %eax.
 *
 * Registers Used:
 *   %rax, %rdx, %rcx for arithmetic operations.
 *   %rbx, %r12, %r13, %r14, %r15 are preserved across calls.
 */
	.globl fade_pixel
fade_pixel:
    pushq   %rbp	#Save the pointer
    pushq   %rbx	#Preserve rbx
    pushq   %r12	#Preserve r12
    pushq   %r13	#Preserve r13
    pushq   %r14	#Preserve r14
    pushq   %r15	#Preserve r15

    
    movq    %rdx, %rbx	# Save the input pixel (originally in rdx) into rbx for repeated use.

    # Multiply the vertical and horizontal gradient factors.
    # Input: gradrow in rdi, gradcol in rsi.
    imulq   %rdi, %rsi       # Now rsi = gradrow * gradcol

    # Load the constant divisor for scaling (1,000,000,000,000).
    movq    $1000000000000, %rcx

    # --- Process Red Channel ---
    movq    %rbx, %rdi       # Pass the pixel to get_r.
    call    get_r            # Result in eax (red channel)
    imulq   %rsi, %rax       # Scale red by the gradient product.
    xorq    %rdx, %rdx       # Clear rdx for division.
    divq    %rcx             # Divide scaled red by the divisor.
    movq    %rax, %r12       # Save faded red in r12.

    # --- Process Green Channel ---
    movq    %rbx, %rdi       # Pass the pixel to get_g.
    call    get_g            # Result in eax (green channel)
    imulq   %rsi, %rax       # Scale green by the gradient product.
    xorq    %rdx, %rdx       # Clear rdx for division.
    divq    %rcx             # Divide scaled green by the divisor.
    movq    %rax, %r13       # Save faded green in r13.

    # --- Process Blue Channel ---
    movq    %rbx, %rdi       # Pass the pixel to get_b.
    call    get_b            # Result in eax (blue channel)
    imulq   %rsi, %rax       # Scale blue by the gradient product.
    xorq    %rdx, %rdx       # Clear rdx for division.
    divq    %rcx             # Divide scaled blue by the divisor.
    movq    %rax, %r14       # Save faded blue in r14.

    # --- Preserve Alpha Channel ---
    movq    %rbx, %rdi       # Pass the pixel to get_a.
    call    get_a            # Result in eax (alpha channel)
    movq    %rax, %r15       # Save the original alpha in r15.

    # --- Reassemble the Faded Pixel ---
    # Pass the faded channels to make_pixel: red, green, blue, then alpha.
    movq    %r12, %rdi       # Faded red as 1st argument.
    movq    %r13, %rsi       # Faded green as 2nd argument.
    movq    %r14, %rdx       # Faded blue as 3rd argument.
    movq    %r15, %rcx       # Original alpha as 4th argument.
    call    make_pixel       # Returns the new pixel in eax.

    # Restore saved registers and the stack frame.
    popq    %r15
    popq    %r14
    popq    %r13
    popq    %r12
    popq    %rbx
    popq    %rbp
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
  pushq %rbx                            # Used for row indexing
  pushq %rbp                            # Used for col indexing
  pushq %r12                            # Used to store original image pointer
  pushq %r13                            # Used to store output image pointer
  pushq %r14                            # Used to store original pixel value
  pushq %r15                            # Used to store alpha value and new pixel value

  movq %rdi, %r12                       # Store input image pointer
  movq %rsi, %r13                       # Store output image pointer

  movq $0, %rbx                         # Init row counter to 0
  jmp .row_cond_rgb                     # Jump to row condition

.row_loop_rgb:                          # Row loop body
  movq $0, %rbp                         # Init column counter to 0
  jmp .col_cond_rgb                     # Jump to col condition

.col_loop_rgb:                          # Col loop body
  movq %r12, %rdi                       # Load original image pointer into first param
  movl %ebp, %esi                       # Load col counter to second param
  movl %ebx, %edx                       # Load row counter to third param
  call compute_index                    # Compute index in 1D, offset in 32 bits

  movq IMAGE_DATA_OFFSET(%r12), %r10    # Load original data array start address to r10
  movl (%r10,%rax,4), %r14d             # Load pixel value to r14 register
  
  # ----- COPY PIXEL -----
  movq %r13, %rdi                       # Load output image pointer into first param
  movl %ebp, %esi                       # Load col counter to second param
  movl %ebx, %edx                       # Load row counter to third param
  call compute_index                    # Compute index in 1D, offset in 32 bits

  movq IMAGE_DATA_OFFSET(%r13), %r10    # Load output data array start address to r10
  movl %r14d, (%r10,%rax,4)             # Load pixel to output array (copy)
  
  # ----- RED PIXEL -----
  movl %r14d, %edi                      # Load pixel value into get_a param
  call get_a                            # Get alpha value of pixel
  movl %eax, %r15d                      # Store alpha value in r15 register for later use

  movl %r14d, %edi                      # Load pixel value into get_r param
  call get_r                            # Get red value of pixel

  movl %eax, %edi                       # Create red version of original pixel
  movl $0, %esi                         # green = 0
  movl $0, %edx                         # blue = 0
  movl %r15d, %ecx                      # alpha = original
  call make_pixel                       # Make red version pixel
  pushq %r15                            # Save alpha value
  movl %eax, %r15d                      # Load new pixel into r15 register
  
  movq %r13, %rdi                       # Load output image pointer into first param
  movl %ebp, %esi                       # Load col counter into second param
  addl IMAGE_WIDTH_OFFSET(%r12), %esi   # Increment col counter by original image width
  movl %ebx, %edx                       # Load row counter into third param
  call compute_index                    # Compute index for red pixel

  movq IMAGE_DATA_OFFSET(%r13), %r10    # Load output data array start address to r10
  movl %r15d, (%r10,%rax,4)             # Load pixel to output array (red)

  # ----- GREEN PIXEL -----
  movl %r14d, %edi                      # Load pixel value into get_g param
  call get_g                            # Get green value of pixel

  popq %r15                             # Restore alpha value
  movl $0, %edi                         # red = 0
  movl %eax, %esi                       # Create green version of original pixel
  movl $0, %edx                         # blue = 0
  movl %r15d, %ecx                      # alpha = original
  call make_pixel                       # Make green version pixel
  pushq %r15                            # Save alpha value
  movl %eax, %r15d                      # Load new pixel into r15 register
  
  movq %r13, %rdi                       # Load output image pointer into first param
  movl %ebp, %esi                       # Load col counter into second param
  movl %ebx, %edx                       # Load row counter into third param
  addl IMAGE_HEIGHT_OFFSET(%r12), %edx  # Increment row counter by original image height
  call compute_index                    # Compute index for green pixel

  movq IMAGE_DATA_OFFSET(%r13), %r10    # Load output data array start address to r10
  movl %r15d, (%r10,%rax,4)             # Load pixel to output array (green)

  # ----- BLUE PIXEL -----
  movl %r14d, %edi                      # Load pixel value into get_b param
  call get_b                            # Get blue value of pixel

  popq %r15                             # Restore alpha value
  movl $0, %edi                         # red = 0
  movl $0, %esi                         # green = 0
  movl %eax, %edx                       # Create blue version of original pixel
  movl %r15d, %ecx                      # alpha = original
  call make_pixel                       # Make blue version pixel
  movl %eax, %r15d                      # Load new pixel into r15 register
  
  movq %r13, %rdi                       # Load output image pointer into first param
  movl %ebp, %esi                       # Load col counter into second param
  addl IMAGE_WIDTH_OFFSET(%r12), %esi   # Increment col counter by original image width
  movl %ebx, %edx                       # Load row counter into third param
  addl IMAGE_HEIGHT_OFFSET(%r12), %edx  # Increment row counter by original image height
  call compute_index                    # Compute index for blue pixel

  movq IMAGE_DATA_OFFSET(%r13), %r10    # Load output data array start address to r10
  movl %r15d, (%r10,%rax,4)             # Load pixel to output array (blue)

  incl %ebp                             # Increment col by 1

.col_cond_rgb:
  movl IMAGE_WIDTH_OFFSET(%r12), %eax   # Load width temporarily
  cmpl %eax, %ebp                       # Compare col with width
  jl .col_loop_rgb                      # Go to col loop if current col is less than width

  incl %ebx                             # Increment row by 1

.row_cond_rgb:
  movl IMAGE_HEIGHT_OFFSET(%r12), %eax  # Load height temporarily
  cmpl %eax, %ebx                       # Compare row with height
  jl .row_loop_rgb                      # Go to row loop if current row is less than height

  popq %r15                             # Restore callee-save registers
  popq %r14
  popq %r13
  popq %r12
  popq %rbp
  popq %rbx
  
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
	pushq %rbx                            # Used for row indexing
  pushq %rbp                            # Used for col indexing
  pushq %r12                            # Used to store original image pointer
  pushq %r13                            # Used to store output image pointer
  pushq %r14                            # Used to store computed index (as it is same for both images)

  movq %rdi, %r12                       # Store input image pointer
  movq %rsi, %r13                       # Store output image pointer

  movq $0, %rbx                         # Init row counter to 0
  jmp .row_cond_grayscale               # Jump to row condition

.row_loop_grayscale:                    # Row loop body
  movq $0, %rbp                         # Init column counter to 0
  jmp .col_cond_grayscale               # Jump to col condition

.col_loop_grayscale:                    # Col loop body
  movq %r12, %rdi                       # Load original image pointer into first param
  movl %ebp, %esi                       # Load col counter to second param
  movl %ebx, %edx                       # Load row counter to third param
  call compute_index                    # Compute index in 1D, offset in 32 bits
  movq $0, %r14                         # Clear r14 register
  movl %eax, %r14d                      # Move resulting offset to r14 register

  movq IMAGE_DATA_OFFSET(%r12), %r10    # Load original data array start address to r10
  movl (%r10,%r14,4), %edi              # Load pixel value into first param
  call to_grayscale                     # Calculate grayscale pixel

  movq IMAGE_DATA_OFFSET(%r13), %r10    # Load output data array start address to r10
  movl %eax, (%r10,%r14,4)              # Load grayscale pixel to output array

  incl %ebp                             # Increment col by 1

.col_cond_grayscale:
  movl IMAGE_WIDTH_OFFSET(%r12), %eax   # Load width temporarily
  cmpl %eax, %ebp                       # Compare col with width
  jl .col_loop_grayscale                # Go to col loop if current col is less than width

  incl %ebx                             # Increment row by 1

.row_cond_grayscale:
  movl IMAGE_HEIGHT_OFFSET(%r12), %eax  # Load height temporarily
  cmpl %eax, %ebx                       # Compare row with height
  jl .row_loop_grayscale                # Go to row loop if current row is less than height

  popq %r14                             # Restore callee-save registers
  popq %r13
  popq %r12
  popq %rbp
  popq %rbx
  
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
    pushq   %rbp                # Save the old base pointer.
    movq    %rsp, %rbp          # Establish the new base pointer.
    subq    $8, %rsp            # Allocate 8 bytes for local storage.

    # Retrieve the image dimensions from the input image structure.
    movl    (%rdi), %r10d       # Load the image width into r10d.
    movl    4(%rdi), %r11d      # Load the image height into r11d.

    # Save the pointers to the input and output image structures.
    movq    %rdi, %r14         # Save input image pointer in r14.
    movq    %rsi, %r15         # Save output image pointer in r15.

    # Initialize loop indices for rows and columns to 0.
    xorl    %r12d, %r12d          # Set row index (i) to 0.
    xorl    %r13d, %r13d          # Set column index (j) to 0.

.row_loop:
    cmpl    %r11d, %r12d       # Compare current row (i) with image height.
    jae     .end_rows          # If i >= height, exit the row loop.
    xorl    %r13d, %r13d	   # Reset column index (j) to 0 at the start of each row.

.col_loop:
    cmpl    %r10d, %r13d       # Compare current column (j) with image width.
    jae     .end_cols          # If j >= width, finish the current row.

    # Compute the 1D index of the current pixel.
    # Parameters for compute_index: 
    #   rdi = input image pointer, rsi = column index, rdx = row index.
    movq    %r14, %rdi         # Pass input image pointer (stored in r14) in rdi.
    movl    %r13d, %esi        # Pass column index (j) in rsi.
    movl    %r12d, %edx        # Pass row index (i) in edx.
    call    compute_index      # Compute the pixel's 1D index.
    movl    %eax, %r9d         # Save the computed index in r9d.

    # Retrieve the pixel value from the input image's data array.
    pushq   %rbx               # Save rbx since it will be used to hold the pixel.
    movq    8(%r14), %r8       # Load the pointer to the pixel data from the input image.
    movl    (%r8, %r9, 4), %ebx  # Load the pixel at index r9d into ebx.

    # Calculate the vertical gradient (gradrow) for the current pixel.
    movq    %r12, %rdi         # Pass row index (i) as first parameter.
    movq    %r11, %rsi         # Pass image height as second parameter.
    pushq   %r12               # Temporarily save row index.
    pushq   %r11               # Temporarily save image height.
    call    gradient           # Compute vertical gradient.
    popq    %r11               # Restore image height.
    popq    %r12               # Restore row index.
    pushq   %rax               # Save the computed gradrow on the stack.

    # Calculate the horizontal gradient (gradcol) for the current pixel.
    movq    %r13, %rdi         # Pass column index (j) as first parameter.
    movq    %r10, %rsi         # Pass image width as second parameter.
    pushq   %r13               # Temporarily save column index.
    pushq   %r10               # Temporarily save image width.
    call    gradient           # Compute horizontal gradient.
    popq    %r10               # Restore image width.
    popq    %r13               # Restore column index.
    pushq   %rax               # Save the computed gradcol on the stack.

    # Prepare to call fade_pixel by retrieving the gradient factors.
    popq    %rsi               # Pop gradcol into rsi (second parameter for fade_pixel).
    popq    %rdi               # Pop gradrow into rdi (first parameter for fade_pixel).
    movq    %rbx, %rdx         # Move the pixel value from rbx into rdx (third parameter).
    call    fade_pixel         # Call fade_pixel to compute the faded pixel.

    popq    %rbx               # Restore rbx.

    # Store the faded pixel in the output image.
    movq    8(%r15), %r8       # Load the pointer to the output image's pixel data.
    movl    %eax, (%r8, %r9, 4)  # Write the faded pixel at the computed index.

    incl    %r13d              # Increment the column index (j).
    jmp     .col_loop          # Repeat for the next column.

.end_cols:
    incl    %r12d              # Increment the row index (i) after completing a row.
    jmp     .row_loop          # Repeat for the next row.

.end_rows:
    addq    $8, %rsp           # Deallocate the local variable space.
    popq    %rbp               # Restore the original base pointer.
    ret                        # Return to the caller.


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
    pushq   %rbp                            # Prologue: save old base pointer
    movq    %rsp, %rbp                      # Establish new base pointer
    subq    $32,  %rsp                      # Reserve 32 bytes on the stack

    pushq   %r12                            # Save callee-saved register r12
    pushq   %r13                            # Save callee-saved register r13
    pushq   %r14                            # Save callee-saved register r14
    pushq   %r15                            # Save callee-saved register r15
    pushq   %r10                            # Save callee-saved register r10 (though not strictly "callee-saved," you had been using it)
    pushq   %r11                            # Save callee-saved register r11 (same note as above)

	movl    $0, %r12d                       # Initialize row index i = 0 in r12d

    movl    (%rdi),  %r10d                  # Load width from *rdi into r10d
    movl    4(%rdi), %r11d                  # Load height from *(rdi+4) into r11d

    cmpl    %r10d, %r11d                    # Compare width (r10d) and height (r11d)
    jne     .DIM_MISMATCH                   # If width != height, jump to return 0

    movl    %r10d,  %r14d                   # Copy width into r14d
    shrl    $1,     %r14d                   # Divide r14d (width) by 2
    movl    %r11d,  %r15d                   # Copy height into r15d
    shrl    $1,     %r15d                   # Divide r15d (height) by 2



    movl    %r10d, %r9d                     # Move width into r9d (for odd check)
    andl    $1,    %r9d                     # Mask out all but the least significant bit
    cmpl    $1,    %r9d                     # Compare the result to 1
    je      .ODDIM_PATH                     # If width is odd, jump to odd-dimension logic


.EVEN_ROW_LOOP:                             # Start of the row loop for even dims

    movl    %r12d, %ecx                     # Copy i into ecx
    subl    %r15d, %ecx                     # ecx = i - (height/2)
    js      .EVEN_PROCEED_ROW               # If ecx < 0 => i < height/2 => proceed
    jmp     .EVEN_END_ROW                   # Otherwise, exit row loop

.EVEN_PROCEED_ROW:                          # Continue row iteration
    movl    %r12d, %r13d                    # Let j = i (r13d = i)

.EVEN_COL_LOOP:                             # Column loop label
    cmpl    %r14d, %r13d                    # Compare j to width/2
    jae     .EVEN_END_COL                   # If j >= width/2, end column loop

    movl    %r10d, %edx                     # edx = width
    imull   %r12d, %edx                     # edx *= i (row index)
    addl    %r13d, %edx                     # edx += j (column index)
    movl    %edx,  %r9d                     # Store final pixel index in r9d

    movq    %rdi,  %r8                      # r8 = input image pointer
    addq    $8,    %r8                      # Skip past width/height in the struct
    movq    (%r8), %r8                      # r8 = pointer to pixel array
    movl    (%r8,%r9,4), %eax               # eax = pixel at index r9

    movq    %rsi,  %r8                      # r8 = output image pointer
    addq    $8,    %r8                      # Skip past width/height in the struct
    movq    (%r8), %r8                      # r8 = pointer to output pixel array
    movl    %eax,  (%r8,%r9,4)              # Write pixel to the same index in output (section A)

    movl    %r10d, %edx                     # edx = width
    imull   %r13d, %edx                     # edx *= j
    addl    %r12d, %edx                     # edx += i
    movl    %eax,  (%r8,%rdx,4)             # Store pixel in wedge B (swap i,j)

    movl    %r10d, %edx                     # edx = width
    imull   %r12d, %edx                     # edx *= i
    addl    %r10d, %edx                     # edx += width
    subl    $1,    %edx                     # edx -= 1
    subl    %r13d, %edx                     # edx -= j
    movl    %edx,  %r9d                     # r9d = mirrored index across vertical axis (A)
    movl    %eax,  (%r8,%r9,4)              # Place pixel there

    movl    %r10d, %edx                     # edx = width
    imull   %r13d, %edx                     # edx *= j
    addl    %r10d, %edx                     # edx += width
    subl    $1,    %edx                     # edx -= 1
    subl    %r12d, %edx                     # edx -= i
    movl    %eax,  (%r8,%rdx,4)             # Mirror wedge B across vertical axis

    movl    %r10d, %edx                     # edx = width
    subl    $1,    %edx                     # edx -= 1
    subl    %r12d, %edx                     # edx -= i
    imull   %r10d, %edx                     # edx *= width
    addl    %r13d, %edx                     # edx += j
    movl    %eax,  (%r8,%rdx,4)             # Mirror A across horizontal axis (C)

    movl    %r10d, %edx                     # edx = width
    subl    $1,    %edx                     # edx -= 1
    subl    %r13d, %edx                     # edx -= j
    imull   %r10d, %edx                     # edx *= width
    addl    %r12d, %edx                     # edx += i
    movl    %eax,  (%r8,%rdx,4)             # Mirror B across horizontal axis (D)

    movl    %r10d, %edx                     # edx = width
    subl    $1,    %edx                     # edx -= 1
    subl    %r13d, %edx                     # edx -= j
    imull   %r10d, %edx                     # edx *= width
    subl    $1,    %edx                     # edx -= 1
    subl    %r12d, %edx                     # edx -= i
    addl    %r10d, %edx                     # edx += width
    movl    %edx,  %r9d                     # r9d = index for vertical mirror of C
    movl    %eax,  (%r8,%r9,4)              # Write pixel

    movl    %r10d, %edx                     # edx = width
    subl    $1,    %edx                     # edx -= 1
    subl    %r12d, %edx                     # edx -= i
    imull   %r10d, %edx                     # edx *= width
    subl    $1,    %edx                     # edx -= 1
    subl    %r13d, %edx                     # edx -= j
    addl    %r10d, %edx                     # edx += width
    movl    %eax,  (%r8,%rdx,4)             # Mirror D across the vertical axis

    incl    %r13d                            # j++
    jmp     .EVEN_COL_LOOP                  # Go back to .EVEN_COL_LOOP

.EVEN_END_COL:                              # End of column loop for even dims
    incl    %r12d                            # i++
    jmp     .EVEN_ROW_LOOP                  # Jump back to row loop start

.EVEN_END_ROW:                              # Done processing even dims
    popq    %r11                            # Restore r11
    popq    %r10                            # Restore r10
    popq    %r15                            # Restore r15
    popq    %r14                            # Restore r14
    popq    %r13                            # Restore r13
    popq    %r12                            # Restore r12
    addq    $32, %rsp                       # Deallocate stack space
    popq    %rbp                            # Restore old base pointer
    movl    $1, %eax                        # Return value = 1
    ret                                     # Return from function

.ODDIM_PATH:                                # Entry point for odd-dimension logic
    incl    %r15d                           # r15d = height/2 + 1
    incl    %r11d                           # r11d = height + 1

.ODDIM_ROW_LOOP:                            # Row loop for odd dimension
    cmpl    %r15d, %r12d                    # Compare i to (height/2 + 1)
    jae     .ODDIM_END_ROW                  # If i >= that value, we're done

    movl    %r12d, %r13d                    # Let j = i

.ODDIM_COL_LOOP:                            # Column loop for odd dimension
    cmpl    %r15d, %r13d                    # Compare j to (height/2 + 1)
    jae     .ODDIM_END_COL
    cmpl    %r11d, %r12d                    # Check i < real height
    jge     .ODDIM_END_COL
    cmpl    %r11d, %r13d                    # Check j < real height
    jge     .ODDIM_END_COL

    pushq   %rdi                            # Save rdi (input pointer)
    pushq   %rsi                            # Save rsi (output pointer)
    movl    %r11d, %edi                     # edi = height + 1
    decl    %edi                            # edi = height
    subl    %r12d, %edi                     # edi = height - i
    cmpl    %r10d, %edi                     # compare with width
    popq    %rsi                            # restore rsi
    popq    %rdi                            # restore rdi
    jge     .ODDIM_CHK_I                    # jump if mirrored row out of range

    pushq   %rdi                            # Save rdi again
    pushq   %rsi                            # Save rsi again
    movl    %r11d, %edi                     # edi = height + 1
    decl    %edi                            # edi = height
    subl    %r13d, %edi                     # edi = height - j
    cmpl    %r10d, %edi                     # compare with width
    popq    %rsi                            # restore rsi
    popq    %rdi                            # restore rdi
    jge     .ODDIM_CHK_J                    # jump if mirrored col out of range

    movl    %r10d, %edx                     # edx = width
    imull   %r12d, %edx                     # edx *= i
    addl    %r13d, %edx                     # edx += j
    movl    %edx,  %r9d                     # pixel index

    movq    %rdi,  %r8                      # r8 = input pointer
    addq    $8,    %r8                      # skip first 8 bytes
    movq    (%r8), %r8                      # r8 = input pixel array
    movl    (%r8,%r9,4), %eax               # eax = pixel at [r8 + r9*4]

    movq    %rsi,  %r8                      # r8 = output pointer
    addq    $8,    %r8                      # skip first 8 bytes
    movq    (%r8), %r8                      # r8 = output pixel array
    movl    %eax,  (%r8,%r9,4)              # Write pixel to top-left wedge

    movl    %r10d, %edx                     # edx = width
    imull   %r13d, %edx                     # edx *= j
    addl    %r12d, %edx                     # edx += i
    movl    %eax,  (%r8,%rdx,4)             # Write pixel for wedge B (swap i/j)

    movl    %r10d, %edx                     # Mirror across vertical axis (A)
    imull   %r12d, %edx
    addl    %r11d, %edx
    decl    %edx
    subl    %r13d, %edx
    movl    %edx,  %r9d
    movl    %eax,  (%r8,%r9,4)

    movl    %r10d, %edx                     # Mirror across vertical axis (B)
    imull   %r13d, %edx
    addl    %r11d, %edx
    decl    %edx
    subl    %r12d, %edx
    movl    %edx,  %r9d
    movl    %eax,  (%r8,%r9,4)

    movl    %r11d, %edx                     # Mirror A across horizontal axis (C)
    decl    %edx
    subl    %r12d, %edx
    imull   %r10d, %edx
    addl    %r13d, %edx
    movl    %edx,  %r9d
    movl    %eax,  (%r8,%r9,4)

    movl    %r11d, %edx                     # Mirror B across horizontal axis (D)
    decl    %edx
    subl    %r13d, %edx
    imull   %r10d, %edx
    addl    %r12d, %edx
    movl    %edx,  %r9d
    movl    %eax,  (%r8,%r9,4)

    movl    %r11d, %edx                     # Reflect C across vertical axis
    decl    %edx
    subl    %r12d, %edx
    imull   %r10d, %edx
    addl    %r11d, %edx
    decl    %edx
    subl    %r13d, %edx
    movl    %edx,  %r9d
    movl    %eax,  (%r8,%r9,4)

    movl    %r11d, %edx                     # Another reflection of C
    decl    %edx
    subl    %r13d, %edx
    imull   %r10d, %edx
    addl    %r11d, %edx
    decl    %edx
    subl    %r12d, %edx
    movl    %edx,  %r9d
    movl    %eax,  (%r8,%r9,4)

    incl    %r13d                            # j++
    jmp     .ODDIM_COL_LOOP                 # Go back to .ODDIM_COL_LOOP

.ODDIM_CHK_I:                               # Label for row boundary check
    cmpl    $0, %r12d                       # Check if i == 0
    je      .ODDIM_SPECIAL_I                # If i == 0, handle specially

    incl    %r13d                            # Otherwise, j++
    jmp     .ODDIM_COL_LOOP                 # Back to column loop

.ODDIM_CHK_J:                               # Label for column boundary check
    cmpl    $0, %r13d                       # Check if j == 0
    je      .ODDIM_SPECIAL_J                # If j == 0, handle specially

    incl    %r13d                            # Otherwise, j++
    jmp     .ODDIM_COL_LOOP                 # Back to column loop

.ODDIM_SPECIAL_I:                           # Special logic if i == 0
    cmpl    %r15d, %r13d                    # Compare j vs half-height+1
    jae     .ODDIM_END_COL                  # If j is out of range, end col loop

    movl    %r10d, %edx                     # edx = width
    imull   %r12d, %edx                     # edx *= i
    addl    %r13d, %edx                     # edx += j
    movl    %edx,  %r9d                     # r9d = pixel index

    movq    %rdi,  %r8                      # r8 = input pointer
    addq    $8,    %r8                      # skip 8 bytes
    movq    (%r8), %r8                      # r8 = input pixel array
    movl    (%r8,%r9,4), %eax               # eax = pixel

    movq    %rsi,  %r8                      # r8 = output pointer
    addq    $8,    %r8                      # skip 8 bytes
    movq    (%r8), %r8                      # r8 = output pixel array
    movl    %eax,  (%r8,%r9,4)              # Write pixel at same index

    movl    %r10d, %edx                     # Wedge B
    imull   %r13d, %edx
    addl    %r12d, %edx
    movl    %edx,  %r9d
    movl    %eax,  (%r8,%r9,4)

    movl    %r10d, %edx                     # Mirror across vertical axis
    imull   %r12d, %edx
    addl    %r11d, %edx
    decl    %edx
    subl    %r13d, %edx
    movl    %edx,  %r9d
    movl    %eax,  (%r8,%r9,4)

    movl    %r11d, %edx                     # Mirror across horizontal axis
    decl    %edx
    subl    %r13d, %edx
    imull   %r10d, %edx
    addl    %r12d, %edx
    movl    %edx,  %r9d
    movl    %eax,  (%r8,%r9,4)

    incl    %r13d                            # j++
    jmp     .ODDIM_COL_LOOP                 # Continue columns

.ODDIM_SPECIAL_J:                           # Special logic if j == 0
    cmpl    %r15d, %r12d                    # Compare i vs half-height+1
    jae     .ODDIM_END_ROW                  # If i out of range, end row loop

    movl    %r10d, %edx                     # edx = width
    imull   %r12d, %edx                     # edx *= i
    addl    %r13d, %edx                     # edx += j
    movl    %edx,  %r9d                     # pixel index

    movq    %rdi,  %r8                      # r8 = input pointer
    addq    $8,    %r8                      # skip first 8 bytes
    movq    (%r8), %r8                      # r8 = input pixel array
    movl    (%r8,%r9,4), %eax               # eax = pixel

    incl    %r13d                            # j++
    jmp     .ODDIM_COL_LOOP                 # loop again

.ODDIM_END_COL:                             # End of col loop for odd dims
    incl    %r12d                            # i++
    jmp     .ODDIM_ROW_LOOP                 # back to row loop

.ODDIM_END_ROW:                             # Done with row loop for odd dims
    popq    %r11                            # Restore r11
    popq    %r10                            # Restore r10
    popq    %r15                            # Restore r15
    popq    %r14                            # Restore r14
    popq    %r13                            # Restore r13
    popq    %r12                            # Restore r12
    addq    $32, %rsp                       # Free stack space
    popq    %rbp                            # Restore old base pointer
    movl    $1, %eax                        # Return 1
    ret                                     # Return from function

.DIM_MISMATCH:                              # Jump target when width != height
    popq    %r11                            # Restore r11
    popq    %r10                            # Restore r10
    popq    %r15                            # Restore r15
    popq    %r14                            # Restore r14
    popq    %r13                            # Restore r13
    popq    %r12                            # Restore r12
    addq    $32, %rsp                       # Release stack memory
    popq    %rbp                            # Restore rbp
    movl    $0, %eax                        # Return 0
    ret                                     # Return



	/* This avoids linker warning about executable stack */
.section .note.GNU-stack,"",@progbits

/*
vim:ft=gas:
*/
