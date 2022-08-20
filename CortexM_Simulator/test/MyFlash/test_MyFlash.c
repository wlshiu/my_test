#include <stdio.h>
#include <unistd.h>
#include "unity.h"
#include "MyFlash.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_coflash_given_argv_should_write_elf_path_to_the_text(void)
{
    FILE *file;
    char *filename = "TEST1.txt", *str, buffer[1024], str1[100], str2[100];
    const char *argv[] = { "myFlash.exe",
                           "program",
                           "STM32F429ZI",
                           "C:/Users/Asus/Desktop/CoIDE/workspace/BlinkyLED/Test01/Debug/bin/Test01.elf"
                         };

    coflash(4, argv);

    file = fopen(filename, "r");

    if(file == NULL)
    {
        printf("error: cannot open the file %s\n", filename);
        return;
    }

    str = fgets(buffer, 1024, file);   // fget from the file for test
    sscanf(str, "%s %s", str1, str2);

    TEST_ASSERT_EQUAL_STRING(argv[3], str1);
    TEST_ASSERT_EQUAL_STRING(argv[2], str2);

    // Close the file
    fclose(file);
}

void test_coflash_given_argv_with_different_arrangement_should_write_elf_path_to_the_text(void)
{
    FILE *file;
    char *filename = "TEST1.txt", *str, buffer[1024], str1[100], str2[100];
    const char *argv[] = {"myFlash.exe",
                          "STM32F429ZI",
                          "C:/Users/Asus/Desktop/CoIDE/workspace/BlinkyLED/Test01/Debug/bin/Test01.elf",
                          "program"
                         };

    coflash(4, argv);

    file = fopen(filename, "r");

    if(file == NULL)
    {
        printf("error: cannot open the file %s\n", filename);
        return;
    }

    str = fgets(buffer, 1024, file);   // fget from the file for test
    sscanf(str, "%s %s", str1, str2);

    TEST_ASSERT_EQUAL_STRING(argv[2], str1);
    TEST_ASSERT_EQUAL_STRING(argv[1], str2);

    // Close the file
    fclose(file);
}
