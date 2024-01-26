#include "jsonMsg.h"

char string_to_publish[950] = {0};

void set_string(const char *string)
{
    // Ensure string length does not exceed the size of the array - 1
    size_t length = strlen(string);
    if (length < sizeof(string_to_publish))
    {
        strncpy(string_to_publish, string, sizeof(string_to_publish) - 1);
        string_to_publish[sizeof(string_to_publish) - 1] = '\0'; // Ensure null-termination
    }
    else
    {
        // Handle error or truncate the string
        printk("String too long, truncating...\n");
        strncpy(string_to_publish, string, sizeof(string_to_publish) - 1);
        string_to_publish[sizeof(string_to_publish) - 1] = '\0'; // Ensure null-termination
    }
}

const char *get_string()
{
    return string_to_publish;
}