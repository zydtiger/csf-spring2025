#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "uint256.h"

// Create a UInt256 value from a single uint32_t value.
// Only the least-significant 32 bits are initialized directly,
// all other bits are set to 0.
UInt256 uint256_create_from_u32( uint32_t val ) {
  UInt256 result;
  result.data[0] = val;
  for (int i = 1; i < 8; i++){
    result.data[i] = 0U;
  }
  return result;
}

// Create a UInt256 value from an array of NWORDS uint32_t values.
// The element at index 0 is the least significant, and the element
// at index 7 is the most significant.
UInt256 uint256_create( const uint32_t data[8] ) {
  UInt256 result;
  for (int i = 0; i < 8; i++){
    result.data[i] = data[i];
  }
  return result;
}

// Create a UInt256 value from a string of hexadecimal digits.
UInt256 uint256_create_from_hex( const char *hex ) {
  uint32_t units[8] = {0};
  int hex_len = strlen(hex);
  int cnt=0;
  for (int i=hex_len-1; i>=0 && cnt<8; i-=8, cnt++) {
    int unit_start = (i-7<0) ? 0 : (i-7);
    int unit_len = i-unit_start+1;
    char unit[9] = {0};
    strncpy(unit, &hex[unit_start], unit_len);
    units[cnt] = strtoul(unit, NULL, 16);
  }
  UInt256 result = uint256_create(units);  
  return result;
}

// Return a dynamically-allocated string of hex digits representing the
// given UInt256 value.
char *uint256_format_as_hex( UInt256 val ) {
  char hex_buf[65] = {0};
  for (int i=0;i<8;i++) {
    uint32_t unit = uint256_get_bits(val, 7-i);
    sprintf(&hex_buf[i*8], "%08x", unit);
  }

  int hex_buf_start = 0;
  // stop at 63 so always include the last digit to result,
  // even if it is '0'
  while(hex_buf[hex_buf_start] == '0' && hex_buf_start < 63)
    hex_buf_start++;
  int hex_len = 64-hex_buf_start;
  char *hex = malloc(hex_len+1);
  if (!hex) return NULL;
  strcpy(hex, &hex_buf[hex_buf_start]);
  hex[hex_len] = '\0';
  return hex;
}

// Get 32 bits of data from a UInt256 value.
// Index 0 is the least significant 32 bits, index 7 is the most
// significant 32 bits.
uint32_t uint256_get_bits( UInt256 val, unsigned index ) {
  return val.data[index];
}

// Return 1 if bit at given index is set, 0 otherwise.
int uint256_is_bit_set( UInt256 val, unsigned index ) {
  unsigned index_32bits = index / 32;
  unsigned bit_position = index - index_32bits * 32;
  uint32_t target_bits = uint256_get_bits(val, index_32bits);
  uint32_t mask = (1 << bit_position);
  return (target_bits & mask) != 0;
}

// Compute the sum of two UInt256 values.
UInt256 uint256_add( UInt256 left, UInt256 right ) {
  UInt256 sum = uint256_create_from_u32(0);
  uint32_t carry = 0;
  for (int i=0;i<8;i++) {
    uint64_t unit_sum = (uint64_t)left.data[i] + (uint64_t)right.data[i] + carry;
    sum.data[i] = (uint32_t)(unit_sum & 0xFFFFFFFF);
    carry = (unit_sum >> 32);
  }
  return sum;
}

// Compute the difference of two UInt256 values.
UInt256 uint256_sub( UInt256 left, UInt256 right ) {
  UInt256 right_complement = uint256_negate(right);
  return uint256_add(left, right_complement);
}

// Return the two's-complement negation of the given UInt256 value.
UInt256 uint256_negate( UInt256 val ) {
  UInt256 result;
  for (int i=0;i<8;i++)
    result.data[i] = ~val.data[i]; // flip bits
  result = uint256_add(result, uint256_create_from_u32(1));
  return result;
}

// Compute the product of two UInt256 values.
UInt256 uint256_mul( UInt256 left, UInt256 right ) {
  UInt256 product;

  memset(&product, 0, sizeof(product));

  for (unsigned i = 0; i < 256; i++){
    if (uint256_is_bit_set(left, i)){
      UInt256 term = uint256_lshift(right, i);
      product = uint256_add(product, term); 

    }
  }

  return product;
}

UInt256 uint256_lshift( UInt256 val, unsigned shift ) {
  assert( shift < 256 );
  UInt256 result;
  unsigned shift_32 = shift >> 5;
  unsigned shift_bit = shift & 31;

  memset(&result, 0, sizeof(result));

  for(int i = 7; i >= (int)shift_32; i--){
    result.data[i] = val.data[i-shift_32];
  }
  if(shift_bit != 0){
    for(int i = 7; i > 0; i--){
      result.data[i] = (result.data[i] << shift_bit) | (result.data[i-1] >> (32 - shift_bit));
    }

    result.data[0] <<= shift_bit;
  }
  return result;
}
