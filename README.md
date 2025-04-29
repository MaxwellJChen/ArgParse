# ArgParse – C++ Command Line Parser & Dynamic Dispatcher

Helper class for CLIs with extensive functions and "routing" logic based on user-inputted strings. Stores functions of any type and calls correct function based on user input. Casts arguments to proper types during runtime, allows flags and customizability of argument ordering, and more. Achieved with variadic templates and lambdas.

```add_command(std::vector<std::string> path, void (*func)(A...))```: stores the "path" of strings inputted by the user and accepts a function which returns void but has any argument type
```execute_command(std::vector<std::string> path, std::vector<std::string> args)```: traverses the provided path and executes the intended function after casting the provided arguments to the correct type

Here is a simple example snippet to illustrate usage.
```
// function to add to ArgParse database
void foo(int x, float y, double z) {
  std::cout<<x + y * z<<std::endl;
}

int main() {
  ArgParse ap;
  
  // add the command based on specified path
  ap.add_command<3, int, float, double>({"bar", "baz", "foo"}, foo);

  // traverses the same path again and calls the foo function
  ap.execute_command({"bar", "baz", "foo"}, {"10", "12.3", "30.5013"}); // prints 385.16599
}
```
