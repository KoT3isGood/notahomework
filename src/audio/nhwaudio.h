#pragma once
void InitAudio();
void DeinitAudio();

void* CreateSound(const char* sound);
void* PlaySound(void* sound);

void SetSoundPosition(void* source, void* pos);
void DestroySource(void* source);

void DestroySound(void* sound);