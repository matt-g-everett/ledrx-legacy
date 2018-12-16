#include "freertos/FreeRTOS.h"

#include "hsv.h"
#include "pixels.h"

#define HSV_HUE_SEXTANT		256
#define HSV_HUE_STEPS		(6 * HSV_HUE_SEXTANT)

#define HSV_HUE_MIN		0
#define HSV_HUE_MAX		(HSV_HUE_STEPS - 1)
#define HSV_SAT_MIN		0
#define HSV_SAT_MAX		255
#define HSV_VAL_MIN		0
#define HSV_VAL_MAX		255

#define HSV_MONOCHROMATIC_TEST(s,v,r,g,b) \
	do { \
		if(!(s)) { \
			 *(r) = *(g) = *(b) = (v); \
			return; \
		} \
	} while(0)

#ifdef HSV_USE_SEXTANT_TEST
#define HSV_SEXTANT_TEST(sextant) \
	do { \
		if((sextant) > 5) { \
			(sextant) = 5; \
		} \
	} while(0)
#else
#define HSV_SEXTANT_TEST(sextant) do { ; } while(0)
#endif

/*
 * Pointer swapping:
 * 	sext.	r g b	r<>b	g<>b	r <> g	result
 *	0 0 0	v u c			!u v c	u v c
 *	0 0 1	d v c				d v c
 *	0 1 0	c v u	u v c			u v c
 *	0 1 1	c d v	v d c		d v c	d v c
 *	1 0 0	u c v		u v c		u v c
 *	1 0 1	v c d		v d c	d v c	d v c
 *
 * if(sextant & 2)
 * 	r <-> b
 *
 * if(sextant & 4)
 * 	g <-> b
 *
 * if(!(sextant & 6) {
 * 	if(!(sextant & 1))
 * 		r <-> g
 * } else {
 * 	if(sextant & 1)
 * 		r <-> g
 * }
 */
#define HSV_SWAPPTR(a,b)	do { uint8_t *tmp = (a); (a) = (b); (b) = tmp; } while(0)
#define HSV_POINTER_SWAP(sextant,r,g,b) \
	do { \
		if((sextant) & 2) { \
			HSV_SWAPPTR((r), (b)); \
		} \
		if((sextant) & 4) { \
			HSV_SWAPPTR((g), (b)); \
		} \
		if(!((sextant) & 6)) { \
			if(!((sextant) & 1)) { \
				HSV_SWAPPTR((r), (g)); \
			} \
		} else { \
			if((sextant) & 1) { \
				HSV_SWAPPTR((r), (g)); \
			} \
		} \
	} while(0)

void hsv2rgb(HSV_t hsv, RGB_t *rgb) {
    fast_hsv2rgb_8bit(hsv.h, hsv.s, hsv.v, &rgb->r, &rgb->g, &rgb->b);
}

void fast_hsv2rgb_8bit(uint16_t h, uint8_t s, uint8_t v, uint8_t *r, uint8_t *g , uint8_t *b)
{
	HSV_MONOCHROMATIC_TEST(s, v, r, g, b);	// Exit with grayscale if s == 0

	uint8_t sextant = h >> 8;

	HSV_SEXTANT_TEST(sextant);		// Optional: Limit hue sextants to defined space

	HSV_POINTER_SWAP(sextant, r, g, b);	// Swap pointers depending which sextant we are in

	*g = v;		// Top level

	// Perform actual calculations
	uint8_t bb;
	uint16_t ww;

	/*
	 * Bottom level: v * (1.0 - s)
	 * --> (v * (255 - s) + error_corr) / 256
	 */
	bb = ~s;
	ww = v * bb;
	ww += 1;		// Error correction
	ww += ww >> 8;		// Error correction
	*b = ww >> 8;

	uint8_t h_fraction = h & 0xff;	// 0...255

	if(!(sextant & 1)) {
		// *r = ...slope_up...;
		/*
		 * Slope up: v * (1.0 - s * (1.0 - h))
		 * --> (v * (255 - (s * (256 - h) + error_corr1) / 256) + error_corr2) / 256
		 */
		ww = !h_fraction ? ((uint16_t)s << 8) : (s * (uint8_t)(-h_fraction));
		ww += ww >> 8;		// Error correction 1
		bb = ww >> 8;
		bb = ~bb;
		ww = v * bb;
		ww += v >> 1;		// Error correction 2
		*r = ww >> 8;
	} else {
		// *r = ...slope_down...;
		/*
		 * Slope down: v * (1.0 - s * h)
		 * --> (v * (255 - (s * h + error_corr1) / 256) + error_corr2) / 256
		 */
		ww = s * h_fraction;
		ww += ww >> 8;		// Error correction 1
		bb = ww >> 8;
		bb = ~bb;
		ww = v * bb;
		ww += v >> 1;		// Error correction 2
		*r = ww >> 8;

		/*
		 * A perfect match for h_fraction == 0 implies:
		 *	*r = (ww >> 8) + (h_fraction ? 0 : 1)
		 * However, this is an extra calculation that may not be required.
		 */
	}
}
