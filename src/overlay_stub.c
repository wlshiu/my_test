/* Overlay stubs that change the call sequences from
     main -> foo
   to
     main -> __wrap_foo -> __real__foo (foo)
   */

/* These functions will be mapped to the real func1, func2, Foo::foo() and
   Bar::bar(), respectively. */
extern void __real_func1(void);
extern void __real_func2(void);
extern void __real__ZN3Foo3fooEv(void);
extern void __real__ZN3Bar3barEv(void);

extern void load_overlay(int n);

void __wrap_func1(void)
{
    load_overlay(1);
    __real_func1();
}

void __wrap_func2(void)
{
    load_overlay(2);
    __real_func2();
}

/* For C++ symbols we need to find the mangled name to wrap it.  */
void __wrap__ZN3Foo3fooEv(void)
{
    load_overlay(3);
    __real__ZN3Foo3fooEv();
}

/* This is the virtual function. */
void __wrap__ZN3Bar3barEv(void)
{
    /* Foo::foo() and Bar::bar() are defined in the same obj and loaded at the
       same time.  */
    load_overlay(3);  
    __real__ZN3Bar3barEv();
}
