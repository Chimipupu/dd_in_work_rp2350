#include "dbg_com.h"
#include "app_main.h"
#include "pico/version.h"
#include <stdio.h>
#include <string.h>
#include "hardware/clocks.h"
#include <stdlib.h>
#include "hardware/watchdog.h"

static void dbg_com_init_msg(void);
static void cmd_help(void);
static void cmd_ver(void);
static void cmd_clock(void);
static void cmd_at_test(void);
static void cmd_pi_calc(const dbg_cmd_args_t* p_args);
static void cmd_rst(void);
static void cmd_unknown(void);
static void cmd_trig(void);
static void cmd_atan2(void);
static void cmd_tan355(void);
static void cmd_isqrt(void);

// コマンドテーブル
static const dbg_cmd_info_t s_cmd_table[] = {
    {"help",    CMD_HELP,    "Show this help message", 0, 0},
    {"ver",     CMD_VER,     "Show version information", 0, 0},
    {"clock",   CMD_CLOCK,   "Show clock information", 0, 0},
    {"at",      CMD_AT_TEST, "int/float/double arithmetic test", 0, 0},
    {"pi",      CMD_PI_CALC, "Calculate pi using Gauss-Legendre", 0, 1},
    {"trig",    CMD_TRIG,    "Run sin,cos,tan functions test", 0, 0},
    {"atan2",   CMD_ATAN2,   "Run atan2 test", 0, 0},
    {"tan355",  CMD_TAN355,  "Run tan(355/226) test", 0, 0},
    {"isqrt",   CMD_ISQRT,   "Run 1/sqrt(x) test", 0, 0},
    {"rst",     CMD_RST,     "Reboot", 0, 0},
    {NULL,      CMD_UNKNOWN, NULL, 0, 0}
};

// コマンドバッファ
static char s_cmd_buffer[DBG_CMD_MAX_LEN];
static int32_t s_cmd_index = 0;

static void dbg_com_init_msg(void)
{
    printf("\nDebug Command Monitor for RP2350 Ver 0.1\n");
    printf("Copyright (c) 2025 Chimipupu(https://github.com/Chimipupu)\n");
    printf("Type 'help' for available commands\n");
}

static void cmd_help(void)
{
    dbg_com_init_msg();

    printf("\nAvailable commands:\n");
    for (int32_t i = 0; s_cmd_table[i].p_cmd_str != NULL; i++) {
        printf("  %-10s - %s\n", s_cmd_table[i].p_cmd_str, s_cmd_table[i].p_description);
    }
}

static void cmd_ver(void)
{
    printf("Pico SDK version: %d.%d.%d\n",
        PICO_SDK_VERSION_MAJOR,
        PICO_SDK_VERSION_MINOR,
        PICO_SDK_VERSION_REVISION);
}

static void cmd_clock(void)
{
    printf("System Clock:\t%d MHz\n", clock_get_hz(clk_sys) / 1000000);
    printf("USB Clock:\t%d MHz\n", clock_get_hz(clk_usb) / 1000000);
}

static void cmd_at_test(void)
{
    printf("\nInteger Arithmetic Test:\n");
    measure_execution_time(int_add_test, "int_add_test");
    measure_execution_time(int_sub_test, "int_sub_test");
    measure_execution_time(int_mul_test, "int_mul_test");
    measure_execution_time(int_div_test, "int_div_test");

    printf("\nFloat Arithmetic Tests:\n");
    measure_execution_time(float_add_test, "float_add_test");
    measure_execution_time(float_sub_test, "float_sub_test");
    measure_execution_time(float_mul_test, "float_mul_test");
    measure_execution_time(float_div_test, "float_div_test");

    printf("\nDouble Arithmetic Tests:\n");
    measure_execution_time(double_add_test, "double_add_test");
    measure_execution_time(double_sub_test, "double_sub_test");
    measure_execution_time(double_mul_test, "double_mul_test");
    measure_execution_time(double_div_test, "double_div_test");
}

static void cmd_pi_calc(const dbg_cmd_args_t* p_args)
{
    int32_t iterations = 3;
    if (p_args->argc > 1) {
        iterations = atoi(p_args->p_argv[1]);
        if (iterations <= 0) {
            printf("Error: Invalid iteration count. Must be positive.\n");
            return;
        }
    }
    printf("\nCalculating Pi using Gauss-Legendre algorithm (%d iterations):\n", iterations);
    for (int32_t i = 1; i <= iterations; i++) {
        double pi = calculate_pi_gauss_legendre(i);
        printf("Iteration %d: π ≈ %.15f\n", i, pi);
    }
}

