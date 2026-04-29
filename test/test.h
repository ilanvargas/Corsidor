#ifndef __TEST_H__
#define __TEST_H__

#define RUN(t)                           \
  printf("\e[1mRunning: " #t "\e[0m\n"); \
  t;

#define COMPLETE printf("TESTS COMPLETED\n");

#define CMP_INT(a, b)              \
  if (a != b) {                    \
    printf("\e[31merror\033[0m "); \
  } else                           \
    printf("\e[32mOK\e[0m ");

#endif  // __TEST_H__
