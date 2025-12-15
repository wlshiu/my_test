// main.c
#include <stdio.h>
#include "foo.hpp"


/* Functions provided by the overlays */
extern "C" {
  extern void func1(void);
  extern void func2(void);
}

int main(void)
{
    printf("Start of main()...\n");
    func1();
    func2();

    /*
     * Call func2() again to demonstrate that we don't need to
     * reload the overlay
     */
    func2();

    func1();

    Foo myfoo;
    printf("%s\n", myfoo.foo());
    printf("%s\n", myfoo.bar());

    Bar mybar;
    printf("%s\n", mybar.foo());
    printf("%s\n", mybar.bar());

    printf("End of main()...\n");

    return 0;
}
