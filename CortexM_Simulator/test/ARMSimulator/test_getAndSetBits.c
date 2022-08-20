#include "unity.h"
#include "getAndSetBits.h"
#include "getMask.h"
#include <stdint.h>


void setUp(void)
{
}

void tearDown(void)
{
}

/*
 *  0100  0110  1110  1000  0001  1110  1010  0111  --> this is a 32 bits integer
 *  ^ ^
 *  | |
 * 31 28
 *
 */
void test_getBits_given_0x46E81EA7_higherRange_31_lowerRange_28_should_return_0x04()
{
    uint32_t dummy = 0x46E81EA7;

    uint32_t result = getBits(dummy, 31, 28);
    TEST_ASSERT_EQUAL(0x04, result);
}



/*
 *  0100  0110  1110  1000  0001  1110  1010  0111  --> this is a 32 bits integer
 *     ^         ^
 *     |         |
 *    27        21
 *
 */
void test_getBits_given_0x46E81EA7_higherRange_27_lowerRange_21_should_return_0x37()
{
    uint32_t dummy = 0x46E81EA7;

    uint32_t result = getBits(dummy, 27, 21);
    TEST_ASSERT_EQUAL(0x37, result);
}



/*
 *  0100  0110  1110  1000  0001  1110  1010  0111  --> this is a 32 bits integer
 *     ^        ^
 *    |         |
 *   27        22
 *
 */
void test_getBits_given_0x46E81EA7_higherRange_27_lowerRange_22_should_return_0x1b()
{
    uint32_t dummy = 0x46E81EA7;

    uint32_t result = getBits(dummy, 27, 22);
    TEST_ASSERT_EQUAL(0x1b, result);
}


/*
 *  0100  0110  1110  1000  0001  1110  1010  0111  --> this is a 32 bits integer
 *     ^     ^
 *    |      |
 *   27     23
 *
 */
void test_getBits_given_0x46E81EA7_higherRange_27_lowerRange_23_should_return_0x0d()
{
    uint32_t dummy = 0x46E81EA7;

    uint32_t result = getBits(dummy, 27, 23);
    TEST_ASSERT_EQUAL(0x0d, result);
}



/*
 *  0100  0110  1110  1000  0001  1110  1010  0111  --> this is a 32 bits integer
 *                                 ^       ^
 *                                |        |
 *                               10        4
 *
 */
void test_getBits_given_0x46E81EA7_higherRange_10_lowerRange_4_should_return_0x6a()
{
    uint32_t dummy = 0x46E81EA7;

    uint32_t result = getBits(dummy, 10, 4);
    TEST_ASSERT_EQUAL(0x6a, result);
}



/*
 *  0100  0110  1110  1000  0001  1110  1010  0111  --> this is a 32 bits integer
 *                                  ^      ^
 *                                  |      |
 *                                 9       4
 *
 */
void test_getBits_given_0x46E81EA7_higherRange_9_lowerRange_4_should_return_0x2a()
{
    uint32_t dummy = 0x46E81EA7;

    uint32_t result = getBits(dummy, 9, 4);
    TEST_ASSERT_EQUAL(0x2a, result);
}



/*
 *  0100  0110  1110  1000  0001  1110  1010  0111  --> this is a 32 bits integer
 *                                   ^     ^
 *                                   |     |
 *                                  8      4
 *
 */
void test_getBits_given_0x46E81EA7_higherRange_9_lowerRange_4_should_return_0x0a()
{
    uint32_t dummy = 0x46E81EA7;

    uint32_t result = getBits(dummy, 8, 4);
    TEST_ASSERT_EQUAL(0x0a, result);
}



//boundary test
/*
 *  0100  0110  1110  1000  0001  1110  1010  0111  --> this is a 32 bits integer
 *                                              ^^
 *                                              ||
 *                                             1 0
 *
 */
void test_getBits_given_0x46E81EA7_higherRange_1_lowerRange_0_should_return_0x03()
{
    uint32_t dummy = 0x46E81EA7;

    uint32_t result = getBits(dummy, 1, 0);
    TEST_ASSERT_EQUAL(0x03, result);
}


//boundary test, single bit
/*
 *  0100  0110  1110  1000  0001  1110  1010  0111  --> this is a 32 bits integer
 *                ^
 *                |
 *               21
 *
 */
void test_getBits_given_0x46E81EA7_higherRange_21_lowerRange_21_should_return_0x01()
{
    uint32_t dummy = 0x46E81EA7;

    uint32_t result = getBits(dummy, 21, 21);
    TEST_ASSERT_EQUAL(0x01, result);
}



/*
 *   0100  0110  1110  1000  0001  1110  1010  0111  --> this is a 32 bits integer
 *                ^    ^
 *                |    |
 *               21    18
 *   Changed:     11  01
 *
 */
void test_setBits_given_0x46E81EA7_higherRange_21_lowerRange_18_and_input_0b1101_should_return_0x46f41ea7()
{
    uint32_t input = 0b1101;
    uint32_t setTarget = 0x46E81EA7;

    uint32_t result = setBits(setTarget, input, 21, 18);

    TEST_ASSERT_EQUAL(0x46f41ea7, result);
}


/*
 *  0100  0110  1110  1000  0001  1110  1010  0111  --> this is a 32 bits integer
 *                    ^             ^
 *                    |             |
 *                   19            9
 *   Changed:         1111  1010  010
 *
 */
void test_setBits_given_0x46E81EA7_higherRange_19_lowerRange_9_and_input_0b11111010010_should_return_0x46efa4a7()
{
    uint32_t input = 0b11111010010;
    uint32_t setTarget = 0x46E81EA7;

    uint32_t result = setBits(setTarget, input, 19, 9);

    TEST_ASSERT_EQUAL(0x46efa4a7, result);
}


//boundary test
/*
 *  0100  0110  1110  1000  0001  1110  1010  0111  --> this is a 32 bits integer
 *                    ^
 *                    |
 *                   19
 *   Changed:         0
 *
 */
void test_setBits_given_0x46E81EA7_higherRange_19_lowerRange_19_and_input_0b0_should_return_0x46E01EA7()
{
    uint32_t input = 0b0;
    uint32_t setTarget = 0x46E81EA7;

    uint32_t result = setBits(setTarget, input, 19, 19);

    TEST_ASSERT_EQUAL(0x46E01EA7, result);
}