static void cmd_rst(void)
{
    printf("Resetting system...\n");
    watchdog_reboot(0, 0, 0);   // WDTで即時リセット
}

static void cmd_unknown(void)
{
    printf("Unknown command. Type 'help' for available commands.\n");
}

static void cmd_trig(void)
{
    measure_execution_time(trig_functions_test, "trig_functions_test");
}

static void cmd_atan2(void)
{
    measure_execution_time(atan2_test, "atan2_test");
}

static void cmd_tan355(void)
{
    measure_execution_time(tan_355_226_test, "tan_355_226_test");
}

static void cmd_isqrt(void)
{
    measure_execution_time(inverse_sqrt_test, "inverse_sqrt_test");
}

/**
 * @brief 文字列をトークンに分割する
 * 
 * @param p_str 分割する文字列
 * @param p_args 引数構造体
 * @return int32_t トークンの数
 */
static int32_t split_string(char* p_str, dbg_cmd_args_t* p_args)
{
    p_args->argc = 0;
    char* p_token = strtok(p_str, " ");

    while (p_token != NULL && p_args->argc < DBG_CMD_MAX_ARGS) {
        p_args->p_argv[p_args->argc++] = p_token;
        p_token = strtok(NULL, " ");
    }

    return p_args->argc;
}

/**
 * @brief デバッグコマンドモニターの初期化
 */
void dbg_com_init(void)
{
    cmd_help();
}

/**
 * @brief コマンド文字列を解析してコマンド種類を返す
 * 
 * @param p_cmd_str コマンド文字列
 * @param p_args 引数構造体
 * @return dbg_cmd_t コマンド種類
 */
static dbg_cmd_t dbg_com_parse_cmd(const char* p_cmd_str, dbg_cmd_args_t* p_args)
{
    for (int32_t i = 0; s_cmd_table[i].p_cmd_str != NULL; i++) {
        if (strcmp(p_cmd_str, s_cmd_table[i].p_cmd_str) == 0) {
            // 引数の数をチェック
            if (p_args->argc - 1 < s_cmd_table[i].min_args || p_args->argc - 1 > s_cmd_table[i].max_args) {
                printf("Error: Invalid number of arguments. Expected %d-%d, got %d\n",
                    s_cmd_table[i].min_args, s_cmd_table[i].max_args, p_args->argc - 1);
                return CMD_UNKNOWN;
            }
            return s_cmd_table[i].cmd_type;
        }
    }
    return CMD_UNKNOWN;
}

/**
 * @brief コマンドを実行する
 * 
 * @param cmd コマンド種類
 * @param p_args 引数構造体
 */
static void dbg_com_execute_cmd(dbg_cmd_t cmd, const dbg_cmd_args_t* p_args)
{
    switch (cmd) {
        case CMD_HELP:
            cmd_help();
            break;

        case CMD_VER:
            cmd_ver();
            break;

        case CMD_CLOCK:
            cmd_clock();
            break;

        case CMD_AT_TEST:
            cmd_at_test();
            break;

        case CMD_PI_CALC:
            cmd_pi_calc(p_args);
            break;

        case CMD_TRIG:
            cmd_trig();
            break;

        case CMD_ATAN2:
            cmd_atan2();
            break;

        case CMD_TAN355:
            cmd_tan355();
            break;

        case CMD_ISQRT:
            cmd_isqrt();
            break;

        case CMD_RST:
            cmd_rst();
            break;

        case CMD_UNKNOWN:
            cmd_unknown();
            break;
    }
    printf("> ");
}

/**
 * @brief デバッグコマンドモニターのメイン処理
 */
void dbg_com_process(void)
{
    if (s_cmd_index >= DBG_CMD_MAX_LEN - 1) {
        s_cmd_index = 0;
    }

    int32_t c = getchar();
    if (c == '\r' || c == '\n') {
        if (s_cmd_index > 0) {
            s_cmd_buffer[s_cmd_index] = '\0';
            printf("\n");

            dbg_cmd_args_t args;
            split_string(s_cmd_buffer, &args);
            if (args.argc > 0) {
                dbg_cmd_t cmd = dbg_com_parse_cmd(args.p_argv[0], &args);
                dbg_com_execute_cmd(cmd, &args);
            }
            s_cmd_index = 0;
        } else {
            printf("\n> ");
        }
    } else if (c == '\b' || c == 127) {
        if (s_cmd_index > 0) {
            s_cmd_index--;
            printf("\b \b");
        }
    } else if (c >= ' ' && c <= '~') {
        s_cmd_buffer[s_cmd_index++] = c;
        putchar(c);
    }
}