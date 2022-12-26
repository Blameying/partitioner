# partitioner

## Introduction
When I wanted to improve the performance of it, I used some tricks which make the logic more complex and leaves some potential bugs.
So I only expose the ratio parameter. As for the coasening size and the counts FM should loop, they are fixed in the program, I will try to change this 
after the submission.

## Usage:
```shell
partitioner 0.5 path/to/100.txt
```
The 0.5 is the ratio.

## Building
```shell
mkdir build && cd build
cmake ..
```
if you want to show more infomation(it's too more)
```
cmake -DDEBUG=1 ..
```


## Dependencies
- cmake
- gcc
- [bitmap library](https://github.com/lemire/EWAHBoolArray) (contained in my source tree)
