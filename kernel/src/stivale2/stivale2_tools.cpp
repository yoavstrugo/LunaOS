#include <stivale2/stivale2_tools.hpp>
#include <strings.hpp>
#include <stddef.h>

void *stivale2_get_tag(stivale2_struct *stivale2Struct, uint64_t id)
{
    stivale2_tag *current_tag = (stivale2_tag *)stivale2Struct->tags;
    for (;;)
    {
        // If the tag pointer is NULL (end of linked list), we did not find
        // the tag. Return NULL to signal this.
        if (current_tag == NULL)
        {
            return NULL;
        }

        // Check whether the identifier matches. If it does, return a pointer
        // to the matching tag.
        if (current_tag->identifier == id)
        {
            return current_tag;
        }

        // Get a pointer to the next tag in the linked list and repeat.
        current_tag = (stivale2_tag *)current_tag->next;
    }
}

stivale2_module *stivale2_get_module(stivale2_struct_tag_modules *modulesStruct, const char *moduleString)
{
    stivale2_module *current_module = modulesStruct->modules;

    for (;;)
    {
        if (current_module == NULL)
            return NULL;

        if (strcmp(current_module->string, moduleString) == 0)
            return current_module;

        current_module++;
    }
}