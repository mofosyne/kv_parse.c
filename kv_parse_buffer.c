/**
 * @file kv_parse_buffer.c
 * @brief Composible ANSI C Key-Value Parser
 *
 * This file contains a parser function that extracts values associated with keys in a formatted
 * key-value file (e.g., "key=value" or "key: value").
 *
 * Copyright (c) 2025 Brian Khuu
 * MIT licensed
 */
#include <stdbool.h>
#include <stddef.h>

char *kv_parse_buffer_next_line(char *str, size_t line_count)
{
    char ch = '\0';

    /* Check if first line */
    if (line_count == 0)
    {
        /* Already at next line */
        return str;
    }

    /* Advance to next line */
    ch = *str;
    str++;
    while (ch != '\0')
    {
        if (ch == '\n')
        {
            ch = *str;
            return (ch == '\0') ? NULL : str;
        }
        ch = *str;
        str++;
    }

    /* File Finished Reading */
    return NULL;
}

char *kv_parse_buffer_check_key(char *str, const char *key)
{
#ifdef KV_PARSE_WHITESPACE_SKIP
    while (*str == ' ' || *str == '\t')
    {
        str++;
    }
#endif

    /* Check For Key */
    for (int i = 0; *str != '\0' && key[i] != '\0'; i++, str++)
    {
        if (*str != key[i])
        {
            /* End of string. Key was not found */
            return NULL;
        }
    }

#ifdef KV_PARSE_WHITESPACE_SKIP
    while (*str == ' ' || *str == '\t')
    {
        str++;
    }
#endif

    /* Check For Key Value Delimiter */
    if (*str != '=' && *str != ':')
    {
        return NULL;
    }

    /* Key Found. Next position is likely the value */
    str++;
    return str;
}

size_t kv_parse_buffer_get_value(char *str, char *value, size_t value_max)
{
#ifdef KV_PARSE_WHITESPACE_SKIP
    while (*str == ' ' || *str == '\t')
    {
        str++;
    }
#endif

    /* Copy Value To Buffer */
#ifdef KV_PARSE_QUOTED_STRINGS
    int quote = '\0';
    int prev = '\0';
#endif
    for (int i = 0; i < (value_max - 1); str++)
    {
        if (*str == '\0' || *str == '\r' || *str == '\n')
        {
            /* End Of Line */
            value[i] = '\0';
#ifdef KV_PARSE_WHITESPACE_SKIP
            while (i > 0 && (value[i - 1] == ' ' || value[i - 1] == '\t'))
            {
                i--;
                value[i] = '\0';
            }
#endif
            return i;
        }
#ifdef KV_PARSE_QUOTED_STRINGS
        else if (quote == '\0' && (*str == '\'' || *str == '"'))
        {
            /* Start Of Quoted String */
            quote = *str;
            continue;
        }
        else if (quote != '\0' && prev != '\\' && *str == quote)
        {
            /* End Of Quoted String. Return Value */
            value[i] = '\0';
            return i;
        }
        else if (quote != '\0' && prev == '\\' && *str == quote)
        {
            /* Escaped Character In Quoted String */
            value[i - 1] = *str == '\0' ? '\0' : *str;
            continue;
        }

        prev = *str;
#endif

        value[i++] = *str == '\0' ? '\0' : *str;
    }

    /* Value too large for buffer. Don't return a value. */
    value[0] = '\0';
    return 0;
}
