#ifndef __TEST_H__
#define __TEST_H__

#define RUN(t) printf("\e[1mRunning: "#t"\e[0m\n"); t;

#define PASSED printf("\e[32mPASSED\e[0m\n");

#define COMPLETE printf("\e[32mTEST COMPLETE\e[0m\n");

#endif // __TEST_H__
