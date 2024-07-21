#ifndef TOKEN_H
#define TOKEN_H

#include "game.h"

typedef int Token;

Token getToken(int shape, int color);

int getTokenColor(Token t);

int getTokenShape(Token t);

int tokensAreSameColor(Token t1, Token t2);

int tokensAreSameShape(Token t1, Token t2);

#endif