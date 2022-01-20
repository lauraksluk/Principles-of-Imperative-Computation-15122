/* Huffman coding
 *
 * 15-122 Principles of Imperative Computation
 */

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "lib/contracts.h"
#include "lib/xalloc.h"
#include "lib/file_io.h"
#include "lib/heaps.h"

#include "freqtable.h"
#include "htree.h"
#include "encode.h"
#include "bitpacking.h"

// Print error message
void error(char *msg) {
  fprintf(stderr, "%s\n", msg);
  exit(1);
}

/* in htree.h: */
/* typedef struct htree_node htree; */
struct htree_node {
  symbol_t value;
  unsigned int frequency;
  htree *left;
  htree *right;
};

/**********************************************/
/* Task 1: Checking data structure invariants */
/**********************************************/

/* forward declaration -- DO NOT MODIFY */
bool is_htree(htree *H);

// Checks if H is a valid Huffman tree leaf.
bool is_htree_leaf(htree *H) {
  if (H == NULL) return false;
  return 0 < H->frequency && H->left == NULL && H->right == NULL;
}

// Returns the sum of the frequency of H's children.
unsigned int freqSum(htree *H) {
  if (is_htree_leaf(H)) return H->frequency;
  return freqSum(H->left) + freqSum(H->right);
}

// Checks if H is a valid Huffman tree interior node.
bool is_htree_interior(htree *H) {
  if (H == NULL) return false;
  return is_htree(H->left) && is_htree(H->right) && freqSum(H) == H->frequency;
}

// Checks if H is a valid Huffman tree.
bool is_htree(htree *H) {
  return H != NULL && (is_htree_leaf(H) || is_htree_interior(H));
}

/********************************************************/
/* Task 2: Building Huffman trees from frequency tables */
/********************************************************/

/* for libs/heaps.c */
bool htree_higher_priority(elem e1, elem e2) {
  return ((htree*)e1)->frequency < ((htree*)e2)->frequency;
}

// Returns boolean depending on if there are 2 or more symbols
// with nonzero frequencies. 
bool count(freqtable_t table) {
  unsigned int count = 0;
  for (unsigned short c = 0; c < NUM_SYMBOLS; c++) {
    if (table[(symbol_t)c] != 0) count ++;
    if (count >= 2) return true;
  }
  return false;
}

// Wrapper function for htree_free.
void htree_free_v(void *input) {
  htree_free((htree*)input);
}

// Builds Huffman tree from Huffman leaves/nodes represented with a min heap.
htree* make_tree(pq_t minHeap) {
  while(!pq_empty(minHeap)) {
    htree *leaf1 = (htree*)pq_rem(minHeap);
    ASSERT(leaf1 != NULL);
    if (!pq_empty(minHeap)) {
      htree *leaf2 = (htree*)pq_rem(minHeap);
      ASSERT(leaf2 != NULL);
      htree *node12 = xmalloc(sizeof(htree));
      ASSERT(node12 != NULL);
      node12->frequency = leaf1->frequency + leaf2->frequency;
      node12->left = leaf1;
      node12->right = leaf2;
      pq_add(minHeap, (elem)node12);
    } else {
      return leaf1;
    }
  }
  // Dummy return statement.
  return NULL;
}

// build a htree from a frequency table
htree* build_htree(freqtable_t table) {
  REQUIRES(is_freqtable(table));
  if (!count(table)) error("Not enough symbols.");
  else {
    // Construct a min heap of htree nodes (leaf).
    pq_t minHeap = pq_new(NUM_SYMBOLS, &htree_higher_priority, &htree_free_v);
    for (unsigned short c = 0; c < NUM_SYMBOLS; c++) {
      if (table[(symbol_t)c] != 0) {
        htree *new = xmalloc(sizeof(htree));
        ASSERT(new != NULL);
        new->value = (symbol_t)c;
        new->frequency = table[(symbol_t)c];
        new->left = NULL;
        new->right = NULL;
        pq_add(minHeap, (elem)new);
      }
    }
    // Combine htree leaves into Huffman tree.
    htree *result = make_tree(minHeap);
    // Free min heap.
    pq_free(minHeap);
    ENSURES(is_htree(result));
    return result;
  }
  // Dummy return statement.
  return NULL;
}

/*******************************************/
/*  Task 3: decoding a text                */
/*******************************************/

// Returns the value that *src_len is supposed to hold.
// Returns the length of final decoded string.
size_t srcDecode(htree *H, bit_t *code) {
  htree *pos = H;
  size_t count = 0;
  int i = 0;
  ASSERT(pos != NULL);
  while (code[i] != '\0') {
    // Follow left branch if bit value is 0.
    if (code[i] == '0') {
      pos = pos->left;
    }
    // Follow right branch if bit value is 1.
    if (code[i] == '1') {
      pos = pos->right;
    }
    // Traversal reaches leaf.
    if (is_htree_leaf(pos)) {
      // Update symbol count and reset pointer.
      count++;
      pos = H;
    }
    // Increment bit string index.
    i++;
  }
  ASSERT(pos != NULL);
  return count;
}

