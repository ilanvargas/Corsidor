#ifndef __TEST_H__
#define __TEST_H__

#define RUN(t)                           \
  printf("\e[1mRunning: " #t "\e[0m\n"); \
  t;

#define PASSED printf("\e[32mPASSED\e[0m\n");

#define COMPLETE printf("\e[32mTEST COMPLETE\e[0m\n");

#define CMP_INT(a,b)                                             \
  if (a!=b){                                                     \
    printf("\033[31merror\033[0m : "#a" != "#b"\n",stderr);      \
    exit(EXIT_FAILURE);                                          \
  }                                                              \
  else                                                           \
    printf("Ok\n");

#endif  // __TEST_H__
