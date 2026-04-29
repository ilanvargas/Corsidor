#ifndef __TEST_H__
#define __TEST_H__

#define RUN(t)                           \
  printf("\e[1mRunning: " #t "\e[0m\n"); \
  t;

#define PASSED printf("\e[32mPASSED\e[0m\n");

#define COMPLETE printf("END TESTS\n");

#define CMP_INT(a,b)  \
  if (a != b){  \
    printf("\e[31merror\033[0m\n"); \
  } \
  else  \
    printf("\e[32mOK\e[0m\n");

#endif  // __TEST_H__
