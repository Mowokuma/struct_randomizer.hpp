# struct_randomizer.hpp
Struct field permutator and size randomizer using C++26 reflection.

# About 
This simply takes normal structs, permutes all of their fields, and inserts random padding between them. As a result, the offset of each field, as well as the overall size of the struct, is unique to each build. 

As of today, only GCC supports C++26 reflection, so you'll have to wait a bit longer to use this with other compilers. To make it work with GCC, you need GCC 16 and must compile with the `-freflection -std=c++26` flags. 

You can play with it on Compiler Explorer [here](https://godbolt.org/z/odf59nMna).
 
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

# Considerations

**It this any useful?**
  
In some cases, I think, yes. See game hacking, for example: struct offsets are always a more or less stable piece between patches/updates. In   most cases, modders/cheaters don't even bother and just assume that the offset did not change. And because a change is rare, they can just manually update it when it happens. We have a lot of tooling to deal with the bother of updating function RVAs, but barely any to deal with structures changing every update.

Also, Linux, for security reasons, seems to [use](https://lwn.net/Articles/722293/) (or maybe it was just proposed) a struct randomizer too, integrated through a compiler plugin.

**Limitations?**

- You won't be able to default initialize a field marked for shuffling. Considering our previous `Player` class, you won't be able to assign a default value to `_health`, for example.

- The way this is implemented, the generated struct containing the shuffled fields is inherited by the original class/struct. I don't know all the implications of doing it this way, but yeah, it's something to keep in mind.
