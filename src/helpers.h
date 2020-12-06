#pragma once

#define bitValue(x) (1 << x)
#define setBit(P, B) (P |= bitValue(B))
#define clearBit(P, B) (P &= ~bitValue(B))
#define toggleBit(P, B) (P ^= bitValue(B))
