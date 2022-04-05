#pragma once

#include <stivale2/stivale2.h>

/**
 * @brief Get the stivale2 tag
 * 
 * @param stivale2_struct The stivale2 struct
 * @param id The tag's id               
 * @return void* A pointer to the tag
 */
void *stivale2_get_tag(stivale2_struct *stivale2Struct, uint64_t id);

/**
 * @brief Get the stivale2 module
 * 
 * @param modules_struct stivale2 module struct
 * @param module_string The module string
 * @return stivale2_module* The module
 */
stivale2_module *stivale2_get_module(stivale2_struct_tag_modules *modulesStruct, const char *moduleString);