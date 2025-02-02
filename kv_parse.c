/**
 * @file kv_parse.c
 * @brief Composible ANSI C Key-Value Parser
 *
 * This file contains a parser function that extracts values associated with keys in a formatted
 * key-value file (e.g., "key=value" or "key: value").
 *
 * Copyright (c) 2025 Brian Khuu
 * MIT licensed
 */
#include <stdbool.h>
#include <stdio.h>

bool kv_parse_next_line(FILE *file, size_t line_count)
{
    char ch = '\0';

    /* Check if first line */
    if (line_count == 0)
    {
        /* Already at next line */
        return true;
    }

    /* Advance to next line */
    ch = getc(file);
    while (ch != EOF)
    {
        if (ch == '\n')
        {
            ch = getc(file);
            if (ch == EOF)
            {
                return false;
            }

            ungetc(ch, file);
            return true;
        }
        ch = getc(file);
    }

    /* File Finished Reading */
    return false;
}

bool kv_parse_check_key(FILE *file, const char *key)
{
    long start_of_line = ftell(file);
    char ch = getc(file);

#ifdef KV_PARSE_WHITESPACE_SKIP
    while (ch == ' ' || ch == '\t')
    {
        ch = getc(file);
    }
#endif

    /* Check For Key */
    for (int i = 0; ch != EOF && key[i] != '\0'; i++, ch = getc(file))
    {
        if (ch != key[i])
        {
            /* End of string. Key was not found */
            fseek(file, start_of_line, SEEK_SET);
            return false;
        }
    }

#ifdef KV_PARSE_WHITESPACE_SKIP
    while (ch == ' ' || ch == '\t')
    {
        ch = getc(file);
    }
#endif

    /* Check For Key Value Delimiter */
    if (ch != '=' && ch != ':')
    {
        fseek(file, start_of_line, SEEK_SET);
        return false;
    }

    /* Key Found. Next position is likely the value */
    return true;
}

size_t kv_parse_get_value(FILE *file, char *value, size_t value_max)
{
    long start_of_value = ftell(file);
    char ch = getc(file);
#ifdef KV_PARSE_WHITESPACE_SKIP
    while (ch == ' ' || ch == '\t')
    {
        ch = getc(file);
    }
#endif

    /* Copy Value To Buffer */
#ifdef KV_PARSE_QUOTED_STRINGS
    int quote = EOF;
    int prev = EOF;
#endif
    for (int i = 0; i < (value_max - 1); ch = getc(file))
    {
        if (ch == EOF || ch == '\r' || ch == '\n')
        {
            /* End Of Line */
            fseek(file, start_of_value, SEEK_SET);
            if (ch == '\n')
            {
                ungetc('\n', file);
            }
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
        else if (quote == EOF && (ch == '\'' || ch == '"'))
        {
            /* Start Of Quoted String */
            quote = ch;
            continue;
        }
        else if (quote != EOF && prev != '\\' && ch == quote)
        {
            /* End Of Quoted String. Return Value */
            fseek(file, start_of_value, SEEK_SET);
            value[i] = '\0';
            return i;
        }
        else if (quote != EOF && prev == '\\' && ch == quote)
        {
            /* Escaped Character In Quoted String */
            value[i - 1] = ch == EOF ? '\0' : ch;
            continue;
        }

        prev = ch;
#endif

        value[i++] = ch == EOF ? '\0' : ch;
    }

    /* Value too large for buffer. Don't return a value. */
    fseek(file, start_of_value, SEEK_SET);
    value[0] = '\0';
    return 0;
}