// Decode code according to H, putting decoded length in src_len
symbol_t* decode_src(htree *H, bit_t *code, size_t *src_len) {
  REQUIRES(is_htree(H) && code != NULL && src_len != NULL);

  // Call helper function for length of decoded string.
  *src_len = srcDecode(H, code);

  // Initialize variables.
  symbol_t *result = xcalloc(*src_len, sizeof(symbol_t));
  ASSERT(result != NULL);
  htree *pos = H;
  int count = 0;
  int i = 0;
  ASSERT(pos != NULL);
  while (code[i] != '\0') {
    // Follow left branch if bit value is 0.
    if (code[i] == '0') {
      pos = pos->left;
    }
    // Follow right branch if bit value is 1.
    if (code[i] == '1') {
      pos = pos->right;
    }
    // Traversal reaches leaf.
    if (is_htree_leaf(pos)) {
      // Fill in decoded string with symbol.
      result[count] = pos->value;
      // Increment decoded string index and reset pointer.
      count++;
      pos = H;
    }
    // Increment bit string index.
    i++;
  }
  ASSERT(pos != NULL);
  // Return error message if decoding fails.
  if (pos != H) error("string cannot be decoded.");
  ENSURES(result != NULL);
  return result;
}

/****************************************************/
/* Tasks 4: Building code tables from Huffman trees */
/****************************************************/

// Returns the maximum possible bit string length for given H (height of Huffman tree).
int length(htree *H) {
  REQUIRES(is_htree(H));

  // Base case.
  if (is_htree_leaf(H)) return 1;
  
  if (is_htree_interior(H)) {
    // Recurse on left and right child.
    int left = length(H->left);
    int right = length(H->right);

    // Obtain current maximum height.
    if (left > right) return left + 1;
    else return right + 1;
  }
  // Dummy return statement.
  return 0;
}

// Recursive helper function for obtaining each symbol's encoding.
void codetable_help(htree *pos, bitstring_t string, codetable_t result, int size) {
  REQUIRES(pos != NULL);

  // Base case.
  if (is_htree_leaf(pos)) {
    // Make a copy of current encoding string.
    bitstring_t copy = xcalloc(size, sizeof(bit_t));
    strcpy(copy, string);

    // Insert into codetable.
    result[pos->value] = copy;

  } else {
    char left = '0';
    char right = '1';

    // Make a copy of cuurrent encoding string.
    char* orig = xcalloc(size, sizeof(bit_t));
    strcpy(orig, string);

    // Traverse to the left child and append "0" to copied encoding.
    codetable_help(pos->left, strncat(orig, &left, 1), result, size);
    // Free copied encoding after recursive call returns.
    free(orig);
    // Traverse to the right child and append "1" to current encoding.
    codetable_help(pos->right, strncat(string, &right, 1), result, size);
  }
}

// Returns code table for characters in H
codetable_t htree_to_codetable(htree *H) {
  REQUIRES(is_htree(H));
  // Obtain (maximum) size of each encoding.
  int size = length(H);

  // Initialize variables.
  bitstring_t bitstring = xcalloc(size, sizeof(bit_t));
  ASSERT(bitstring != NULL);
  codetable_t result = xcalloc(NUM_SYMBOLS, sizeof(bitstring_t));
  ASSERT(result != NULL);
  htree *pos = H;
  // Call recursive helper function to populate codetable.
  codetable_help(pos, bitstring, result, size);
  // Free encoding string used in helper function. 
  free(bitstring);

  ENSURES(result != NULL);
  return result;
}

/*******************************************/
/*  Task 5: Encoding a text                */
/*******************************************/

// Helper function to determine the length of the encoded bitstring.
size_t encode_len(codetable_t table, symbol_t *src, size_t src_len) {
  // Initialize size counter taking into account NUL-terminating character.
  size_t result = 1;
  
  for (size_t i = 0; i < src_len; i++) {
    if (table[src[i]] != NULL) {

      // Add length of each bitstring to counter.
      size_t j = 0;
      while ((table[src[i]])[j] != '\0') {
        result++;
        j++;
      }
    } else {
      // Integer "flag" to note unable to encode.
      return 0;
    }
  }
  return result;
}

