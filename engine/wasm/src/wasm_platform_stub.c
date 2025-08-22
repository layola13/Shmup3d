#include "texture.h"
#include "world.h"
#include "netchannel.h"
#include "sounds.h"

// Global variables stubs
net_channel_t net;

// Function stubs
void SND_InitSoundTrack(char* filename,unsigned int startAt) {}
void SND_StartSoundTrack(void) {}
void SND_StopSoundTrack(void) {}
void Native_UploadScore(int score) {}

void SND_BACKEND_Upload(sound_t* sound, int soundID) {}
void SND_BACKEND_Init(void) {}
void SND_BACKEND_Play(int soundID) {}

void loadNativePNG(texture_t* tex) {}
int dEngine_resize(int width, int height) {return 0;}
