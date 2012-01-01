
/*
 * Maximum number of sprites possible
 */
#define CACHE_SZ 21
typedef struct __sprite_{
	u16 width; // pixels
	u16 height; // pixels
	u32 * data; // height * width
} sprite_t;
/*
 * Maximum sprite file size
 */
#define MAX_FILE_SIZ (1024 * 1024 * 4)
/*
 * Gets the associated sprite with a certain graphics file;
 * if the sprite is already cached then that one is returned;
 * If the sprite is not on cache it is read from disk.
 */
sprite_t * new_sprite(u16 id);
/*
 * Reads a sprite directly from disk without updating/checking the sprite cache
 */
sprite_t * read_bmp24_sprite(const char * path);