

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "regex.h"



#if 1
char* extract_value(char *string, char *pattern)
{
    int begin, end;
    char *word = NULL;

    regex_t     regex;

    /**
     *  [0] is the all groups of matched string and
     *  [others] are the target group which are used `(...)` express
     */
    regmatch_t  match[3];

    regcomp(&regex, pattern, REG_EXTENDED);
    if( (regexec(&regex, string, 3, match, 0)) == 0 )
    {
        word = malloc(256);

        for(int i = 0; i < 3; i++)
        {
            char    str[256] = {0};

            begin  = (int)match[i].rm_so;
            end    = (int)match[i].rm_eo;

            memset(word, 0x0, 256);
            memcpy(word, &string[begin], end - begin);

            memcpy(str, &string[begin], end - begin);
            printf("[match %d]: %s\n", i, str);
        }

    }
    regfree(&regex);
    return word;
}

int main()
{
//    char *match = extract_value("xoffset=0     yoffset=-4    xadvance=5", "yoffset=([-0-9]+)");
    char *match = extract_value("xoffset=0     yoffset=-4    xadvance=5", "yoffset=([-0-9]+)\\s+xadvance=([-0-9]+)");
//    printf("%s\n", match);
    return 0;
}
#else


int main()
{
    regex_t regex;
    int return_value;
    char error_message[100];

    // Target string to test
    const char *text = "user@example.com";

    /**
     *  POSIX Extended Regular Expression pattern
     *  ps. Not support `\d` and `\w` in POSIX Regular,
     *      use `[a-z0-9]` to replace
     */
    const char *pattern = "^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$";

    // 1. Compile the regular expression
    return_value = regcomp(&regex, pattern, REG_EXTENDED | REG_ICASE);
    if (return_value != 0)
    {
        fprintf(stderr, "Could not compile regex\n");
        return 1;
    }

    const size_t    nmatch = 1;
    regmatch_t      matchptr[1] = {0};

    // 2. Execute the regular expression match
    return_value = regexec(&regex, text, nmatch, matchptr, 0);

    // 3. Evaluate the result
    if (return_value == 0)
    {
        printf("Match found.\n");

        for (int i = matchptr[0].rm_so; i < matchptr[0].rm_eo; i++)
        {
            printf("%c", text[i]);
        }
    }
    else if (return_value == REG_NOMATCH)
    {
        printf("No match found for '%s'.\n", text);
    }
    else
    {
        regerror(return_value, &regex, error_message, sizeof(error_message));
        fprintf(stderr, "Regex match failed: %s\n", error_message);
        return 1;
    }

    // 4. Free allocated memory used by the regex structure
    regfree(&regex);

    return 0;
}
#endif // 1
