#pragma once

char *GetThreadInfoBuffer();
void InitThreadInfoBuffer();
bool PutStringIntoThreadInfoBuffer(const char *str);
bool PutCharIntoThreadInfoBuffer(char curChar);
void PutXMLEscapedStringIntoThreadInfoBuffer(const char *str);