#pragma once
#include <cstddef>

typedef size_t Slot;
typedef size_t CellHandle;
typedef double Time;
typedef int    ParticleID;

#define NS_TO_SECONDS(ns) (double)ns / (double)1000000000
