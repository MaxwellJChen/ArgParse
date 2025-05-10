# CLI Dispatcher: C++ CLI Argument Parser & Function Dispatcher

## Capabilities
Typically, extensive if statements and switch cases are needed to route to the correct command in CLI development, after which, error handling and casting must also be programmed by the developer. Dispatcher is a helper class which completely eliminates this overhead by handling command execution and argument parsing under-the-hood. 
- Calls the correct method based on inputted strings
- Casts method arguments to any user-defined type
- Accepts any method as long as static or free methods with void return type
- Supports aliasing and customizable error messages
- Stored in a single header file
- Supports flags and default argument values

## Usage

Here is an example to illustrate how Dispatcher can simplify CLI development. 

Previously, calling the correct function based on ```argc``` and ```argv``` values required extensive routing with comparisons and casting all done by the developer. This can lead to hard-to-maintain code. 

In the simplified example below, ```void foo(double)```, ```void bar(float, std::string)```, and ```void baz(custom_type_t)``` are placeholders for real functions a developer might want to call in a CLI. There are 3 commands in this hypothetical CLI, but the code is over 50 lines. Scaling to 10s or even 100s of commands in this way is undesirable.

```
int main(int argc, char* argv) {
  if(argc < 2) {
    std::cout<<"Invalid argument count"<<std::endl;
    return 1;
  }

  if(!strcmp(argv[1], "foo") || !strcmp(argv[1], "f")) { // might want to alias foo as f
    if(argc < 3) {
      std::cout<<"Missing additional argument for foo"<<std::endl;
      return 0;
    }

    try {
      foo(std::strtod(argv[1]));
    }
    catch(...) {
      std::cout<<"Command failed"<<std::endl;
    }
  }
  else if(argc > 3 && (!strcmp(argv[1], "bar") || !strcmp(argv[1], "b")) && !strcmp(argv[2], "test")) {
    if(argc < 5) {
      std::cout<<"Missing additional arguments for bar"<<std::endl;
      return 0;
    }

    try {
      bar(std::strtof(argv[3]), std::string(argv[4]));
    }
    catch(...) {
      std::cout<<"Command failed"<<std::endl;
    }
  }
  else if(!strcmp(argv[1], "baz")) {
    if(argc < 3) {
      std::cout<<"Missing additional arguments for baz"<<std::endl;
      return 0;
    }

    try {
      baz(convert_to_custom_type(argv[2]); // baz accepts a custom user-defined type
    }
    catch(...) {
      "std::cout<<"Baz failed"<<std::endl;
    }
  }

  std::cout<<"Command not found"<<std::endl;
}
```

To create similar behavior with Dispatcher, the result is simpler and more readable.
```
int main(int argc, char* argv) {
  Dispatcher d;

  d.add_command({"foo"}, foo);
  d.add_invalid_args_message({"foo"}, "Invalid arguments for foo");
  d.add_alias({"foo"}, "f");

  d.add_command({"bar", "test"}, bar);
  d.add_invalid_args_message({"bar", "test"}, "Command failed");
  d.add_alias({"bar"}, "b");

  d.add_conversion(convert_to_custom_type); // add the custom conversion to Dispatcher
  d.add_command({"baz"}, baz);
  d.add_invalid_args_message({"baz"}, "Baz failed");

  d.execute_command(argc, argv);
}
```

The core of Dispatcher is the idea of a "path". Every command a CLI accepts consists of a path, then a set of arguments and flags. The "path" indicates the right method to call. E.g., ```git remote add``` is a path followed by 2 arguments. Similarly, ```ls``` is a path optionally followed by no arguments. In every CLI, paths between different commands are distinct and can be used to uniquely identify a command.

```void add_command(std::vector<std::string> path, void (*func)(A...))```: Add a method to the Dispatcher object. Users must specify the path needed to reach the function along with a pointer to the function itself. Again, the function must return void and be free or static.

```void execute_command(int argc, char* argv[])```: Executes function based on CLI input.

```void add_alias(std::vector<std::string> path, std::string alias)```: Add an alias for the final string in a path.

```void add_invalid_args_message(std::vector<std::string> path, std::string msg)```: Add an error message if the method defined by the path fails to cast its arguments.

```void add_invalid_command_message(std::vector<std::string> path, std::string msg)```: Add an error message if there is no method at the defined path.

```void add_conversion(std::function<T(std::string)> convert)```: Add a conversion for a custom, user-defined type.

```void add_flag(std::vector<std::string> path, int idx, std::string flag)```: Adds a flag specifying a position.

```void add_default(std::vector<std::string> path, int idx, std::string default_value)```: Adds a default value for a certain argument in a command.

## Under-The-Hood
The main obstacle to Dispatcher is finding a way to store heterogenous functions, accepting potentially any type, and properly casting input strings to the correct arguments (or failing). This was achieved with ```std::any```, variadic templates, and lambdas. Custom conversions are implemented with a hash map which accepts a ```std::type_index``` object and returns the proper conversion.

Paths themselves are represented in a tree. Oftentimes, the same path members are used to specify functions with similar usage. Thus, it is logical to represent the CLI as a tree, traverse down branches based on the input path, and then execute the function at the discovered node. Each node contains metadata about error messages, methods to execute, flags and default values, etc.

All code is in the ```dispatcher.h``` file.
