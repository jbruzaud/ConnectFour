cmake_minimum_required(VERSION 3.9)
project(P4Client C)

set(CMAKE_C_STANDARD 99)

add_executable(P4Client main.c main.h affichage.c affichage.h InputManage.c InputManage.h)