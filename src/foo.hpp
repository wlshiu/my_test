struct Foo
{
    // demo for normal member method
    const char* foo();

    // demo for virtual method
    virtual const char* bar();  
};

struct Bar: Foo
{
    const char* bar() override;
};
