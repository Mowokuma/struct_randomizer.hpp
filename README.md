# struct_randomizer.hpp
Struct field permutator and size randomizer using C++26 reflection.

# About 
This simply takes normal structs, permutes all of their fields, and inserts random padding between them. As a result, the offset of each field, as well as the overall size of the struct, is unique to each build. 

As of today, only GCC supports C++26 reflection, so you'll have to wait a bit longer to use this with other compilers. To make it work with GCC, you need GCC 16 and must compile with the `-freflection -std=c++26` flags. 

You can play with it on Compiler Explorer (here)[https://godbolt.org/z/odf59nMna].
 
# Use

- `P_PRIVATE` to declare private fields.
- `P_PUBLIC` to declare public fields.
- `P_PROTECTED` to declare protected fields.
- `F(type, name)` to define a field. 

Example: 
```cpp
class Player : 
            P_PRIVATE(
                F(int, _x),
                F(int, _y),
                F(int, _z),
                F(int, _health),
                F(int, _mana)
            )
{
    public:
        Player() = default;

        int& x() { return _x; }
        int& y() { return _y; }
        int& z() { return _z; }

        int& health() { return _health; }
        int& mana()   { return _mana;   }
};
```
