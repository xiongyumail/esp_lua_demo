#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_lua.h"
#include "esp_lua_lib.h"
#include "driver/gpio.h"

static const char *TAG = "main";

static int gpio_lua_init(lua_State *L) 
{
    int ret = -1;
    gpio_num_t io = (gpio_num_t)luaL_checknumber(L,1);
    char *mode = luaL_checklstring(L, 2, NULL);

    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;

    if (strcmp(mode, "OUTPUT") == 0) {
        io_conf.mode = GPIO_MODE_OUTPUT;
    } else if (strcmp(mode, "INPUT") == 0) {
        io_conf.mode = GPIO_MODE_INPUT;
    } else {
        io_conf.mode = GPIO_MODE_OUTPUT;
    }
    
    io_conf.pin_bit_mask = 1ULL << io;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    ret = gpio_config(&io_conf);

    if (ret == 0) {
        lua_pushboolean(L, true);
    } else {
        lua_pushboolean(L, false);
    }
    return 1;
}

static int gpio_lua_write(lua_State *L) 
{
    int ret = -1;
    gpio_num_t io = (gpio_num_t)luaL_checknumber(L,1);
    uint32_t level = luaL_checknumber(L,2);

    ret = gpio_set_level(io, level);
    
    if (ret == 0) {
        lua_pushboolean(L, true);
    } else {
        lua_pushboolean(L, false);
    }
    return 1;
}

static const luaL_Reg gpio_lib[] = {
    {"init", gpio_lua_init},
    {"write", gpio_lua_write},
    {NULL, NULL}
};

LUAMOD_API int esp_lib_gpio(lua_State *L) 
{
    luaL_newlib(L, gpio_lib);
    lua_pushstring(L, "0.1.0");
    lua_setfield(L, -2, "_version");
    return 1;
}

static const luaL_Reg mylibs[] = {
    {"sys", esp_lib_sys},
    {"net", esp_lib_net},
    {"web", esp_lib_web},
    {"mqtt", esp_lib_mqtt},
    {"httpd", esp_lib_httpd},
    {"gpio", esp_lib_gpio},
    {NULL, NULL}
};

const char LUA_SCRIPT_INIT[] = " \
assert(sys.init()) \
dofile(\'/lua/init.lua\') \
";

void lua_task(void *arg)
{
    char *ESP_LUA_ARGV[5] = {"./lua", "-i", "-e", LUA_SCRIPT_INIT, NULL}; // enter interactive mode after executing 'script'

    esp_lua_init(NULL, NULL, mylibs);

    while (1) {
        esp_lua_main(4, ESP_LUA_ARGV);
        printf("lua exit\n");
        vTaskDelay(2000 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

void app_main()
{  
    xTaskCreate(lua_task, "lua_task", 10240, NULL, 5, NULL);
}
