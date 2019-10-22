# Simple Shell
## Compile
```bash 
osh> g++ src/prog.cpp -o prog
osh> ./prog 
```
## Features
### Overview
> Example
```bash 
osh> ls & la & ll &
osh> clear
osh> exit
```
### History
> Example
```bash 
osh> !!
osh> history
```
### Redirecting I-O
> Example
```bash 
osh> cat prog.cpp > ex.txt
osh> sort < ex.txt
```
### Pipe
> Example
```bash
osh> cat prog.cpp | grep return
```
