#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include "lib/contracts.h"
#include "lib/xalloc.h"
#include "strbuf.h"

// Checks that the NUL-terminator doesn't exist until the end of the string.
bool no_nul_term_until_end(char* buffer, size_t len)
{
  REQUIRES(buffer != NULL);
  size_t i = 0;
  ASSERT(i <= len);
  for (; i < len; i++)
  {
    if (buffer[i] == '\0') return false;
  }
  ASSERT(i <= len);
  return buffer[len] == '\0';
}

// Checks if sb satisfies the data structure invariants of being a string buffer.
bool is_strbuf(struct strbuf* sb) {
  return sb != NULL && sb->buf != NULL && sb->limit > 0 
         && sb->limit > sb->len && sb->buf[sb->len] == '\0'
         && no_nul_term_until_end(sb->buf, sb->len)
         && strlen(sb->buf) == sb->len;
}

// Returns a string buffer of size init_limit, consisting of an empty string.
struct strbuf *strbuf_new(size_t init_limit)
{
  REQUIRES(0 < init_limit);
  struct strbuf* new = xmalloc(sizeof(struct strbuf));
  ASSERT(new != NULL);
  new->limit = init_limit;
  new->len = 0;
  new->buf = xcalloc(init_limit, sizeof(char));
  new->buf[0] = '\0';
  ASSERT(new->buf != NULL);
  ENSURES(is_strbuf(new));
  return new;
}

// Returns a copy of the string-occupied portion of the string buffer.
// The copy is NUL-terminated.
char *strbuf_str(struct strbuf* sb)
{
  REQUIRES(is_strbuf(sb));
  char* temp = xcalloc((sb->len + 1), sizeof(char));
  ASSERT(temp != NULL);
  temp = strcpy(temp, sb->buf);
  ENSURES(temp != NULL);
  return temp;
}

// Concept from Lecture 11, Unbounded Arrays
// Resizes given array to double its size when full.
void resize(struct strbuf* sb)
{
  REQUIRES(is_strbuf(sb) && sb->limit <= INT_MAX/2);
  sb->limit = 2 * (sb->limit);
  char* new = xcalloc((sb->limit), sizeof(char));
  ASSERT(new != NULL);
  size_t i = 0;
  ASSERT(i <= sb->len);
  for (; i < sb->len; i++)
  {
    new[i] = sb->buf[i];
  }
  ASSERT(i <= sb->len);
  free(sb->buf);
  sb->buf = new;
  ENSURES(is_strbuf(sb));
}

// Modifies string buffer to add in input string, str, of length, len.
void strbuf_add(struct strbuf* sb, char* str, size_t len)
{
  REQUIRES(is_strbuf(sb) && str != NULL && strlen(str) == len);
  size_t i = 0;
  ASSERT(i <= len);
  for (; i < len; i++)
  {
    if (sb->len == sb->limit - 1) {
      resize(sb);
    }
    sb->buf[sb->len] = str[i];
    (sb->len)++;
    sb->buf[sb->len] = '\0';
  }
  ASSERT(i <= len);
  ENSURES(is_strbuf(sb));
}

// Modifies string buffer to add in input string, str.
void strbuf_addstr(struct strbuf* sb, char* str)
{
  REQUIRES(is_strbuf(sb) && str != NULL);
  size_t len = strlen(str);
  strbuf_add(sb, str, len);
  ENSURES(is_strbuf(sb));
}

// Deallocates the struct sb, and returns the embedded buffer array.
char *strbuf_dealloc(struct strbuf* sb)
{
  REQUIRES(is_strbuf(sb));
  char* buffer = sb->buf;
  free(sb);
  ENSURES(buffer != NULL);
  return buffer;
}