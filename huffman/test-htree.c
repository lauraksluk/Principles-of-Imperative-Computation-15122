/* Huffman coding
 *
 * Main file for testing is_htree
 * 15-122 Principles of Imperative Computation
 */

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "lib/xalloc.h"
#include "lib/contracts.h"

#include "htree.h"

struct htree_node {
  symbol_t value;
  unsigned int frequency;
  htree *left;
  htree *right;
};

bool is_htree(htree *H);
bool is_htree_leaf(htree *H);
bool is_htree_interior(htree *H);


int main () {
  // Write tests for is_tree here


  printf("Success!\n");
  return 0;
}
