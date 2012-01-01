#include "common.h"
static u8 buf[MAX_FILE_SIZ];
sprite_t * read_bmp24_sprite(const char * path){
	assert(path != NULL);
	FILE * fd = fopen(path, "rb");
	assert(fd != NULL);
	u32 bread = fread(buf, sizeof(u8), MAX_FILE_SIZ, fd);
	assert(bread > 0);
	assert(ferror(fd) == 0);
	fclose(fd);
	assert((buf[0] == 'B') && (buf[1] == 'M'));
	u32 offset = ((u32 *)(buf + 0xa))[0];
	sprite_t * sp = (sprite_t *)malloc(sizeof(sprite_t));
	assert(sp != NULL);
	sp->width = ((u32 *)(buf + 0x12))[0];
	sp->height = ((u32 *)(buf + 0x16))[0];;
	sp->data = (u32 *)malloc(sp->width * sp->height * sizeof(u32));
	assert(sp->data != NULL);
	u8 * data = buf + offset;
	u8 padding_bytes = ((sp->width * 3) % 4) == 0 ? 0 : 4 - ((sp->width * 3) % 4);
	u32 k = 0;
	for(u16 i = 0; i < sp->height; ++i, k += padding_bytes)
		for(u16 j = 0; j < sp->width; ++j){
			u32 blue = (u32)data[k++];
			u32 green = (u32)data[k++];
			u32 red = (u32)data[k++];
			sp->data[(sp->height - 1 - i) * sp->width + j] = (red << 16) | (green << 8) | blue;
		}
	return sp;
}
static sprite_t * _read_bmp24_sprite(const char * path){
	FILE * fd = fopen(path, "rb");
	assert(fd != NULL);
	u32 bread = fread(buf, sizeof(u8), MAX_FILE_SIZ, fd);
	assert(bread > 0);
	assert(ferror(fd) == 0);
	fclose(fd);
	assert((buf[0] == 'B') && (buf[1] == 'M'));
	u32 offset = ((u32 *)(buf + 0xa))[0];
	sprite_t * sp = (sprite_t *)malloc(sizeof(sprite_t));
	assert(sp != NULL);
	sp->width = ((u32 *)(buf + 0x12))[0];
	sp->height = ((u32 *)(buf + 0x16))[0];;
	sp->data = (u32 *)malloc(sp->width * sp->height * sizeof(u32));
	assert(sp->data != NULL);
	u8 * data = buf + offset;
	u8 padding_bytes = ((sp->width * 3) % 4) == 0 ? 0 : 4 - ((sp->width * 3) % 4);
	u32 k = 0;
	for(u16 i = 0; i < sp->height; ++i, k += padding_bytes)
		for(u16 j = 0; j < sp->width; ++j){
			u32 blue = (u32)data[k++];
			u32 green = (u32)data[k++];
			u32 red = (u32)data[k++];
			sp->data[(sp->height - 1 - i) * sp->width + j] = (red << 16) | (green << 8) | blue;
		}
	return sp;
}
static char strbuf[1024];
static sprite_t * cache[CACHE_SZ];
sprite_t * new_sprite(u16 id){
	assert(id < CACHE_SZ);
	if(cache[id] == NULL){
		sprintf(strbuf, "gfx/%u.bmp", id);
		cache[id] = _read_bmp24_sprite(strbuf);
	}
	return cache[id];
}