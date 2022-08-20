#include "apex_cpu.h"


// forward declarations
void generate_prompt(APEX_CPU *cpu, const char *filename);
void clear_buffer();

int main(int argc, char const *argv[])
{
    APEX_CPU *cpu = NULL;

    if (argc < 2)
    {
        fprintf(stderr, "APEX_Help: Usage %s <input_file>\n", argv[0]);
        exit(1);
    }

    printf("----------------------------------------------\n");
    printf("  APEX Simulator v2.0\n");
    printf("----------------------------------------------\n");
    printf("  commands: [init | initialize]\n"
           "            [sim|Simulate <count>]\n"
           "            [d|Display]\n"
           "            [mem <address>] [n]\n"
           "            [q (exit)]\n");
    printf("----------------------------------------------\n");

    generate_prompt(cpu, argv[1]);

    if (cpu != NULL)
        APEX_cpu_stop(cpu);

    return 0;
}

/**
 * Method to return user prompt, parse user input and call appropriate methods in apex_cpu.c & apex_cpu_b.c
 *
 * @param cpu pointer to the current instance of cpu
 * @param filename name of the input file
 */
void generate_prompt(APEX_CPU *cpu, const char *filename)
{
    char user_prompt_val[50];
    int count, address;

    while (TRUE)
    {
        printf("\nAPEX:> ");
        scanf("%s", user_prompt_val);

        if( strlen(user_prompt_val) == 0 )
            continue;

        if( strncasecmp(user_prompt_val, "Q", 1) == 0 )
        {
            printf("APEX_CPU: Simulation Stopped\n");
            if (cpu != NULL) APEX_cpu_stop(cpu);
            break;
        }
        else if( strncasecmp(user_prompt_val, "init", strlen("init")) == 0 ||
                 strncasecmp(user_prompt_val, "initialize", strlen("initialize")) == 0 )
        {
            cpu = APEX_cpu_init(filename);
            if (!cpu)
            {
                fprintf(stderr, "APEX_Error: Unable to initialize CPU\n");
                exit(1);
            }
        }
        else if( strncasecmp(user_prompt_val, "n", strlen("n")) == 0 ||
                 strncasecmp(user_prompt_val, "next", strlen("next")) == 0 )
        {
            APEX_cpu_run(cpu, 0, true);
        }
        else
        {
            if( strncasecmp(user_prompt_val, "d", strlen("d")) == 0 ||
                strncasecmp(user_prompt_val, "display", strlen("display")) == 0 )
            {
                display(cpu);
                clear_buffer();
            }
            else if( strcmp(user_prompt_val, "print_rob") == 0 ||
                     strcmp(user_prompt_val, "PrintROB") == 0)
            {
                print_reorder_buffer(cpu);
                clear_buffer();
            }
            else if( strcmp(user_prompt_val, "print_iq") == 0 ||
                    strcmp(user_prompt_val, "PrintIQ") == 0 )
            {
                print_issue_queue(cpu);
                clear_buffer();

            }
            else if( strncasecmp(user_prompt_val, "sim", strlen("sim")) == 0 ||
                     strncasecmp(user_prompt_val, "simulate", strlen("simulate")) == 0 )
            {
                printf("\n  Set cycles> ");
                scanf("%d", &count);
                APEX_cpu_run(cpu, count, true);
                clear_buffer();

            }
            else if( strncasecmp(user_prompt_val, "mem", strlen("mem")) == 0 )
            {
                printf("\n  Set address> 0x");
                scanf("%x", &address);
                show_mem(cpu, address);
                clear_buffer();

            }
            else
            {
                clear_buffer();
                printf("Invalid Command: [ %s ] \n\n--------------------------------------------------------------------",
                       user_prompt_val);
                printf("\n commands supported:\n");
                printf("--------------------------------------------------------------------\n");
                printf("   [initialize | init]     - to initialize cpu\n"
                       "   [sim|Simulate <count>]  - to simulate <count> cycles\n"
                       "   [d|Display]             - to display stage contents\n"
                       "   [mem <address>]         - to show contents in memory <address>\n"
                       "   [PrintROB | print_rob]  - to print contents of ROB\n"
                       "   [PrintIQ | print_iq]    - to print contents of Issue Queue\n"
                       "   [n|next]                - proceed by one cycle\n");
                printf("--------------------------------------------------------------------\n");
            }
        }

        if( cpu != NULL )
        {
            if( cpu->insn_completed == cpu->code_memory_size &&
                (strncasecmp(user_prompt_val, "simulate", strlen("simulate")) == 0 ||
                 strncasecmp(user_prompt_val, "next", strlen("next")) == 0 ||
                 strncasecmp(user_prompt_val, "n", strlen("n")) == 0) )
            {
                printf("\nAPEX_CPU: Simulation Complete, cycles = %d instructions retired = %d\n",
                       cpu->clock, cpu->insn_completed);
            }
        }
    }
}

/**
 * Method to remove extraneous characters after necessary user input has been read by scanf()
 */
void clear_buffer()
{
    while (getchar() != '\n') {}
}

