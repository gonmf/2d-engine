/*
 * PIXEL MASKS
 * These are interpolated with the RGB value of each pixel to determine the
 * behaviour of paint jobs on the same area. Transparency just skips the pixel.
 * Collision masks if together equal a collision mask from the registered engine
 * combinations then the associated callback function is invoked.
 */
#define COLLISION0 1 << 0
#define COLLISION1 1 << 1
#define COLLISION2 1 << 2
#define COLLISION3 1 << 3
#define COLLISION4 1 << 4
#define COLLISION5 1 << 5
#define COLLISION6 1 << 6
#define COLLISION7 1 << 7
#define COLLISION8 1 << 8
#define COLLISION9 1 << 9
// ... and so on
/*
 * Transparent color
 */
#define TRANSPARENT 0x00ff00ff
/*
 * Background when background sprite is not defined
 */
#define BACKGROUND 0x00100000
/*
 * sprite_t queue structue
 */
typedef struct __sprite_stack_{
	struct __sprite_stack_ * next;
	sprite_t * sprite;
} sprite_stack_t;
/*
 * Max number of object structures cached in memory to prevent malloc/free lag
 */
#define MAX_OBJECT_POOL 200
typedef struct __object_{
	struct __object_ * next; // For visibility order
	u32 id; // Unmanaged ID
	u32 mask; // Masks to apply for collision detection on painting
	sprite_stack_t * sprites; // Might be sprite queue
	i16 left;
	i16 top;
	u8 keep_alive; // whether object should disappear if it leaves the screen
	u8 dispose_of; // whether object should be disposed of ASAP
	u16 age; // in frames
	u16 start_left;
	u16 start_top;
	u8 smudge;
	void (* func)(struct __object_ * obj); // Callback on engine step
} object_t;
/*
 * For internal use of the engine only
 */
typedef struct __collision_{
	struct __collision_ * next;
	u32 mask_combination;
	void (* func)(object_t *, object_t *);	
} collision_t;
typedef struct __engine_{
	Display * dsp;
	Window win;
	GC gc;
	Pixmap double_buffer;
	sprite_t * background; // Warning: Performance issues - Avoid using
	u16 height;
	u16 width;
	u8 * smudge1; // initial smudge value
	u8 * smudge2; // modified smudge value
	u32 * painted; // height * width
	object_t ** obj_painted; // height * width
	u32 * topaint; // height * width
	u32 * topaint_flags; // height * width
	object_t * objects; // objects queue
	collision_t * collision_events; // collision queues
	u32 last_rgb;
	u32 paint_count;
	u8 auto_commit;
} engine_t;

/*
 * Creates new window with the specified size and background solid color
 */
engine_t * new_engine(u16 scr_width, u16 scr_height, sprite_t * bg);
/*
 * Creates new object with sprite by class_id, callback function is optional
 * class_id - used to select the first sprite of the sprite stack to be loaded
 * unique_id - not used, use for whatever you need
 * mask - Binary mask of the collisions registered with this object
 * keep_alive - whether object is to be kept alive even if it goes out of screen
 * pos_x - position from the left
 * pos_y - position from the top
 * smudge - smudge effect duration of object
 * func - callback on engine step
 */
object_t * new_object(u32 class_id, u32 unique_id, u32 mask, u8 keep_alive, i16 pos_x, i16 pos_y, u8 smudge, void (* func)(object_t *));
/*
 * Adds an object to the START of the queue (this means this object is the least
 * visible on a paint job
 */
void add_object_engine(engine_t * en, object_t * ob);
/*
 * Adds a collision mask to the engine queue
 */
void add_collision_engine(engine_t * en, u32 mask_combo, void (* func)(object_t *, object_t *));
/*
 * Repaints background, objects and checks for collisions.
 * EVERY 255 PAINTS THE SCREEN IS FULLY REPAINT INSTEAD OF PARTIALLY.
 */
void paint_engine(engine_t * en);
/*
 * Forces next call to paint_engine to repaint whole screen
 */
void force_next_paint_engine(engine_t * en);
/*
 * Commits painting to X server if engine is set to manual commit
 */
void manual_commit_engine(engine_t * en);
/*
 * Calls the functions associated with the alive objects
 */
void run_objects_engine(engine_t * en);
/*
 * Calls callback function if the specified key is being pressed passing to it
 * the specified key code. Callback is optional.
 * Returns true if key detected.
 */
u8 poll_keybd_engine(engine_t * en, u8 keycode, void (* func)(u8));
/*
 * Calls callback function if any mouse key is being pressed passing to it the
 * mouse keycode. Callback is optional.
 * Returns true if mouse press detected.
 */
u8 poll_mouse_engine(engine_t * en, void (* func)(u32));
/*
 * Adds a loaded sprite into the top of an object animation queue.
 */
void add_sprite_object(object_t * obj, sprite_t * sprite);
/*
 * Allocates space for an object structure or uses pre-allocated objects
 */
object_t * alloc_object();
/*
 * Deallocates or caches the space of an object to be used by future new_object
 * calls
 */
void destroy_object(object_t * ob);
/*
 * Equivalent to paint engine followed by running the engine objects
 */
void step_engine(engine_t * en);


/*
 * Drawing trecniques and effects
 * Don't use at the same time as game object rendering
 */
/*
 * Draws a sprite in the location specified (does not affect intermediate
 * buffers of game, object association and so on)
 */
void draw_sprite_engine(engine_t * en, sprite_t * sprite, u16 left, u16 top);
/*
 * Drwas a sprite in the location specified with the specified opacity
 * (affects last painted buffer since that is a requirement to use gradual
 * fade-in effects)
 */
void fade_in_sprite_engine(engine_t * en, sprite_t * sprite, u16 left, u16 top, u8 opacity);
/*
 * Performs maintenance tasks that might not be done while busy for speed
 * reasons.
 * Call this when the application is waiting for input or on a timer.
 */
void freetime_engine(engine_t * en);