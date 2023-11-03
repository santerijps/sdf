# Schematic Data Format (`SDF`)

`SDF` is a data format designed for config files, as well as data transfer.
With an easy and familiar syntax, you can write readable, unambiguous config files and data files.

## Syntax

```sdf
# Simple key-value pairs are separated by the equals sign
name = Alice
age = 35

# Objects
person {
  name = Alice
  age = 35
}

# Lists
fruits [
  apple
  banana
  cantaloupe
]

# List of objects
people [
  {name = Alice; age = 35}
  {name = Bob;   age = 55}
]
```
