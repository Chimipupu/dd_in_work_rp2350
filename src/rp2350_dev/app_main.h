#ifndef APP_MAIN_H
#define APP_MAIN_H

#include <stdint.h>

// 四則演算の回数（整数、float,double）100万回
#define TEST_LOOP_CNT 1000000

void app_core_0_main(void);
void app_core_1_main(void);
void floating_point_test(void);
void pi_calculation_wrapper(void);
void measure_execution_time(void (*p_func)(void), const char* p_func_name, ...);
double calculate_pi_gauss_legendre(int iterations);
void trig_functions_test(void);
void atan2_test(void);
void tan_355_226_test(void);
void inverse_sqrt_test(void);
void int_arithmetic_test(void);
void float_arithmetic_test(void);
void double_arithmetic_test(void);
void int_add_test(void);
void int_sub_test(void);
void int_mul_test(void);
void int_div_test(void);
void float_add_test(void);
void float_sub_test(void);
void float_mul_test(void);
void float_div_test(void);
void double_add_test(void);
void double_sub_test(void);
void double_mul_test(void);
void double_div_test(void);

#endif // APP_MAIN_H