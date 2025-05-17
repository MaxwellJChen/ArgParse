# Dispatcher: C++ CLI Argument Parser & Function Dispatcher

## Capabilities
Dispatcher is a helper class for CLI development which handles command execution and argument parsing under-the-hood. Dispatcher completely eliminates the extensive if statements and switch cases typically required to route to the correct command, in addition to the error handling and argument conversions afterwards. Dispatcher requires C++17 or higher.
- Calls the correct method based CLI input
- Casts method arguments to any user-defined type
- Can execute any method as long as static or free with void return type
- Supports aliasing
- Supports flags
- Supports default argument values
- Supports custom error messages for different error scenarios
- Stored in a single header file

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

The core of Dispatcher is the idea of a "path". Every command a CLI accepts consists of a path, then a set of arguments and flags. The "path" indicates the right method to call. E.g., ```git remote add``` is a path followed by 2 arguments. Similarly, ```ls``` is a path optionally followed by no arguments. In every CLI, paths between different commands are distinct and can be used to uniquely identify a command. Dispatcher takes advantage of this by representing paths in a tree and routing between commands automatically.

## Interface
```template<typename ...Args> void add_command(const std::vector<std::string> path, void (*func)(Args...))```: Adds a new command specified by a path.
- ```...Args```: A parameter pack indicating the argument types for the inputted command.
- ```const std::vector<std::string> path```: The path specifying the new command.
- ```void (*func)(Args...)```: The function to add. Must return void and be free or static.
<br>

```void execute_command(int argc, char* argv[])```
- ```int argc```: Equivalent to typical ```argc``` parameter in normal CLIs.
- ```char* argv[]```: Equivalent to typical ```argv``` parameter in normal CLIs.
<br>

```template<typename T> void add_conversion(std::function<T(std::string)> convert)```: Adds a new type conversion for custom types. Must be called when inputted functions run on non-default types. Default types are ```int```, ```float```, ```double```, and ```std::string```.
- ```T```: Parameter indicating the return type of the function.
- ```std::function<T(std::string)> convert```: The function which converts from a string to the desired type.
<br>

```void add_alias(std::vector<std::string> path, std::string alias)```: Adds an alias for the final name in a provided path. The path must already exist before execution.
- ```std::vector<std::string> path```: The path to alias.
- ```std::string alias```: The value which can be used in place of the final name in the provided path.
<br>

```void add_positional_flag(std::vector<std::string> path, int idx, std::string flag)```: Adds a positional flag for a command specified by a path.
- ```std::vector<std::string> path```: The path to add the flag to.
- ```int idx```: The 0-index of the argument the flag refers to.
- ```std::string flag```: The flag to add.
<br>

```template<typename T> void add_value_flag(std::vector<std::string> path, int idx, std::string flag, T value)```: Adds a value flag for a command specified by a path.
- ```T```: The type of the value which the flag specifies.
- ```std::vector<std::string> path```: The path to add the flag to.
- ```int idx```: The 0-index of the argument the flag refers to.
- ```std::string flag```: The flag to add.
- ```T value```: The value which the flag specifies.
<br>

```template<typename T> void add_default(std::vector<std::string> path, int idx, T def)```: Adds a default value for a specific argument in a command specified by a path. Default arguments are used when no value is specified for an argument for a command.
- ```T```: The type of the default value used.
- ```std::vector<std::string> path```: The path to add the default value to.
- ```int idx```: The 0-index of the argument the default value can replace.
- ```T def```: The default value added.
<br>

```void add_specific_invalid_args_message(std::vector<std::string> path, std::string msg)```: Adds a message to print when invalid arguments are provided to the command at the specified path.
- ```std::vector<std::string> path```: The path where the message is to be added.
- ```std::string msg```: The message which is added to the command.
<br>

```void add_specific_invalid_command_message(std::vector<std::string> path, std::string msg)```: Adds a message to print when the provided path does not specify any valid command.
- ```std::vector<std::string> path```: The path where the message is to be added.
- ```std::string msg```: The message which is added to the command.
<br>

```void add_default_invalid_args_message(std::string msg)```: Adds a message to print when invalid arguments are passed in general.
- ```std::string msg```: The message to print.
<br>

```void add_default_invalid_command_message(std::string msg)```: Adds a message to print when any provided path does not refer to a valid command.
- ```std::string msg```: The message to print.
<br>

```void add_specific_invalid_args_func(std::vector<std::string> path, InvalidArgsFunc func)```: Adds a function to run when invalid arguments are provided to the command at the specified path.
- ```std::vector<std::string> path```: The path where the function is to be added.
- ```InvalidArgsFunc func```: The function which is added to the command.
<br>

```void add_specific_invalid_command_func(std::vector<std::string> path, InvalidCommandFunc func)```: Adds a function to run when the provided path does not specify any valid command.
- ```std::vector<std::string> path```: The path where the function is to be added.
- ```InvalidCommandFunc func```: The function which is added to the command.
<br>

```void add_default_invalid_args_func(InvalidArgsFunc func)```: Adds a function to run when invalid arguments are passed in general.
- ```InvalidArgsFunc func```: The function to run.
<br>

```void add_default_invalid_command_func(InvalidCommandFunc func)```: Adds a function to run when any provided path does not refer to a valid command.
- ```InvalidCommandFunc func```: The function to run.
<br>

```void set_arg_name(std::vector<std::string> path, int idx, std::string name)```: Sets the name of a specified argument.
- ```std::vector<std::string> path```: The path where the argument is stored.
- ```int idx```: The 0-index of the argument to name.
- ```std::string name```: The name to specify the argument.

## Under-The-Hood
The main obstacle to Dispatcher is finding a way to store heterogenous functions, accepting potentially any type, and properly casting input strings to the correct arguments (or failing). This was achieved with ```std::any```, variadic templates, and lambdas. Custom conversions are implemented with a hash map which accepts a ```std::type_index``` object and returns the proper conversion.

Paths themselves are represented in a tree. Oftentimes, the same path members are used to specify functions with similar usage. Thus, it is logical to represent the CLI as a tree, traverse down branches based on the input path, and then execute the function at the discovered node. Each node contains metadata about error messages, methods to execute, flags and default values, etc.

All code is in the ```dispatcher.h``` file.
