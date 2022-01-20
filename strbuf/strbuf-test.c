#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "lib/xalloc.h"
#include "strbuf.h"

int main() {
  struct strbuf* buf1 = strbuf_new(5);
  char* hi = xcalloc(3, sizeof(char));
  hi[0] = 'h';
  hi[1] = 'i';
  hi[2] = '\0';
  strbuf_addstr(buf1, hi);
  buf1->buf[0] = '\0';
  buf1->buf[1] = '\0';
  buf1->len = 0;
  assert(is_strbuf(buf1) == true);
  
  char* laur = xcalloc(5, sizeof(char));
  laur[0] = 'l';
  laur[1] = 'a';
  laur[2] = 'u';
  laur[3] = 'r';
  laur[4] = '\0';
  strbuf_add(buf1, laur, 4);
  assert(buf1->buf[0] == 'l');
  assert(buf1->buf[1] == 'a');
  assert(buf1->buf[2] == 'u');
  assert(buf1->buf[3] == 'r');
  assert(buf1->buf[4] == '\0');

  strbuf_add(buf1, laur, 4);
  strbuf_addstr(buf1, laur);
  char* b1 = strbuf_str(buf1);
  assert(b1[0] == 'l');
  assert(b1[1] == 'a');
  assert(b1[2] == 'u');
  assert(b1[3] == 'r');
  assert(b1[4] == 'l');
  assert(b1[5] == 'a');
  assert(b1[6] == 'u');
  assert(b1[7] == 'r');
  assert(b1[8] == 'l');
  assert(b1[9] == 'a');
  assert(b1[10] == 'u');
  assert(b1[11] == 'r');
  assert(b1[12] == '\0');
  printf("buf1 is good.\n");
  free(buf1->buf);
  free(buf1);
  free(hi);
  free(laur);
  free(b1);

  struct strbuf* buf2 = strbuf_new(5);
  char* h = xcalloc(2, sizeof(char));
  h[0] = 'h';
  h[1] = '\0';
  strbuf_addstr(buf2, h);
  buf2->buf[2] = 'l';
  assert(buf2->len == 1);
  assert(strlen(strbuf_str(buf2)) == 1);
  buf2->buf[1] = 'e';
  buf2->buf[3] = '\0';
  buf2->len = 3;
  char* b2 = strbuf_str(buf2);
  assert(strlen(b2) == 3);
  char* o = xcalloc(2, sizeof(char));
  o[0] = 'o';
  o[1] = '\0';
  strbuf_addstr(buf2, o);
  b2 = strbuf_str(buf2);
  assert(b2[0] == 'h');
  assert(b2[1] == 'e');
  assert(b2[2] == 'l');
  assert(b2[3] == 'o');
  assert(b2[4] == '\0');
  printf("buf2 is good.\n");
  free(buf2->buf);
  free(buf2);
  free(h);
  free(o);
  free(b2);

  struct strbuf* buf3 = xmalloc(sizeof(struct strbuf));
  buf3->limit = 2;
  buf3->len = 0;
  buf3->buf = xcalloc(2, sizeof(char));
  char* emptyS = xcalloc(2, sizeof(char));
  emptyS[0] = '\0';
  emptyS[1] = '\0';
  strbuf_addstr(buf3, emptyS);
  assert(buf3->buf[0] == '\0');
  assert(buf3->buf[1] == '\0');
  buf3->len = 1;
  assert(is_strbuf(buf3) == false);
  buf3->len = 0;
  char* hithere = xcalloc(10, sizeof(char));
  hithere[0] = 'h';
  hithere[1] = 'i';
  hithere[2] = ' ';
  hithere[3] = 't';
  hithere[4] = 'h';
  hithere[5] = 'e';
  hithere[6] = 'r';
  hithere[7] = 'e';
  hithere[8] = '!';
  hithere[9] = '\0';
  strbuf_addstr(buf3, hithere);
  char* b3 = strbuf_str(buf3);
  assert(b3[0] == hithere[0]);
  assert(b3[1] == hithere[1]);
  assert(b3[2] == hithere[2]);
  assert(b3[3] == hithere[3]);
  assert(b3[4] == hithere[4]);
  assert(b3[5] == hithere[5]);
  assert(b3[6] == hithere[6]);
  assert(b3[7] == hithere[7]);
  assert(b3[8] == hithere[8]);
  assert(b3[9] == hithere[9]);

  assert(b3[0] == buf3->buf[0]);
  assert(b3[1] == buf3->buf[1]);
  assert(b3[2] == buf3->buf[2]);
  assert(b3[3] == buf3->buf[3]);
  assert(b3[4] == buf3->buf[4]);
  assert(b3[5] == buf3->buf[5]);
  assert(b3[6] == buf3->buf[6]);
  assert(b3[7] == buf3->buf[7]);
  assert(b3[8] == buf3->buf[8]);
  assert(b3[9] == buf3->buf[9]);

  printf("buf3 is good.\n");
  free(buf3->buf);
  free(buf3);
  free(emptyS);
  free(hithere);
  free(b3);

  return 0;
}