// Encodes source according to codetable
bit_t* encode_src(codetable_t table, symbol_t *src, size_t src_len) {
  REQUIRES(table != NULL && src != NULL);

  // Call helper function to determine length of encoded string.
  size_t size = encode_len(table, src, src_len);
  if (size == 0) error("string cannot be encoded.");

  // Initialize variables.
  bit_t *result = xcalloc(size, sizeof(bit_t));
  ASSERT(result != NULL);
  size_t count = 0;

  for (size_t i = 0; i < src_len; i++) {
    if (table[src[i]] != NULL) {

      // Loop through bitstring to add into result.
      size_t j = 0;
      while ((table[src[i]])[j] != '\0') {
        result[count] = table[src[i]][j];
        count++;
        ASSERT(count < size);
        // Bitstring needs to be NUL-terminated.
        result[count] = '\0';
        j++;
      }
    } else {
      error("string cannot be encoded.");
    }
  }
  ASSERT(result[size - 1] == '\0');
  ENSURES(result != NULL);
  return result;
}


/**************************************************/
/*  Task 6: Building a frequency table from file  */
/**************************************************/

// Build a frequency table from a source file (or STDIN)
freqtable_t build_freqtable(char *fname) {
  REQUIRES(fname != NULL);

  // Initialize variable.
  freqtable_t table = xcalloc(NUM_SYMBOLS, sizeof(unsigned int));
  ASSERT(table != NULL);
  // Open file to read and initialize variable.
  FILE *desc = fopen(fname, "r");
  assert(desc != NULL);

  for (int sym = fgetc(desc); sym != EOF; sym = fgetc(desc)) {
    // Increment frequency count in table.
    (table[sym])++;
  }
  // Close file.
  fclose(desc);

  ENSURES(is_freqtable(table));
  return table;
}

/************************************************/
/*  Task 7: Packing and unpacking a bitstring   */
/************************************************/

// Helper function for packing a single byte.
uint8_t pack_help(char *s) {
  REQUIRES(s != NULL);

  // Initialize byte result.
  uint8_t result = 0;

  // Convert "binary" to "decimal".
  for (int i = 0; i < 8; i++) {
    result *= 2;
    if (s[i] == '1') {
      result++;
    }
  }
  return result;
}

// Pack a string of bits into an array of bytes; length = strlen(bits)/8
uint8_t* pack(bit_t *bits) {
  REQUIRES(bits != NULL);
  // Determine length of byte array.
  size_t bit_len = strlen(bits);
  unsigned int length = num_padded_bytes(bit_len);

  // Edge case.
  if (bit_len == 0) return NULL;
  
  // Initialize byte array.
  uint8_t *result = xcalloc(length, sizeof(uint8_t));
  ASSERT(result != NULL);

  // Initialize temporary "substring" to pass into helper function.
  char *temp = xcalloc(9, sizeof(char));
  ASSERT(temp != NULL);
  // Initialize index of temporary string.
  size_t index = 0;
  // Initialize index of byte array.
  unsigned int byte_count = 0;

  for (size_t i = 0; i < bit_len; i++) {
    // Every 8 characters in bitstring, call helper function to pack byte.
    if (i != 0 && i % 8 == 0) {
      temp[8] = '\0';
      result[byte_count] = pack_help(temp);
      // Reset/modify temporary string index and byte array index.
      index = 0;
      byte_count++;
    }
    // Copy over characters from bitstring to temporary string.
    temp[index] = bits[i];
    index++;
  }
  // Check if last byte needs to be padded with 0s.
  if (index != 0) {
    while (index < 8) {
      temp[index] = '0';
      index++;
    }
    // Pack byte.
    result[byte_count] = pack_help(temp);
  }
  // Free temporary string.
  free(temp);
  return result;
}

// Helper function for unpacking a single byte.
void unpack_help(uint8_t n, char *temp) {
  REQUIRES(temp != NULL);

  // Fill in character array with ASCII "bits".
  for (int i = 0; i < 8; i++) {
    if (n % 2 == 0) temp[7 - i] = '0';
    if (n % 2 == 1) temp[7 - i] = '1';
    n /= 2;
  }
}

// Unpack an array of bytes c of length len into a NUL-terminated string of ASCII bits
bit_t* unpack(uint8_t *c, size_t len) {
  REQUIRES(c != NULL);

  // Initalize variables.
  bit_t *temp = xcalloc(8, sizeof(char));
  ASSERT(temp != NULL);

  size_t length = 8 * len + 1; 
  bit_t *result = xcalloc(length, sizeof(char));
  ASSERT(result != NULL);

  for (size_t i = 0; i < len; i++) {
    // Call helper function for each "byte" in byte array.
    unpack_help(c[i], temp);

    // Copy over characters into final bitstring.
    for (size_t j = 0; j < 8; j++) {
      result[8*i+j] = temp[j];
    }
  }
  free(temp);
  // Bitstring needs to be NUL-terminated.
  result[length - 1] = '\0';
  ENSURES(result != NULL);
  return result;
}