#include "foo.hpp"

const char* Foo::foo() {
  return "Foo:foo()";
}

const char* Foo::bar() {
  return "Foo:bar()";
}

const char* Bar::bar() {
  return "Bar::bar()";
}
